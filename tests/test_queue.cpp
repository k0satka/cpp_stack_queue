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

    const std::string* original_address = &q.front();

    for (int i = 0; i < 100; i++)
        q.push(q.front());

    EXPECT_EQ(q.size(), 101u);
    EXPECT_EQ(&q.front(), original_address);

    while (!q.empty()) {
        EXPECT_EQ(q.front(), "qwerty");
        q.pop();
    }
}

//rule файв

TEST(TQueueTest, copy_constructor) {
    TQueue<int> source;

    for (int i = 0; i < 100; i++)
        source.push(i);

    for (int i = 0; i < 17; i++)
        source.pop();

    TQueue<int> copy(source);

    EXPECT_EQ(copy.size(), 83u);
    EXPECT_EQ(copy.front(), 17);
    EXPECT_EQ(copy.back(), 99);

    source.front() = 1000;
    source.back() = 2000;

    EXPECT_EQ(copy.front(), 17);
    EXPECT_EQ(copy.back(), 99);

    for (int i = 17; i < 100; i++) {
        ASSERT_FALSE(copy.empty());
        EXPECT_EQ(copy.front(), i);
        copy.pop();
    }

    EXPECT_TRUE(copy.empty());
}

TEST(TQueueTest, copy_assignment) {
    TQueue<int> source;

    for (int i = 0; i < 100; i++)
        source.push(i);

    TQueue<int> target;
    target.push(999);
    target.push(888);

    target = source;

    EXPECT_EQ(target.size(), 100u);
    EXPECT_EQ(target.front(), 0);
    EXPECT_EQ(target.back(), 99);

    source.front() = 500;

    EXPECT_EQ(target.front(), 0);
    EXPECT_EQ(source.front(), 500);
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
    TQueue<int> source;

    for (int i = 0; i < 100; i++)
        source.push(i);

    int* first_address = &source.front();
    int* last_address = &source.back();

    TQueue<int> moved(std::move(source));

    EXPECT_EQ(moved.size(), 100u);
    EXPECT_EQ(moved.front(), 0);
    EXPECT_EQ(moved.back(), 99);

    EXPECT_EQ(&moved.front(), first_address);
    EXPECT_EQ(&moved.back(), last_address);

    EXPECT_TRUE(source.empty());

    source.push(42);
    EXPECT_EQ(source.front(), 42);
}

TEST(TQueueTest, move_assignment) {
    TQueue<int> source;

    for (int i = 0; i < 100; i++)
        source.push(i);

    int* original_address = &source.front();

    TQueue<int> target;
    target.push(999);
    target.push(888);

    target = std::move(source);

    EXPECT_EQ(target.size(), 100u);
    EXPECT_EQ(target.front(), 0);
    EXPECT_EQ(target.back(), 99);
    EXPECT_EQ(&target.front(), original_address);

    EXPECT_TRUE(source.empty());

    source.push(42);
    EXPECT_EQ(source.front(), 42);
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
    const int baseline = QueueImmovable::alive;

    {
        TQueue<QueueImmovable> q;

        QueueImmovable& first = q.emplace(10);
        QueueImmovable* address = &first;

        for (int i = 1; i <= 100; i++)
            q.emplace(i);

        EXPECT_EQ(q.size(), 101u);
        EXPECT_EQ(q.front().value, 10);
        EXPECT_EQ(q.back().value, 100);

        EXPECT_EQ(&q.front(), address);
        EXPECT_EQ(QueueImmovable::alive, baseline + 101);

        q.pop();

        EXPECT_EQ(q.front().value, 1);
        EXPECT_EQ(QueueImmovable::alive, baseline + 100);
    }

    EXPECT_EQ(QueueImmovable::alive, baseline);
}

//исключения

TEST(TQueueTest, emplace_exception_safety) {
    const int baseline = QueueImmovable::alive;

    {
        TQueue<QueueImmovable> q;

        for (int i = 0; i < 100; i++)
            q.emplace(i);

        auto* first_address = &q.front();
        auto* last_address = &q.back();

        EXPECT_THROW(q.emplace(-1), std::runtime_error);

        EXPECT_EQ(q.size(), 100u);
        EXPECT_EQ(q.front().value, 0);
        EXPECT_EQ(q.back().value, 99);
        EXPECT_EQ(&q.front(), first_address);
        EXPECT_EQ(&q.back(), last_address);
        EXPECT_EQ(QueueImmovable::alive, baseline + 100);
    }

    EXPECT_EQ(QueueImmovable::alive, baseline);
}

TEST(TQueueTest, push_exception_safety) {
    QueueCopyProbe::reset();

    QueueCopyProbe value(42);
    TQueue<QueueCopyProbe> q;

    for (int i = 0; i < 80; i++)
        q.push(value);

    const int alive_before = QueueCopyProbe::alive;

    QueueCopyProbe::copies = 0;
    QueueCopyProbe::fail_at = 1;

    EXPECT_THROW(q.push(value), std::runtime_error);

    QueueCopyProbe::reset();

    EXPECT_EQ(q.size(), 80u);
    EXPECT_EQ(q.front().value, 42);
    EXPECT_EQ(q.back().value, 42);
    EXPECT_EQ(QueueCopyProbe::alive, alive_before);
}

TEST(TQueueTest, copy_constructor_exception_safety) {
    QueueCopyProbe::reset();

    const int baseline = QueueCopyProbe::alive;

    {
        QueueCopyProbe value(42);
        TQueue<QueueCopyProbe> source;

        for (int i = 0; i < 80; i++)
            source.push(value);

        const int alive_before = QueueCopyProbe::alive;

        QueueCopyProbe::copies = 0;
        QueueCopyProbe::fail_at = 35;

        EXPECT_THROW(
            { TQueue<QueueCopyProbe> copy(source); },
            std::runtime_error
        );

        QueueCopyProbe::reset();

        EXPECT_EQ(source.size(), 80u);
        EXPECT_EQ(source.front().value, 42);
        EXPECT_EQ(QueueCopyProbe::alive, alive_before);
    }

    EXPECT_EQ(QueueCopyProbe::alive, baseline);
}

TEST(TQueueTest, copy_assignment_exception_safety) {
    QueueCopyProbe::reset();

    QueueCopyProbe value(42);
    QueueCopyProbe old_value(99);

    TQueue<QueueCopyProbe> source;
    for (int i = 0; i < 80; i++)
        source.push(value);

    TQueue<QueueCopyProbe> target;
    target.push(old_value);

    const int alive_before = QueueCopyProbe::alive;

    QueueCopyProbe::copies = 0;
    QueueCopyProbe::fail_at = 35;

    EXPECT_THROW(target = source, std::runtime_error);

    QueueCopyProbe::reset();

    EXPECT_EQ(target.size(), 1u);
    EXPECT_EQ(target.front().value, 99);
    EXPECT_EQ(target.back().value, 99);

    EXPECT_EQ(source.size(), 80u);
    EXPECT_EQ(source.front().value, 42);

    EXPECT_EQ(QueueCopyProbe::alive, alive_before);
}