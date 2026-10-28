#include <gtest/gtest.h>
#include "tqueue.h"

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
    for (int i = 1; i <= 4; ++i)
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
    for (int i = 1; i <= 4; ++i)
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
    for (int i = 1; i <= 4; ++i)
        q.push_back(i);

    q.pop_front();
    q.pop_front();

    q.push_back(5);
    q.push_back(6);
    q.push_back(7);

    EXPECT_EQ(q.size(), 5u);
    EXPECT_EQ(q.getCapacity(), 5u);

    for (int i = 3; i <= 7; ++i) {
        ASSERT_FALSE(q.empty());
        EXPECT_EQ(q.front(), i);
        q.pop_front();
    }

    EXPECT_TRUE(q.empty());
}

TEST(TDequeTest, resize_back_wrapped) {
    TDeque<int> q(4);
    for (int i = 1; i <= 4; ++i)
        q.push_back(i);

    q.pop_front();
    q.pop_front();

    q.push_back(5);
    q.push_back(6);
    q.push_back(7);

    EXPECT_EQ(q.size(), 5u);
    EXPECT_EQ(q.getCapacity(), 8u);

    for (int i = 3; i <= 7; ++i) {
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

    for (int i = 0; i <= 4; ++i) {
        ASSERT_FALSE(q.empty());
        EXPECT_EQ(q.front(), i);
        q.pop_front();
    }

    EXPECT_TRUE(q.empty());
}

//rule 5

TEST(TDequeTest, copy_constructor) {
    TDeque<int> q(4);
    for (int i = 1; i <= 4; ++i)
        q.push_back(i);

    q.pop_front();
    q.push_back(5);

    TDeque<int> copy(q);

    q.front() = 20;
    q.back() = 50;

    EXPECT_EQ(copy.size(), 4u);
    EXPECT_EQ(copy.getCapacity(), 4u);

    for (int i = 2; i <= 5; ++i) {
        ASSERT_FALSE(copy.empty());
        EXPECT_EQ(copy.front(), i);
        copy.pop_front();
    }

    EXPECT_EQ(q.front(), 20);
    EXPECT_EQ(q.back(), 50);
}

TEST(TDequeTest, copy_assignment) {
    TDeque<int> q(4);
    for (int i = 1; i <= 4; ++i)
        q.push_back(i);

    q.pop_front();
    q.push_back(5);

    TDeque<int> copy(2);
    copy.push_back(10);
    copy.push_back(20);

    copy = q;

    q.front() = 20;
    q.back() = 50;

    EXPECT_EQ(copy.size(), 4u);
    EXPECT_EQ(copy.getCapacity(), 4u);

    for (int i = 2; i <= 5; ++i) {
        ASSERT_FALSE(copy.empty());
        EXPECT_EQ(copy.front(), i);
        copy.pop_front();
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

    for (int i = 1; i <= 3; ++i) {
        ASSERT_FALSE(q.empty());
        EXPECT_EQ(q.front(), i);
        q.pop_front();
    }
}

TEST(TDequeTest, copy_empty) {
    TDeque<int> empty;
    TDeque<int> copy(empty);

    EXPECT_TRUE(copy.empty());
    EXPECT_EQ(copy.getCapacity(), 0u);

    TDeque<int> target(5);
    target.push_back(10);
    target = empty;

    EXPECT_TRUE(target.empty());
    EXPECT_EQ(target.getCapacity(), 0u);

    TDeque<int> reserved(7);
    TDeque<int> reserved_copy(reserved);

    EXPECT_TRUE(reserved_copy.empty());
    EXPECT_EQ(reserved_copy.getCapacity(), 7u);
}

TEST(TDequeTest, move_constructor) {
    TDeque<int> q(5);
    q.push_back(1);
    q.push_back(2);
    q.push_back(3);
    q.pop_front();

    TDeque<int> moved(std::move(q));

    EXPECT_EQ(moved.size(), 2u);
    EXPECT_EQ(moved.getCapacity(), 5u);
    EXPECT_EQ(moved.front(), 2);
    EXPECT_EQ(moved.back(), 3);

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.getCapacity(), 0u);

    q.push_back(42);

    EXPECT_EQ(q.front(), 42);
    EXPECT_EQ(moved.front(), 2);
}

TEST(TDequeTest, move_assignment) {
    TDeque<int> q(3);
    q.push_back(1);
    q.push_back(2);

    TDeque<int> target(4);
    target.push_back(42);
    target.push_back(142);

    target = std::move(q);

    EXPECT_EQ(target.size(), 2u);
    EXPECT_EQ(target.getCapacity(), 3u);
    EXPECT_EQ(target.front(), 1);
    EXPECT_EQ(target.back(), 2);

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.getCapacity(), 0u);

    q.push_front(5);

    EXPECT_EQ(q.front(), 5);
    EXPECT_EQ(target.front(), 1);
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
        TDeque<int> source(3);
        source.push_back(10);
        source.push_back(20);

        q = std::move(source);
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
    int baseline = CopyProbe::alive;

    {
        TDeque<CopyProbe> q(4);

        EXPECT_EQ(CopyProbe::alive, baseline);

        CopyProbe value(10);

        q.push_back(value);
        q.push_front(value);

        EXPECT_EQ(q.size(), 2u);
        EXPECT_EQ(CopyProbe::alive, baseline + 3);

        q.pop_front();
        EXPECT_EQ(CopyProbe::alive, baseline + 2);

        q.pop_back();
        EXPECT_TRUE(q.empty());
        EXPECT_EQ(q.getCapacity(), 4u);
        EXPECT_EQ(CopyProbe::alive, baseline + 1);

        q.push_back(value);
        EXPECT_EQ(q.front().value, 10);

        q.clear();
        EXPECT_EQ(CopyProbe::alive, baseline + 1);
    }

    EXPECT_EQ(CopyProbe::alive, baseline);
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

    auto front_address =
        reinterpret_cast<std::uintptr_t>(&q.front());

    auto back_address =
        reinterpret_cast<std::uintptr_t>(&q.back());

    EXPECT_EQ(front_address % alignof(AlignedValue), 0u);
    EXPECT_EQ(back_address % alignof(AlignedValue), 0u);

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

    int alive_before = CopyProbe::alive;

    CopyProbe::copies = 0;
    CopyProbe::fail_at = 3;

    EXPECT_THROW(q.push_back(c), std::runtime_error);

    CopyProbe::reset();

    EXPECT_EQ(q.size(), 2u);
    EXPECT_EQ(q.getCapacity(), 2u);
    EXPECT_EQ(q.front().value, 10);
    EXPECT_EQ(q.back().value, 20);
    EXPECT_EQ(CopyProbe::alive, alive_before);
}

TEST(TDequeTest, push_front_exception_safety) {
    CopyProbe::reset();

    CopyProbe a(10), b(20), c(5);

    TDeque<CopyProbe> q(2);
    q.push_back(a);
    q.push_back(b);

    int alive_before = CopyProbe::alive;

    CopyProbe::copies = 0;
    CopyProbe::fail_at = 3;

    EXPECT_THROW(q.push_front(c), std::runtime_error);

    CopyProbe::reset();

    EXPECT_EQ(q.size(), 2u);
    EXPECT_EQ(q.getCapacity(), 2u);
    EXPECT_EQ(q.front().value, 10);
    EXPECT_EQ(q.back().value, 20);
    EXPECT_EQ(CopyProbe::alive, alive_before);
}

TEST(TDequeTest, push_without_resize_exception_safety) {
    CopyProbe::reset();

    CopyProbe a(10), b(20);

    TDeque<CopyProbe> q(4);
    q.push_back(a);

    int alive_before = CopyProbe::alive;

    CopyProbe::copies = 0;
    CopyProbe::fail_at = 1;

    EXPECT_THROW(q.push_front(b), std::runtime_error);

    CopyProbe::reset();

    EXPECT_EQ(q.size(), 1u);
    EXPECT_EQ(q.getCapacity(), 4u);
    EXPECT_EQ(q.front().value, 10);
    EXPECT_EQ(CopyProbe::alive, alive_before);
}

TEST(TDequeTest, copy_constructor_exception_safety) {
    CopyProbe::reset();

    TDeque<CopyProbe> q(3);
    CopyProbe a(1), b(2), c(3);

    q.push_back(a);
    q.push_back(b);
    q.push_back(c);

    int alive_before = CopyProbe::alive;

    CopyProbe::copies = 0;
    CopyProbe::fail_at = 3;

    EXPECT_THROW(
        { TDeque<CopyProbe> copy(q); },
        std::runtime_error
    );

    CopyProbe::reset();

    EXPECT_EQ(q.size(), 3u);
    EXPECT_EQ(q.front().value, 1);
    EXPECT_EQ(q.back().value, 3);
    EXPECT_EQ(CopyProbe::alive, alive_before);
}

TEST(TDequeTest, copy_assignment_exception_safety) {
    CopyProbe::reset();

    CopyProbe a(1), b(2), c(3), old(99);

    TDeque<CopyProbe> source(3);
    source.push_back(a);
    source.push_back(b);
    source.push_back(c);

    TDeque<CopyProbe> target(2);
    target.push_back(old);

    int alive_before = CopyProbe::alive;

    CopyProbe::copies = 0;
    CopyProbe::fail_at = 3;

    EXPECT_THROW(target = source, std::runtime_error);

    CopyProbe::reset();

    EXPECT_EQ(target.size(), 1u);
    EXPECT_EQ(target.getCapacity(), 2u);
    EXPECT_EQ(target.front().value, 99);

    EXPECT_EQ(source.size(), 3u);
    EXPECT_EQ(source.front().value, 1);
    EXPECT_EQ(source.back().value, 3);

    EXPECT_EQ(CopyProbe::alive, alive_before);
}