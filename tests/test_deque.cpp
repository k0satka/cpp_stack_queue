#include <gtest/gtest.h>
#include "tdeque.h"

//вспомогательный тип

struct CopyProbe {
    inline static int alive = 0;
    inline static int copies = 0;
    inline static int fail_at = 0;

    int value;

    explicit CopyProbe(int v) : value(v) { ++alive; }

    CopyProbe(const CopyProbe& other) : value(other.value) {
        ++copies;
        if (fail_at != 0 && copies == fail_at) throw std::runtime_error("copy failed");
        ++alive;
    }

    CopyProbe(CopyProbe&&) = delete;
    CopyProbe& operator=(const CopyProbe&) = delete;
    CopyProbe& operator=(CopyProbe&&) = delete;

    ~CopyProbe() noexcept { --alive; }

    static void reset() {
        copies = 0;
        fail_at = 0;
    }
};

struct alignas(64) AlignedValue {
    int value;

    explicit AlignedValue(int v) : value(v) {}
};

struct Immovable {
    int value;

    explicit Immovable(int v) : value(v) {}

    Immovable(const Immovable&) = delete;
    Immovable(Immovable&&) = delete;

    Immovable& operator=(const Immovable&) = delete;
    Immovable& operator=(Immovable&&) = delete;
};

struct ThrowOnConstruct {
    inline static int alive = 0;

    int value;

    explicit ThrowOnConstruct(int v) : value(v) {
        if (v == -1) throw std::runtime_error("construction failed");

        ++alive;
    }

    ThrowOnConstruct(const ThrowOnConstruct&) = delete;
    ThrowOnConstruct(ThrowOnConstruct&&) = delete;

    ~ThrowOnConstruct() { --alive; }
};

//базовые проверки

TEST(TDequeTest, default_constructor) {
    TDeque<int> q;

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0u);
    EXPECT_EQ(q.getCapacity(), 0u);
    EXPECT_THROW(q.front(), std::out_of_range);
    EXPECT_THROW(q.back(), std::out_of_range);
}

TEST(TDequeTest, capacity_constructor) {
    TDeque<int> q(7);

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0u);
    EXPECT_EQ(q.getCapacity(), 7u);
}

TEST(TDequeTest, push_back) {
    TDeque<int> q(3);

    q.push_back(1);
    q.push_back(2);
    q.push_back(3);

    EXPECT_EQ(q.size(), 3u);
    EXPECT_EQ(q.getCapacity(), 3u);
    EXPECT_EQ(q.front(), 1);
    EXPECT_EQ(q.back(), 3);
}

TEST(TDequeTest, push_front) {
    TDeque<int> q(3);

    q.push_front(1);
    q.push_front(2);
    q.push_front(3);

    EXPECT_EQ(q.size(), 3u);
    EXPECT_EQ(q.getCapacity(), 3u);
    EXPECT_EQ(q.front(), 3);
    EXPECT_EQ(q.back(), 1);
}

TEST(TDequeTest, push_alias_with_resize) {
    TDeque<int> q1(2);
    q1.push_back(1);
    q1.push_back(2);
    q1.push_back(q1.front());

    EXPECT_EQ(q1.size(), 3u);
    EXPECT_EQ(q1.getCapacity(), 4u);
    EXPECT_EQ(q1.front(), 1);
    EXPECT_EQ(q1.back(), 1);

    TDeque<int> q2(2);
    q2.push_back(1);
    q2.push_back(2);
    q2.push_front(q2.back());

    EXPECT_EQ(q2.size(), 3u);
    EXPECT_EQ(q2.getCapacity(), 4u);
    EXPECT_EQ(q2.front(), 2);
    EXPECT_EQ(q2.back(), 2);
}

TEST(TDequeTest, pop_back) {
    TDeque<int> q(4);
    for (int i = 1; i <= 4; i++)
        q.push_back(i);

    q.pop_back();
    EXPECT_EQ(q.size(), 3u);
    EXPECT_EQ(q.back(), 3);

    q.pop_back();
    EXPECT_EQ(q.size(), 2u);
    EXPECT_EQ(q.back(), 2);

    EXPECT_EQ(q.front(), 1);
    EXPECT_EQ(q.getCapacity(), 4u);
}

TEST(TDequeTest, pop_front) {
    TDeque<int> q(4);
    for (int i = 1; i <= 4; i++)
        q.push_back(i);

    q.pop_front();
    EXPECT_EQ(q.size(), 3u);
    EXPECT_EQ(q.front(), 2);

    q.pop_front();
    EXPECT_EQ(q.size(), 2u);
    EXPECT_EQ(q.front(), 3);

    EXPECT_EQ(q.back(), 4);
    EXPECT_EQ(q.getCapacity(), 4u);
}

TEST(TDequeTest, pop_last_keeps_capacity) {
    TDeque<int> q(3);

    q.push_back(10);
    q.pop_back();

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.getCapacity(), 3u);

    q.push_front(20);
    q.pop_front();

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.getCapacity(), 3u);

    q.push_back(30);
    EXPECT_EQ(q.front(), 30);
    EXPECT_EQ(q.back(), 30);
}

TEST(TDequeTest, empty_operations) {
    TDeque<int> q(3);

    q.pop_front();
    q.pop_back();

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0u);
    EXPECT_EQ(q.getCapacity(), 3u);
    EXPECT_THROW(q.front(), std::out_of_range);
    EXPECT_THROW(q.back(), std::out_of_range);
}

TEST(TDequeTest, front_back_references) {
    TDeque<int> q(3);
    q.push_back(10);
    q.push_back(20);

    q.front() = 100;
    q.back() = 200;

    const TDeque<int>& cq = q;

    EXPECT_EQ(cq.front(), 100);
    EXPECT_EQ(cq.back(), 200);
}

TEST(TDequeTest, clear) {
    TDeque<int> q(3);
    q.push_back(1);
    q.push_back(2);
    q.push_back(3);

    q.clear();

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0u);
    EXPECT_EQ(q.getCapacity(), 0u);

    q.push_back(2);
    q.push_front(1);

    EXPECT_EQ(q.size(), 2u);
    EXPECT_EQ(q.front(), 1);
    EXPECT_EQ(q.back(), 2);
}

//кольцевой буффер

TEST(TDequeTest, circular_buffer) {
    TDeque<int> q(5);
    for (int i = 1; i <= 4; i++)
        q.push_back(i);

    q.pop_front();
    q.pop_front();

    q.push_back(5);
    q.push_back(6);
    q.push_back(7);

    EXPECT_EQ(q.size(), 5u);
    EXPECT_EQ(q.getCapacity(), 5u);

    for (int i = 3; i <= 7; i++) {
        ASSERT_FALSE(q.empty());
        EXPECT_EQ(q.front(), i);
        q.pop_front();
    }

    EXPECT_TRUE(q.empty());
}

TEST(TDequeTest, resize_back_wrapped) {
    TDeque<int> q(4);
    for (int i = 1; i <= 4; i++)
        q.push_back(i);

    q.pop_front();
    q.pop_front();

    q.push_back(5);
    q.push_back(6);
    q.push_back(7);

    EXPECT_EQ(q.size(), 5u);
    EXPECT_EQ(q.getCapacity(), 8u);

    for (int i = 3; i <= 7; i++) {
        ASSERT_FALSE(q.empty());
        EXPECT_EQ(q.front(), i);
        q.pop_front();
    }

    EXPECT_TRUE(q.empty());
}

TEST(TDequeTest, resize_front_wrapped) {
    TDeque<int> q(4);

    q.push_back(3);
    q.push_back(4);
    q.push_back(5);
    q.push_back(6);

    q.pop_back();
    q.pop_back();

    q.push_front(2);
    q.push_front(1);
    q.push_front(0);

    EXPECT_EQ(q.size(), 5u);
    EXPECT_EQ(q.getCapacity(), 8u);

    for (int i = 0; i <= 4; i++) {
        ASSERT_FALSE(q.empty());
        EXPECT_EQ(q.front(), i);
        q.pop_front();
    }

    EXPECT_TRUE(q.empty());
}

//rule 5

TEST(TDequeTest, copy_constructor) {
    TDeque<int> q(4);
    for (int i = 1; i <= 4; i++)
        q.push_back(i);

    q.pop_front();
    q.push_back(5);

    TDeque<int> q2(q);

    q.front() = 20;
    q.back() = 50;

    EXPECT_EQ(q2.size(), 4u);
    EXPECT_EQ(q2.getCapacity(), 4u);

    for (int i = 2; i <= 5; i++) {
        ASSERT_FALSE(q2.empty());
        EXPECT_EQ(q2.front(), i);
        q2.pop_front();
    }

    EXPECT_EQ(q.front(), 20);
    EXPECT_EQ(q.back(), 50);
}

TEST(TDequeTest, copy_assignment) {
    TDeque<int> q(4);
    for (int i = 1; i <= 4; i++)
        q.push_back(i);

    q.pop_front();
    q.push_back(5);

    TDeque<int> q2(2);
    q2.push_back(10);
    q2.push_back(20);

    q2 = q;

    q.front() = 20;
    q.back() = 50;

    EXPECT_EQ(q2.size(), 4u);
    EXPECT_EQ(q2.getCapacity(), 4u);

    for (int i = 2; i <= 5; i++) {
        ASSERT_FALSE(q2.empty());
        EXPECT_EQ(q2.front(), i);
        q2.pop_front();
    }
}

TEST(TDequeTest, self_assignment) {
    TDeque<int> q(3);
    q.push_back(1);
    q.push_back(2);
    q.push_back(3);

    q = q;

    EXPECT_EQ(q.size(), 3u);
    EXPECT_EQ(q.getCapacity(), 3u);

    for (int i = 1; i <= 3; i++) {
        ASSERT_FALSE(q.empty());
        EXPECT_EQ(q.front(), i);
        q.pop_front();
    }
}

TEST(TDequeTest, copy_empty) {
    TDeque<int> q;
    TDeque<int> q2(q);

    EXPECT_TRUE(q2.empty());
    EXPECT_EQ(q2.getCapacity(), 0u);

    TDeque<int> q3(5);
    q3.push_back(10);
    q3 = q;

    EXPECT_TRUE(q3.empty());
    EXPECT_EQ(q3.getCapacity(), 0u);

    TDeque<int> q4(7);
    TDeque<int> q5(q4);

    EXPECT_TRUE(q5.empty());
    EXPECT_EQ(q5.getCapacity(), 7u);
}

TEST(TDequeTest, move_constructor) {
    TDeque<int> q(5);
    q.push_back(1);
    q.push_back(2);
    q.push_back(3);
    q.pop_front();

    TDeque<int> q2(std::move(q));

    EXPECT_EQ(q2.size(), 2u);
    EXPECT_EQ(q2.getCapacity(), 5u);
    EXPECT_EQ(q2.front(), 2);
    EXPECT_EQ(q2.back(), 3);

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.getCapacity(), 0u);

    q.push_back(42);

    EXPECT_EQ(q.front(), 42);
    EXPECT_EQ(q2.front(), 2);
}

TEST(TDequeTest, move_assignment) {
    TDeque<int> q(3);
    q.push_back(1);
    q.push_back(2);

    TDeque<int> q2(4);
    q2.push_back(42);
    q2.push_back(142);

    q2 = std::move(q);

    EXPECT_EQ(q2.size(), 2u);
    EXPECT_EQ(q2.getCapacity(), 3u);
    EXPECT_EQ(q2.front(), 1);
    EXPECT_EQ(q2.back(), 2);

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.getCapacity(), 0u);

    q.push_front(5);

    EXPECT_EQ(q.front(), 5);
    EXPECT_EQ(q2.front(), 1);
}

TEST(TDequeTest, self_move_assignment) {
    TDeque<int> q(3);
    q.push_back(1);
    q.push_back(2);

    q = std::move(q);

    EXPECT_EQ(q.size(), 2u);
    EXPECT_EQ(q.getCapacity(), 3u);
    EXPECT_EQ(q.front(), 1);
    EXPECT_EQ(q.back(), 2);
}

TEST(TDequeTest, move_source_destruction) {
    TDeque<int> q;

    {
        TDeque<int> q2(3);
        q2.push_back(10);
        q2.push_back(20);

        q = std::move(q2);
    }

    EXPECT_EQ(q.size(), 2u);
    EXPECT_EQ(q.front(), 10);
    EXPECT_EQ(q.back(), 20);

    q.pop_front();
    EXPECT_EQ(q.front(), 20);
}

//ограничения T

TEST(TDequeTest, element_lifetime) {
    CopyProbe::reset();
    int n = CopyProbe::alive;

    {
        TDeque<CopyProbe> q(4);

        EXPECT_EQ(CopyProbe::alive, n);

        CopyProbe x(10);

        q.push_back(x);
        q.push_front(x);

        EXPECT_EQ(q.size(), 2u);
        EXPECT_EQ(CopyProbe::alive, n + 3);

        q.pop_front();
        EXPECT_EQ(CopyProbe::alive, n + 2);

        q.pop_back();
        EXPECT_TRUE(q.empty());
        EXPECT_EQ(q.getCapacity(), 4u);
        EXPECT_EQ(CopyProbe::alive, n + 1);

        q.push_back(x);
        EXPECT_EQ(q.front().value, 10);

        q.clear();
        EXPECT_EQ(CopyProbe::alive, n + 1);
    }

    EXPECT_EQ(CopyProbe::alive, n);
}

TEST(TDequeTest, non_default_non_assignable_type) {
    CopyProbe::reset();

    CopyProbe a(10), b(20);
    TDeque<CopyProbe> q(2);

    q.push_back(a);
    q.push_front(b);

    EXPECT_EQ(q.size(), 2u);
    EXPECT_EQ(q.front().value, 20);
    EXPECT_EQ(q.back().value, 10);
}

TEST(TDequeTest, over_aligned_type) {
    TDeque<AlignedValue> q(1);

    AlignedValue a(10), b(20);
    q.push_back(a);
    q.push_back(b);

    auto p1 = reinterpret_cast<std::uintptr_t>(&q.front());

    auto p2 = reinterpret_cast<std::uintptr_t>(&q.back());

    EXPECT_EQ(p1 % alignof(AlignedValue), 0u);
    EXPECT_EQ(p2 % alignof(AlignedValue), 0u);

    EXPECT_EQ(q.front().value, 10);
    EXPECT_EQ(q.back().value, 20);
}

//проверка безопасности исключений

TEST(TDequeTest, push_back_exception_safety) {
    CopyProbe::reset();

    CopyProbe a(10), b(20), c(30);

    TDeque<CopyProbe> q(2);
    q.push_back(a);
    q.push_back(b);

    int n = CopyProbe::alive;

    CopyProbe::copies = 0;
    CopyProbe::fail_at = 3;

    EXPECT_THROW(q.push_back(c), std::runtime_error);

    CopyProbe::reset();

    EXPECT_EQ(q.size(), 2u);
    EXPECT_EQ(q.getCapacity(), 2u);
    EXPECT_EQ(q.front().value, 10);
    EXPECT_EQ(q.back().value, 20);
    EXPECT_EQ(CopyProbe::alive, n);
}

TEST(TDequeTest, push_front_exception_safety) {
    CopyProbe::reset();

    CopyProbe a(10), b(20), c(5);

    TDeque<CopyProbe> q(2);
    q.push_back(a);
    q.push_back(b);

    int n = CopyProbe::alive;

    CopyProbe::copies = 0;
    CopyProbe::fail_at = 3;

    EXPECT_THROW(q.push_front(c), std::runtime_error);

    CopyProbe::reset();

    EXPECT_EQ(q.size(), 2u);
    EXPECT_EQ(q.getCapacity(), 2u);
    EXPECT_EQ(q.front().value, 10);
    EXPECT_EQ(q.back().value, 20);
    EXPECT_EQ(CopyProbe::alive, n);
}

TEST(TDequeTest, push_without_resize_exception_safety) {
    CopyProbe::reset();

    CopyProbe a(10), b(20);

    TDeque<CopyProbe> q(4);
    q.push_back(a);

    int n = CopyProbe::alive;

    CopyProbe::copies = 0;
    CopyProbe::fail_at = 1;

    EXPECT_THROW(q.push_front(b), std::runtime_error);

    CopyProbe::reset();

    EXPECT_EQ(q.size(), 1u);
    EXPECT_EQ(q.getCapacity(), 4u);
    EXPECT_EQ(q.front().value, 10);
    EXPECT_EQ(CopyProbe::alive, n);
}

TEST(TDequeTest, copy_constructor_exception_safety) {
    CopyProbe::reset();

    TDeque<CopyProbe> q(3);
    CopyProbe a(1), b(2), c(3);

    q.push_back(a);
    q.push_back(b);
    q.push_back(c);

    int n = CopyProbe::alive;

    CopyProbe::copies = 0;
    CopyProbe::fail_at = 3;

    EXPECT_THROW(
        { TDeque<CopyProbe> q2(q); },
        std::runtime_error
    );

    CopyProbe::reset();

    EXPECT_EQ(q.size(), 3u);
    EXPECT_EQ(q.front().value, 1);
    EXPECT_EQ(q.back().value, 3);
    EXPECT_EQ(CopyProbe::alive, n);
}

TEST(TDequeTest, copy_assignment_exception_safety) {
    CopyProbe::reset();

    CopyProbe a(1), b(2), c(3), d(99);

    TDeque<CopyProbe> q(3);
    q.push_back(a);
    q.push_back(b);
    q.push_back(c);

    TDeque<CopyProbe> q2(2);
    q2.push_back(d);

    int n = CopyProbe::alive;

    CopyProbe::copies = 0;
    CopyProbe::fail_at = 3;

    EXPECT_THROW(q2 = q, std::runtime_error);

    CopyProbe::reset();

    EXPECT_EQ(q2.size(), 1u);
    EXPECT_EQ(q2.getCapacity(), 2u);
    EXPECT_EQ(q2.front().value, 99);

    EXPECT_EQ(q.size(), 3u);
    EXPECT_EQ(q.front().value, 1);
    EXPECT_EQ(q.back().value, 3);

    EXPECT_EQ(CopyProbe::alive, n);
}

//deque v2 тесты

TEST(DequeBlocksTest, destroys_multiple_blocks) {
    CopyProbe::reset();

    CopyProbe x(42);
    const int n = CopyProbe::alive;

    {
        Deque<CopyProbe> q;

        for (int i = 0; i < 80; i++)
            q.push_back(x);

        EXPECT_EQ(CopyProbe::alive, n + 80);
    }

    EXPECT_EQ(CopyProbe::alive, n);
}

TEST(DequeBlocksTest, push_front_into_existing_block) {
    Deque<int> q;

    q.push_back(10);
    q.push_front(20);
    q.push_front(30);

    EXPECT_EQ(q.size(), 3u);
    EXPECT_EQ(q.front(), 30);
    EXPECT_EQ(q.back(), 10);
}

TEST(DequeBlocksTest, push_front_across_blocks) {
    Deque<int> q;

    for (int i = 0; i < 100; i++) {
        q.push_front(i);
    }

    EXPECT_EQ(q.size(), 100u);
    EXPECT_EQ(q.front(), 99);
    EXPECT_EQ(q.back(), 0);
}

TEST(DequeBlocksTest, pop_back_across_blocks) {
    Deque<int> q;

    for (int i = 0; i < 100; i++)
        q.push_back(i);

    for (int i = 99; i >= 0; i--) {
        ASSERT_FALSE(q.empty());
        EXPECT_EQ(q.back(), i);
        q.pop_back();
    }

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0u);

    q.push_back(42);
    EXPECT_EQ(q.front(), 42);
    EXPECT_EQ(q.back(), 42);
}

TEST(DequeBlocksTest, pop_front_across_blocks) {
    Deque<int> q;

    for (int i = 0; i < 100; i++)
        q.push_back(i);

    for (int i = 0; i < 100; i++) {
        ASSERT_FALSE(q.empty());
        EXPECT_EQ(q.front(), i);
        q.pop_front();
    }

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0u);

    q.push_front(42);
    EXPECT_EQ(q.front(), 42);
    EXPECT_EQ(q.back(), 42);
}

TEST(DequeBlocksTest, pop_mixed) {
    Deque<int> q;

    for (int i = 0; i < 100; i++)
        q.push_front(i);

    for (int i = 0; i < 40; i++)
        q.pop_back();

    for (int i = 0; i < 30; i++)
        q.pop_front();

    EXPECT_EQ(q.size(), 30u);
    EXPECT_EQ(q.front(), 69);
    EXPECT_EQ(q.back(), 40);

    while (!q.empty())
        q.pop_back();

    q.pop_back();
    q.pop_front();

    EXPECT_TRUE(q.empty());
}

TEST(DequeBlocksTest, pop_destroys_elements) {
    CopyProbe::reset();
    int n = CopyProbe::alive;

    {
        CopyProbe x(10);
        Deque<CopyProbe> q;

        for (int i = 0; i < 100; i++)
            q.push_back(x);

        EXPECT_EQ(CopyProbe::alive, n + 101);

        for (int i = 0; i < 50; i++)
            q.pop_front();

        EXPECT_EQ(CopyProbe::alive, n + 51);

        for (int i = 0; i < 50; i++)
            q.pop_back();

        EXPECT_EQ(CopyProbe::alive, n + 1);
    }

    EXPECT_EQ(CopyProbe::alive, n);
}

TEST(DequeBlocksTest, clear_and_reuse) {
    Deque<int> q;

    for (int i = 0; i < 100; ++i)
        q.push_back(i);

    q.clear();

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0u);

    q.push_front(10);
    q.push_back(20);

    EXPECT_EQ(q.size(), 2u);
    EXPECT_EQ(q.front(), 10);
    EXPECT_EQ(q.back(), 20);
}

TEST(DequeBlocksTest, move_constructor) {
    Deque<int> q;

    for (int i = 0; i < 100; ++i)
        q.push_back(i);

    int* p1 = &q.front();
    int* p2 = &q.back();

    Deque<int> q2(std::move(q));

    EXPECT_EQ(q2.size(), 100u);
    EXPECT_EQ(q2.front(), 0);
    EXPECT_EQ(q2.back(), 99);

    //проверяем что объекты T не перемещались
    EXPECT_EQ(&q2.front(), p1);
    EXPECT_EQ(&q2.back(), p2);

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0u);

    //можно ли исходный контейнер использовать повторно
    q.push_back(42);
    EXPECT_EQ(q.front(), 42);
}

TEST(DequeBlocksTest, move_assignment) {
    Deque<int> q;
    for (int i = 0; i < 100; ++i)
        q.push_back(i);

    int* p = &q.front();

    Deque<int> q2;
    q2.push_back(999);
    q2.push_back(888);

    q2 = std::move(q);

    EXPECT_EQ(q2.size(), 100u);
    EXPECT_EQ(q2.front(), 0);
    EXPECT_EQ(q2.back(), 99);
    EXPECT_EQ(&q2.front(), p);

    EXPECT_TRUE(q.empty());

    q.push_back(42);
    EXPECT_EQ(q.front(), 42);

    q2 = std::move(q2);
    EXPECT_EQ(q2.size(), 100u);
    EXPECT_EQ(q2.front(), 0);
    EXPECT_EQ(q2.back(), 99);
}

TEST(DequeBlocksTest, copy_constructor_multiple_blocks) {
    Deque<int> q;

    for (int i = 0; i < 100; i++)
        q.push_back(i);

    for (int i = 0; i < 17; i++)
        q.pop_front();

    Deque<int> q2(q);

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
        q2.pop_front();
    }

    EXPECT_TRUE(q2.empty());
}

TEST(DequeBlocksTest, copy_constructor_exception_safety) {
    CopyProbe::reset();

    CopyProbe x(42);
    Deque<CopyProbe> q;

    for (int i = 0; i < 80; ++i)
        q.push_back(x);

    const int n = CopyProbe::alive;

    CopyProbe::copies = 0;
    CopyProbe::fail_at = 35;

    EXPECT_THROW({ Deque<CopyProbe> q2(q); }, std::runtime_error);

    CopyProbe::reset();

    EXPECT_EQ(q.size(), 80u);
    EXPECT_EQ(CopyProbe::alive, n);
}

TEST(DequeBlocksTest, emplace_back_immovable) {
    Deque<Immovable> q;

    Immovable& x = q.emplace_back(10);
    Immovable* p = &x;

    for (int i = 1; i <= 100; i++)
        q.emplace_back(i);

    EXPECT_EQ(q.size(), 101u);
    EXPECT_EQ(q.front().value, 10);
    EXPECT_EQ(q.back().value, 100);

    EXPECT_EQ(&q.front(), p);
    EXPECT_EQ(x.value, 10);

    q.pop_front();

    EXPECT_EQ(q.front().value, 1);
    EXPECT_EQ(q.size(), 100u);
}

TEST(DequeBlocksTest, emplace_both_ends_immovable) {
    Deque<Immovable> q;

    Immovable& x = q.emplace_back(100);
    Immovable* p = &x;

    for (int i = 1; i <= 100; i++) {
        q.emplace_front(-i);
        q.emplace_back(i);
    }

    EXPECT_EQ(q.size(), 201u);
    EXPECT_EQ(q.front().value, -100);
    EXPECT_EQ(q.back().value, 100);

    EXPECT_EQ(&x, p);
    EXPECT_EQ(x.value, 100);
}

TEST(DequeBlocksTest, push_move_only) {
    Deque<std::unique_ptr<int>> q;

    q.push_back(std::make_unique<int>(10));
    q.push_front(std::make_unique<int>(20));

    EXPECT_EQ(q.size(), 2u);
    EXPECT_EQ(*q.front(), 20);
    EXPECT_EQ(*q.back(), 10);

    auto ptr = std::make_unique<int>(30);

    q.push_back(std::move(ptr));

    EXPECT_EQ(ptr, nullptr);
    EXPECT_EQ(*q.back(), 30);
}

TEST(DequeBlocksTest, emplace_exception_safety) {
    const int n = ThrowOnConstruct::alive;

    {
        Deque<ThrowOnConstruct> q;

        for (int i = 0; i < 100; i++) {
            q.emplace_back(i);
            q.emplace_front(i + 100);
        }

        auto* p1 = &q.front();
        auto* p2 = &q.back();

        EXPECT_THROW(q.emplace_back(-1), std::runtime_error);
        EXPECT_THROW(q.emplace_front(-1), std::runtime_error);

        EXPECT_EQ(q.size(), 200u);
        EXPECT_EQ(ThrowOnConstruct::alive, n + 200);

        EXPECT_EQ(&q.front(), p1);
        EXPECT_EQ(&q.back(), p2);
    }

    EXPECT_EQ(ThrowOnConstruct::alive, n);
}
