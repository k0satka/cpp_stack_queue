#include <gtest/gtest.h>
#include "tqueue.h"

//вспомогательные типы
struct QueueImmovable {
    inline static int alive = 0;
    int value;

    explicit QueueImmovable(int v) : value(v) {
        if (v < 0) throw std::runtime_error("construction failed");
        ++alive;
    }

    QueueImmovable(const QueueImmovable&) = delete;
    QueueImmovable(QueueImmovable&&) = delete;

    QueueImmovable& operator=(const QueueImmovable&) = delete;
    QueueImmovable& operator=(QueueImmovable&&) = delete;

    ~QueueImmovable() noexcept { --alive; }
};

struct QueueCopyProbe {
    inline static int alive = 0;
    inline static int copies = 0;
    inline static int fail_at = 0;

    int value;

    explicit QueueCopyProbe(int v) : value(v) { ++alive; }

    QueueCopyProbe(const QueueCopyProbe& other) : value(other.value) {
        ++copies;
        if (fail_at != 0 && copies == fail_at) throw std::runtime_error("copy failed");
        ++alive;
    }

    QueueCopyProbe(QueueCopyProbe&&) = delete;

    ~QueueCopyProbe() noexcept { --alive; }

    static void reset() {
        copies = 0;
        fail_at = 0;
    }
};

//базовые операции

TEST(TQueueTest, default_constructor) {
    TQueue<int> q;

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0u);
    EXPECT_THROW(q.front(), std::out_of_range);
    EXPECT_THROW(q.back(), std::out_of_range);
    EXPECT_NO_THROW(q.pop());
}

TEST(TQueueTest, fifo_order) {
    TQueue<int> q;

    for (int i = 0; i < 100; i++)
        q.push(i);

    EXPECT_EQ(q.size(), 100u);
    EXPECT_EQ(q.front(), 0);
    EXPECT_EQ(q.back(), 99);

    for (int i = 0; i < 100; i++) {
        ASSERT_FALSE(q.empty());
        EXPECT_EQ(q.front(), i);
        EXPECT_EQ(q.back(), 99);
        q.pop();
    }

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0u);
}

TEST(TQueueTest, front_back_references) {
    TQueue<int> q;

    q.push(10);
    q.push(20);
    q.push(30);

    q.front() = 100;
    q.back() = 228;

    const TQueue<int>& cq = q;

    EXPECT_EQ(cq.front(), 100);
    EXPECT_EQ(cq.back(), 228);

    q.pop();

    EXPECT_EQ(q.front(), 20);
    EXPECT_EQ(q.back(), 228);
}

TEST(TQueueTest, clear_and_reuse) {
    TQueue<int> q;

    for (int i = 0; i < 100; i++)
        q.push(i);

    q.clear();

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0u);

    q.push(42);
    q.push(43);

    EXPECT_EQ(q.front(), 42);
    EXPECT_EQ(q.back(), 43);

    q.clear();
    q.clear();

    EXPECT_TRUE(q.empty());
}

TEST(TQueueTest, push_own_front) {
    TQueue<std::string> q;

    q.push("qwerty");

    const std::string* p = &q.front();

    for (int i = 0; i < 100; i++)
        q.push(q.front());

    EXPECT_EQ(q.size(), 101u);
    EXPECT_EQ(&q.front(), p);

    while (!q.empty()) {
        EXPECT_EQ(q.front(), "qwerty");
        q.pop();
    }
}

//rule файв

TEST(TQueueTest, copy_constructor) {
    TQueue<int> q;

    for (int i = 0; i < 100; i++)
        q.push(i);

    for (int i = 0; i < 17; i++)
        q.pop();

    TQueue<int> q2(q);

    EXPECT_EQ(q2.size(), 83u);
    EXPECT_EQ(q2.front(), 17);
    EXPECT_EQ(q2.back(), 99);

    q.front() = 1000;
    q.back() = 2000;

    EXPECT_EQ(q2.front(), 17);
    EXPECT_EQ(q2.back(), 99);

    for (int i = 17; i < 100; i++) {
        ASSERT_FALSE(q2.empty());
        EXPECT_EQ(q2.front(), i);
        q2.pop();
    }

    EXPECT_TRUE(q2.empty());
}

TEST(TQueueTest, copy_assignment) {
    TQueue<int> q;

    for (int i = 0; i < 100; i++)
        q.push(i);

    TQueue<int> q2;
    q2.push(999);
    q2.push(888);

    q2 = q;

    EXPECT_EQ(q2.size(), 100u);
    EXPECT_EQ(q2.front(), 0);
    EXPECT_EQ(q2.back(), 99);

    q.front() = 500;

    EXPECT_EQ(q2.front(), 0);
    EXPECT_EQ(q.front(), 500);
}

TEST(TQueueTest, self_copy_assignment) {
    TQueue<int> q;

    q.push(10);
    q.push(20);

    q = q;

    EXPECT_EQ(q.size(), 2u);
    EXPECT_EQ(q.front(), 10);
    EXPECT_EQ(q.back(), 20);
}

TEST(TQueueTest, move_constructor) {
    TQueue<int> q;

    for (int i = 0; i < 100; i++)
        q.push(i);

    int* p1 = &q.front();
    int* p2 = &q.back();

    TQueue<int> q2(std::move(q));

    EXPECT_EQ(q2.size(), 100u);
    EXPECT_EQ(q2.front(), 0);
    EXPECT_EQ(q2.back(), 99);

    EXPECT_EQ(&q2.front(), p1);
    EXPECT_EQ(&q2.back(), p2);

    EXPECT_TRUE(q.empty());

    q.push(42);
    EXPECT_EQ(q.front(), 42);
}

TEST(TQueueTest, move_assignment) {
    TQueue<int> q;

    for (int i = 0; i < 100; i++)
        q.push(i);

    int* p = &q.front();

    TQueue<int> q2;
    q2.push(999);
    q2.push(888);

    q2 = std::move(q);

    EXPECT_EQ(q2.size(), 100u);
    EXPECT_EQ(q2.front(), 0);
    EXPECT_EQ(q2.back(), 99);
    EXPECT_EQ(&q2.front(), p);

    EXPECT_TRUE(q.empty());

    q.push(42);
    EXPECT_EQ(q.front(), 42);
}

TEST(TQueueTest, self_move_assignment) {
    TQueue<int> q;

    q.push(10);
    q.push(20);

    q = std::move(q);

    EXPECT_EQ(q.size(), 2u);
    EXPECT_EQ(q.front(), 10);
    EXPECT_EQ(q.back(), 20);
}

//странные типы

TEST(TQueueTest, push_move_only) {
    TQueue<std::unique_ptr<int>> q;

    q.push(std::make_unique<int>(10));
    q.push(std::make_unique<int>(20));

    EXPECT_EQ(q.size(), 2u);
    EXPECT_EQ(*q.front(), 10);
    EXPECT_EQ(*q.back(), 20);

    auto ptr = std::make_unique<int>(30);
    q.push(std::move(ptr));

    EXPECT_EQ(ptr, nullptr);
    EXPECT_EQ(*q.back(), 30);

    q.pop();
    EXPECT_EQ(*q.front(), 20);

    q.pop();
    EXPECT_EQ(*q.front(), 30);
}

TEST(TQueueTest, emplace_immovable) {
    const int n = QueueImmovable::alive;

    {
        TQueue<QueueImmovable> q;

        QueueImmovable& x = q.emplace(10);
        QueueImmovable* p = &x;

        for (int i = 1; i <= 100; i++)
            q.emplace(i);

        EXPECT_EQ(q.size(), 101u);
        EXPECT_EQ(q.front().value, 10);
        EXPECT_EQ(q.back().value, 100);

        EXPECT_EQ(&q.front(), p);
        EXPECT_EQ(QueueImmovable::alive, n + 101);

        q.pop();

        EXPECT_EQ(q.front().value, 1);
        EXPECT_EQ(QueueImmovable::alive, n + 100);
    }

    EXPECT_EQ(QueueImmovable::alive, n);
}

//исключения

TEST(TQueueTest, emplace_exception_safety) {
    const int n = QueueImmovable::alive;

    {
        TQueue<QueueImmovable> q;

        for (int i = 0; i < 100; i++)
            q.emplace(i);

        auto* p1 = &q.front();
        auto* p2 = &q.back();

        EXPECT_THROW(q.emplace(-1), std::runtime_error);

        EXPECT_EQ(q.size(), 100u);
        EXPECT_EQ(q.front().value, 0);
        EXPECT_EQ(q.back().value, 99);
        EXPECT_EQ(&q.front(), p1);
        EXPECT_EQ(&q.back(), p2);
        EXPECT_EQ(QueueImmovable::alive, n + 100);
    }

    EXPECT_EQ(QueueImmovable::alive, n);
}

TEST(TQueueTest, push_exception_safety) {
    QueueCopyProbe::reset();

    QueueCopyProbe x(42);
    TQueue<QueueCopyProbe> q;

    for (int i = 0; i < 80; i++)
        q.push(x);

    const int n = QueueCopyProbe::alive;

    QueueCopyProbe::copies = 0;
    QueueCopyProbe::fail_at = 1;

    EXPECT_THROW(q.push(x), std::runtime_error);

    QueueCopyProbe::reset();

    EXPECT_EQ(q.size(), 80u);
    EXPECT_EQ(q.front().value, 42);
    EXPECT_EQ(q.back().value, 42);
    EXPECT_EQ(QueueCopyProbe::alive, n);
}

TEST(TQueueTest, copy_constructor_exception_safety) {
    QueueCopyProbe::reset();

    const int n = QueueCopyProbe::alive;

    {
        QueueCopyProbe x(42);
        TQueue<QueueCopyProbe> q;

        for (int i = 0; i < 80; i++)
            q.push(x);

        const int n2 = QueueCopyProbe::alive;

        QueueCopyProbe::copies = 0;
        QueueCopyProbe::fail_at = 35;

        EXPECT_THROW(
            { TQueue<QueueCopyProbe> q2(q); },
            std::runtime_error
        );

        QueueCopyProbe::reset();

        EXPECT_EQ(q.size(), 80u);
        EXPECT_EQ(q.front().value, 42);
        EXPECT_EQ(QueueCopyProbe::alive, n2);
    }

    EXPECT_EQ(QueueCopyProbe::alive, n);
}

TEST(TQueueTest, copy_assignment_exception_safety) {
    QueueCopyProbe::reset();

    QueueCopyProbe x(42);
    QueueCopyProbe y(99);

    TQueue<QueueCopyProbe> q;
    for (int i = 0; i < 80; i++)
        q.push(x);

    TQueue<QueueCopyProbe> q2;
    q2.push(y);

    const int n = QueueCopyProbe::alive;

    QueueCopyProbe::copies = 0;
    QueueCopyProbe::fail_at = 35;

    EXPECT_THROW(q2 = q, std::runtime_error);

    QueueCopyProbe::reset();

    EXPECT_EQ(q2.size(), 1u);
    EXPECT_EQ(q2.front().value, 99);
    EXPECT_EQ(q2.back().value, 99);

    EXPECT_EQ(q.size(), 80u);
    EXPECT_EQ(q.front().value, 42);

    EXPECT_EQ(QueueCopyProbe::alive, n);
}
