#include <gtest/gtest.h>
#include "tqueue.h"

TEST(TDequeTest, default_constructor) {
    TDeque<int> q;

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0u);
    EXPECT_EQ(q.getCapacity(), 0u);
    EXPECT_THROW(q.front(), std::out_of_range);
    EXPECT_THROW(q.back(), std::out_of_range);
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
    EXPECT_EQ(q.size(), 0u);
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

TEST(TDequeTest, push_back) {
    TDeque<int> q(3u);
    q.push_back(1); q.push_back(2); q.push_back(3);

    EXPECT_EQ(q.size(), 3u);
    EXPECT_EQ(q.getCapacity(), 3u);
    EXPECT_EQ(q.front(), 1);
    EXPECT_EQ(q.back(), 3);
}

TEST(TDequeTest, push_alias_with_resize) {
    TDeque<int> q1(2);
    q1.push_back(1);
    q1.push_back(2);

    q1.push_back(q1.front());

    EXPECT_EQ(q1.size(), 3u);
    EXPECT_EQ(q1.front(), 1);
    EXPECT_EQ(q1.back(), 1);

    TDeque<int> q2(2);
    q2.push_back(1);
    q2.push_back(2);

    q2.push_front(q2.back());

    EXPECT_EQ(q2.size(), 3u);
    EXPECT_EQ(q2.front(), 2);
    EXPECT_EQ(q2.back(), 2);
}

TEST(TDequeTest, push_front) {
    TDeque<int> q(3u);
    q.push_front(1); q.push_front(2); q.push_front(3);

    EXPECT_EQ(q.size(), 3u);
    EXPECT_EQ(q.getCapacity(), 3u);
    EXPECT_EQ(q.front(), 3);
    EXPECT_EQ(q.back(), 1);
}

TEST(TDequeTest, push_exception_safety) {
    ThrowOnCopy::throw_on_assign = false;

    TDeque<ThrowOnCopy> q(2);
    q.push_back(ThrowOnCopy(1));
    q.push_back(ThrowOnCopy(2));

    int alive_before = ThrowOnCopy::alive;

    ThrowOnCopy::throw_on_assign = true;

    EXPECT_THROW(
        q.push_back(ThrowOnCopy(3)),
        std::runtime_error
    );

    EXPECT_THROW(
        q.push_front(ThrowOnCopy(5)),
        std::runtime_error
    );

    ThrowOnCopy::throw_on_assign = false;

    EXPECT_EQ(q.size(), 2u);
    EXPECT_EQ(q.getCapacity(), 2u);
    EXPECT_EQ(q.front().value, 1);
    EXPECT_EQ(q.back().value, 2);
    EXPECT_EQ(ThrowOnCopy::alive, alive_before);
}

TEST(TDequeTest, pop_back) {
    TDeque<int> q(4u);
    q.push_back(1); q.push_back(2); q.push_back(3); q.push_back(4);

    q.pop_back();
    EXPECT_EQ(q.size(), 3u);
    EXPECT_EQ(q.front(), 1);
    EXPECT_EQ(q.back(), 3);

    q.pop_back();
    EXPECT_EQ(q.size(), 2u);
    EXPECT_EQ(q.back(), 2);
}

TEST(TDequeTest, pop_front) {
    TDeque<int> q(4u);
    q.push_back(1); q.push_back(2); q.push_back(3); q.push_back(4);

    q.pop_front();
    EXPECT_EQ(q.size(), 3u);
    EXPECT_EQ(q.front(), 2);
    EXPECT_EQ(q.back(), 4);

    q.pop_front();
    EXPECT_EQ(q.size(), 2u);
    EXPECT_EQ(q.front(), 3);
}

TEST(TDequeTest, circular_buffer) {
    TDeque<int> q(5u);
    q.push_back(1); q.push_back(2); q.push_back(3); q.push_back(4);
    q.pop_front(); q.pop_front();

    q.push_back(5); q.push_back(6); q.push_back(7);

    EXPECT_EQ(q.size(), 5u);
    EXPECT_EQ(q.getCapacity(), 5u);
    for (int i = 3; i < 8; i++) {
        EXPECT_EQ(q.front(), i);
        q.pop_front();
    }
    EXPECT_TRUE(q.empty());
}

TEST(TDequeTest, resize) {
    TDeque<int> q(4u);
    q.push_back(1); q.push_back(2); q.push_back(3); q.push_back(4);
    q.pop_front(); q.pop_front();
    q.push_back(5); q.push_back(6);

    q.push_back(7);

    EXPECT_EQ(q.size(), 5u);
    EXPECT_EQ(q.getCapacity(), 8u);
    for (int i = 3; i < 8; i++) {
        EXPECT_EQ(q.front(), i);
        q.pop_front();
    }
    EXPECT_TRUE(q.empty());
}

TEST(TDequeTest, copy_constructor) {
    TDeque<int> q(4u);
    q.push_back(1); q.push_back(2); q.push_back(3); q.push_back(4);
    q.pop_front();
    q.push_back(5);

    TDeque<int> copy(q);
    q.front() = 20;
    q.back() = 50;

    EXPECT_EQ(copy.size(), 4u);
    EXPECT_EQ(copy.getCapacity(), 4u);
    for (int i = 2; i < 6; i++) {
        EXPECT_EQ(copy.front(), i);
        copy.pop_front();
    }
}

TEST(TDequeTest, copy_assignment) {
    TDeque<int> q(4u);
    q.push_back(1); q.push_back(2); q.push_back(3); q.push_back(4);
    q.pop_front();
    q.push_back(5);

    TDeque<int> copy(2u);
    copy.push_back(10); copy.push_back(20);
    copy = q;
    q.front() = 20;
    q.back() = 50;

    EXPECT_EQ(copy.size(), 4u);
    EXPECT_EQ(copy.getCapacity(), 4u);
    for (int i = 2; i < 6; i++) {
        EXPECT_EQ(copy.front(), i);
        copy.pop_front();
    }
}

TEST(TDequeTest, self_assignment) {
    TDeque<int> q(4u);
    q.push_back(1); q.push_back(2); q.push_back(3); q.push_back(4);
    q.pop_front();
    q.push_back(5);

    q = q;

    EXPECT_EQ(q.size(), 4u);
    EXPECT_EQ(q.getCapacity(), 4u);
    for (int i = 2; i < 6; i++) {
        EXPECT_EQ(q.front(), i);
        q.pop_front();
    }
}

TEST(TDequeTest, clear) {
    TDeque<int> q(3u);
    q.push_back(1); q.push_back(2); q.push_back(3);

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

TEST(TDequeTest, empty_operations) {
    TDeque<int> q(3u);

    q.pop_front();
    q.pop_back();

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0u);
    EXPECT_EQ(q.getCapacity(), 3u);
    EXPECT_THROW(q.front(), std::out_of_range);
    EXPECT_THROW(q.back(), std::out_of_range);
}

TEST(TDequeTest, checking_restrictions_T) { //изучаю поведение при необычных T
    //1
    struct NoDefault {
        int value;

        explicit NoDefault(int v) : value(v) {}
    };

    TDeque<NoDefault> q; //можно, но q(1) ошибка компиляции тк new T[]() не сможет использовать конструктор по умолчанию

    //2
    struct NonAssignable {
        int value = 0;

        NonAssignable() = default;
        NonAssignable(const NonAssignable&) = default;

        NonAssignable& operator=(const NonAssignable&) = delete;
    };

    TDeque<NonAssignable> q1(3); //тут можем тк NonAssignable имеет деф. конструктор

    //q1.push_back(NonAssignable{}); //нельзя, тк нет копир. присваивания
    //а если копирование есть но может выдать throw? -может быть утечка памяти см. тест copy_constructor_exception_safety

    //3
    TDeque<Tracked> q2(3);

    q2.push_back(Tracked{});
    EXPECT_EQ(q2.size(), 1u);
    EXPECT_EQ(Tracked::alive, 3u);

    q2.push_back(Tracked{});
    EXPECT_EQ(q2.size(), 2u);
    EXPECT_EQ(Tracked::alive, 3u);

    q2.pop_front();
    EXPECT_EQ(q2.size(), 1u);
    EXPECT_EQ(Tracked::alive, 3u);

    q2.clear();
    EXPECT_TRUE(q2.empty());
    EXPECT_EQ(Tracked::alive, 0u);
}

//проверяем и исправляем утечку если присваив копир в T выдаст throw
TEST(TDequeTest, resize_exception_safety) { 
    TDeque<ThrowOnCopy> q(2);
    q.push_back(ThrowOnCopy(1));
    q.push_back(ThrowOnCopy(1));
    EXPECT_EQ(ThrowOnCopy::alive, 2u);

    ThrowOnCopy::throw_on_assign = true;
    EXPECT_TRUE(ThrowOnCopy::throw_on_assign);

    EXPECT_THROW(q.push_back(ThrowOnCopy(1)), std::runtime_error); //ключевой момент
    ThrowOnCopy::throw_on_assign = false;

    EXPECT_EQ(q.size(), 2u);
    EXPECT_EQ(q.getCapacity(), 2u);
    EXPECT_EQ(q.front().value, 1);
    EXPECT_EQ(q.back().value, 1);
    EXPECT_EQ(ThrowOnCopy::alive, 2u);
}

TEST(TDequeTest, copy_constructor_exception_safety) {
    TDeque<ThrowOnCopy> q(2);
    q.push_back(ThrowOnCopy(1));
    q.push_back(ThrowOnCopy(1));
    EXPECT_EQ(ThrowOnCopy::alive, 2u);

    ThrowOnCopy::throw_on_assign = true;
    EXPECT_TRUE(ThrowOnCopy::throw_on_assign);

    EXPECT_THROW(TDeque<ThrowOnCopy> q1(q), std::runtime_error); //?
    ThrowOnCopy::throw_on_assign = false;

    EXPECT_EQ(ThrowOnCopy::alive, 2u);
}

TEST(TDequeTest, assigning_copy_exception_safety) {
    TDeque<ThrowOnCopy> q(2);
    q.push_back(ThrowOnCopy(1));
    q.push_back(ThrowOnCopy(1));
    EXPECT_EQ(ThrowOnCopy::alive, 2u);

    ThrowOnCopy::throw_on_assign = true;
    EXPECT_TRUE(ThrowOnCopy::throw_on_assign);

    TDeque<ThrowOnCopy> q1(3);
    EXPECT_THROW(q1 = q, std::runtime_error);
    ThrowOnCopy::throw_on_assign = false;

    EXPECT_EQ(q1.size(), 0u);
    EXPECT_EQ(q1.getCapacity(), 3u);
    EXPECT_EQ(ThrowOnCopy::alive, 5u);
}