#include <gtest/gtest.h>
#include "tstack.h"

//дополнительные типы
struct StackImmovable {
    inline static int alive = 0;

    int value;

    explicit StackImmovable(int v) : value(v) {
        if (v < 0) throw std::runtime_error("construction failed");
        ++alive;
    }

    StackImmovable(const StackImmovable&) = delete;
    StackImmovable(StackImmovable&&) = delete;

    StackImmovable& operator=(const StackImmovable&) = delete;
    StackImmovable& operator=(StackImmovable&&) = delete;

    ~StackImmovable() noexcept { --alive; }
};

struct StackCopyProbe {
    inline static int alive = 0;
    inline static int copies = 0;
    inline static int fail_at = 0;

    int value;

    explicit StackCopyProbe(int v) : value(v) { ++alive; }

    StackCopyProbe(const StackCopyProbe& other) : value(other.value) {
        ++copies;
        if (fail_at != 0 && copies == fail_at) throw std::runtime_error("copy failed");
        ++alive;
    }

    StackCopyProbe(StackCopyProbe&&) = delete;
    StackCopyProbe& operator=(const StackCopyProbe&) = delete;

    ~StackCopyProbe() noexcept { --alive; }

    static void reset() {
        copies = 0;
        fail_at = 0;
    }
};

//бвзовые операции

TEST(TStackTest, default_constructor) {
    TStack<int> s;

    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0u);
    EXPECT_THROW(s.top(), std::out_of_range);

    EXPECT_NO_THROW(s.pop());
}

TEST(TStackTest, lifo_order) {
    TStack<int> s;

    for (int i = 0; i < 100; i++)
        s.push(i);

    EXPECT_EQ(s.size(), 100u);
    EXPECT_EQ(s.top(), 99);

    for (int i = 99; i >= 0; i--) {
        ASSERT_FALSE(s.empty());
        EXPECT_EQ(s.top(), i);
        s.pop();
    }

    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0u);
}

TEST(TStackTest, top_reference) {
    TStack<int> s;

    s.push(10);
    s.push(20);

    s.top() = 100;

    EXPECT_EQ(s.top(), 100);

    const TStack<int>& cs = s;

    EXPECT_EQ(cs.top(), 100);

    s.pop();
    EXPECT_EQ(s.top(), 10);
}

TEST(TStackTest, clear_and_reuse) {
    TStack<int> s;

    for (int i = 0; i < 100; ++i) s.push(i);

    s.clear();

    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.size(), 0u);
    EXPECT_THROW(s.top(), std::out_of_range);

    s.push(42);

    EXPECT_EQ(s.size(), 1u);
    EXPECT_EQ(s.top(), 42);

    s.clear();
    s.clear();

    EXPECT_TRUE(s.empty());
}

TEST(TStackTest, push_own_top) {
    TStack<std::string> s;

    s.push("privetik");

    for (int i = 0; i < 100; i++)
        s.push(s.top());

    EXPECT_EQ(s.size(), 101u);
    EXPECT_EQ(s.top(), "privetik");

    while (!s.empty()) {
        EXPECT_EQ(s.top(), "privetik");
        s.pop();
    }
}

//rule five

TEST(TStackTest, copy_constructor) {
    TStack<std::string> s;

    for (int i = 0; i < 100; i++) s.push(std::to_string(i));

    TStack<std::string> s2(s);

    EXPECT_EQ(s2.size(), 100u);
    EXPECT_EQ(s2.top(), "99");

    s.top() = "privetik";

    EXPECT_EQ(s2.top(), "99");
    EXPECT_EQ(s.top(), "privetik");

    for (int i = 99; i >= 0; i--) {
        ASSERT_FALSE(s2.empty());
        EXPECT_EQ(s2.top(), std::to_string(i));
        s2.pop();
    }

    EXPECT_TRUE(s2.empty());
}

TEST(TStackTest, copy_assignment) {
    TStack<int> s;
    for (int i = 0; i < 100; i++)
        s.push(i);

    TStack<int> s2;
    s2.push(999);
    s2.push(888);

    s2 = s;

    EXPECT_EQ(s2.size(), 100u);
    EXPECT_EQ(s2.top(), 99);

    s.top() = 500;

    EXPECT_EQ(s2.top(), 99);
    EXPECT_EQ(s.top(), 500);
}

TEST(TStackTest, self_copy_assignment) {
    TStack<int> s;

    s.push(10);
    s.push(20);

    s = s;

    EXPECT_EQ(s.size(), 2u);
    EXPECT_EQ(s.top(), 20);

    s.pop();
    EXPECT_EQ(s.top(), 10);
}

TEST(TStackTest, move_constructor) {
    TStack<int> s;

    for (int i = 0; i < 100; i++)
        s.push(i);

    int* p = &s.top();

    TStack<int> s2(std::move(s));

    EXPECT_EQ(s2.size(), 100u);
    EXPECT_EQ(s2.top(), 99);
    EXPECT_EQ(&s2.top(), p);

    EXPECT_TRUE(s.empty());

    s.push(42);
    EXPECT_EQ(s.top(), 42);
}

TEST(TStackTest, move_assignment) {
    TStack<int> s;

    for (int i = 0; i < 100; i++)
        s.push(i);

    int* p = &s.top();

    TStack<int> s2;
    s2.push(1000);
    s2.push(2000);

    s2 = std::move(s);

    EXPECT_EQ(s2.size(), 100u);
    EXPECT_EQ(s2.top(), 99);
    EXPECT_EQ(&s2.top(), p);

    EXPECT_TRUE(s.empty());

    s.push(42);
    EXPECT_EQ(s.top(), 42);
}

TEST(TStackTest, self_move_assignment) {
    TStack<int> s;

    s.push(10);
    s.push(20);

    s = std::move(s);

    EXPECT_EQ(s.size(), 2u);
    EXPECT_EQ(s.top(), 20);
}

//странные типы

TEST(TStackTest, push_move_only) {
    TStack<std::unique_ptr<int>> s;

    s.push(std::make_unique<int>(10));
    s.push(std::make_unique<int>(20));

    EXPECT_EQ(s.size(), 2u);
    EXPECT_EQ(*s.top(), 20);

    auto ptr = std::make_unique<int>(30);
    s.push(std::move(ptr));

    EXPECT_EQ(ptr, nullptr);
    EXPECT_EQ(*s.top(), 30);

    s.pop();
    EXPECT_EQ(*s.top(), 20);

    s.pop();
    EXPECT_EQ(*s.top(), 10);
}

TEST(TStackTest, emplace_immovable) {
    const int n = StackImmovable::alive;

    {
        TStack<StackImmovable> s;

        auto& x = s.emplace(10);
        StackImmovable* p = &x;

        for (int i = 1; i <= 100; ++i)
            s.emplace(i);

        EXPECT_EQ(s.size(), 101u);
        EXPECT_EQ(s.top().value, 100);

        EXPECT_EQ(&x, p);
        EXPECT_EQ(x.value, 10);
        EXPECT_EQ(StackImmovable::alive, n + 101);

        s.pop();

        EXPECT_EQ(s.top().value, 99);
        EXPECT_EQ(StackImmovable::alive, n + 100);
    }

    EXPECT_EQ(StackImmovable::alive, n);
}

//исключения

TEST(TStackTest, emplace_exception_safety) {
    const int n = StackImmovable::alive;

    {
        TStack<StackImmovable> s;

        for (int i = 0; i < 100; i++)
            s.emplace(i);

        auto* p = &s.top();

        EXPECT_THROW(s.emplace(-1), std::runtime_error);

        EXPECT_EQ(s.size(), 100u);
        EXPECT_EQ(s.top().value, 99);
        EXPECT_EQ(&s.top(), p);
        EXPECT_EQ(StackImmovable::alive, n + 100);
    }

    EXPECT_EQ(StackImmovable::alive, n);
}

TEST(TStackTest, push_exception_safety) {
    StackCopyProbe::reset();

    StackCopyProbe x(42);

    TStack<StackCopyProbe> s;
    s.push(x);

    const int n = StackCopyProbe::alive;

    StackCopyProbe::copies = 0;
    StackCopyProbe::fail_at = 1;

    EXPECT_THROW(s.push(x), std::runtime_error);

    StackCopyProbe::reset();

    EXPECT_EQ(s.size(), 1u);
    EXPECT_EQ(s.top().value, 42);
    EXPECT_EQ(StackCopyProbe::alive, n);
}

TEST(TStackTest, copy_constructor_exception_safety) {
    StackCopyProbe::reset();

    const int n = StackCopyProbe::alive;

    {
        StackCopyProbe x(42);
        TStack<StackCopyProbe> s;

        for (int i = 0; i < 80; ++i)
            s.push(x);

        const int n2 = StackCopyProbe::alive;

        StackCopyProbe::copies = 0;
        StackCopyProbe::fail_at = 35;

        EXPECT_THROW({ TStack<StackCopyProbe> s2(s); }, std::runtime_error);

        StackCopyProbe::reset();

        EXPECT_EQ(s.size(), 80u);
        EXPECT_EQ(s.top().value, 42);
        EXPECT_EQ(StackCopyProbe::alive, n2);
    }

    EXPECT_EQ(StackCopyProbe::alive, n);
}

TEST(TStackTest, copy_assignment_exception_safety) {
    StackCopyProbe::reset();

    StackCopyProbe x(42);
    StackCopyProbe y(99);

    TStack<StackCopyProbe> s;
    for (int i = 0; i < 80; ++i)
        s.push(x);

    TStack<StackCopyProbe> s2;
    s2.push(y);

    const int n = StackCopyProbe::alive;

    StackCopyProbe::copies = 0;
    StackCopyProbe::fail_at = 35;

    EXPECT_THROW(s2 = s, std::runtime_error);

    StackCopyProbe::reset();

    EXPECT_EQ(s2.size(), 1u);
    EXPECT_EQ(s2.top().value, 99);

    EXPECT_EQ(s.size(), 80u);
    EXPECT_EQ(s.top().value, 42);

    EXPECT_EQ(StackCopyProbe::alive, n);
}
