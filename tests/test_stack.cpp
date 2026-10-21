#include <gtest/gtest.h>
#include "tstack.h"

TEST(TStackTest, default_constructor) {
    TStack<int> s;

    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0u);
    EXPECT_EQ(s.getCapacity(), 0u);
    EXPECT_THROW(s.top(), std::out_of_range);
}

TEST(TStackTest, push) {
    TStack<int> s(3u);
    s.push(1); s.push(2); s.push(3);

    EXPECT_EQ(s.size(), 3u);
    EXPECT_EQ(s.getCapacity(), 3u);
    EXPECT_EQ(s.top(), 3);
}

TEST(TStackTest, push_with_resize) {
    TStack<int> s(3u);
    s.push(1); s.push(2); s.push(3); s.push(4);

    EXPECT_EQ(s.size(), 4u);
    EXPECT_EQ(s.getCapacity(), 6u);
    EXPECT_EQ(s.top(), 4);
}

TEST(TStackTest, push_top_with_resize) {
    TStack<int> s(1u);
    s.push(10);

    s.push(s.top());

    EXPECT_EQ(s.size(), 2u);
    EXPECT_EQ(s.getCapacity(), 2u);
    EXPECT_EQ(s.top(), 10);
    s.pop();
    EXPECT_EQ(s.top(), 10);
}

TEST(TStackTest, pop) {
    TStack<int> s(4u);
    s.push(1); s.push(2); s.push(3); s.push(4);

    s.pop();
    EXPECT_EQ(s.size(), 3u);
    EXPECT_EQ(s.getCapacity(), 4u);
    EXPECT_EQ(s.top(), 3);

    s.pop();
    EXPECT_EQ(s.size(), 2u);
    EXPECT_EQ(s.getCapacity(), 4u);
    EXPECT_EQ(s.top(), 2);

    s.pop();
    EXPECT_EQ(s.size(), 1u);
    EXPECT_EQ(s.getCapacity(), 2u);
    EXPECT_EQ(s.top(), 1);

    s.pop();
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0u);
    EXPECT_EQ(s.getCapacity(), 0u);
    EXPECT_THROW(s.top(), std::out_of_range);
}

TEST(TStackTest, pop_empty) {
    TStack<int> s;

    s.pop();

    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0u);
    EXPECT_EQ(s.getCapacity(), 0u);
}

TEST(TStackTest, copy_constructor) {
    TStack<int> s(3u);
    s.push(1); s.push(2); s.push(3);

    TStack<int> copy(s);
    s.top() = 10;
    s.push(4);

    EXPECT_EQ(copy.size(), 3u);
    EXPECT_EQ(copy.getCapacity(), 3u);
    EXPECT_EQ(copy.top(), 3);
    copy.pop();
    EXPECT_EQ(copy.top(), 2);
    copy.pop();
    EXPECT_EQ(copy.top(), 1);
}

TEST(TStackTest, copy_assignment) {
    TStack<int> s(3u);
    s.push(1); s.push(2); s.push(3);

    TStack<int> copy(2u);
    copy.push(10); copy.push(20);
    copy = s;
    s.top() = 30;

    EXPECT_EQ(copy.size(), 3u);
    EXPECT_EQ(copy.getCapacity(), 3u);
    EXPECT_EQ(copy.top(), 3);
    copy.pop();
    EXPECT_EQ(copy.top(), 2);
    copy.pop();
    EXPECT_EQ(copy.top(), 1);
}

TEST(TStackTest, self_assignment) {
    TStack<int> s(3u);
    s.push(1); s.push(2); s.push(3);

    s = s;

    EXPECT_EQ(s.size(), 3u);
    EXPECT_EQ(s.getCapacity(), 3u);
    EXPECT_EQ(s.top(), 3);
    s.pop();
    EXPECT_EQ(s.top(), 2);
    s.pop();
    EXPECT_EQ(s.top(), 1);
}

TEST(TStackTest, clear) {
    TStack<int> s(3u);
    s.push(1); s.push(2); s.push(3);

    s.clear();

    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0u);
    EXPECT_EQ(s.getCapacity(), 0u);

    s.push(4);
    EXPECT_EQ(s.size(), 1u);
    EXPECT_EQ(s.top(), 4);
}

TEST(TStackTest, top_reference) {
    TStack<int> s;
    s.push(1);

    s.top() = 2;
    EXPECT_EQ(s.top(), 2);

    const TStack<int>& const_s = s;
    EXPECT_EQ(const_s.top(), 2);
}

