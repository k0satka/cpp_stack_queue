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
    TStack<std::string> source;

    for (int i = 0; i < 100; i++) source.push(std::to_string(i));

    TStack<std::string> copy(source);

    EXPECT_EQ(copy.size(), 100u);
    EXPECT_EQ(copy.top(), "99");

    source.top() = "privetik";

    EXPECT_EQ(copy.top(), "99");
    EXPECT_EQ(source.top(), "privetik");

    for (int i = 99; i >= 0; i--) {
        ASSERT_FALSE(copy.empty());
        EXPECT_EQ(copy.top(), std::to_string(i));
        copy.pop();
    }

    EXPECT_TRUE(copy.empty());
}

TEST(TStackTest, copy_assignment) {
    TStack<int> source;
    for (int i = 0; i < 100; i++)
        source.push(i);

    TStack<int> target;
    target.push(999);
    target.push(888);

    target = source;

    EXPECT_EQ(target.size(), 100u);
    EXPECT_EQ(target.top(), 99);

    source.top() = 500;

    EXPECT_EQ(target.top(), 99);
    EXPECT_EQ(source.top(), 500);
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
    TStack<int> source;

    for (int i = 0; i < 100; i++)
        source.push(i);

    int* original_address = &source.top();

    TStack<int> moved(std::move(source));

    EXPECT_EQ(moved.size(), 100u);
    EXPECT_EQ(moved.top(), 99);
    EXPECT_EQ(&moved.top(), original_address);

    EXPECT_TRUE(source.empty());

    source.push(42);
    EXPECT_EQ(source.top(), 42);
}

TEST(TStackTest, move_assignment) {
    TStack<int> source;

    for (int i = 0; i < 100; i++)
        source.push(i);

    int* original_address = &source.top();

    TStack<int> target;
    target.push(1000);
    target.push(2000);

    target = std::move(source);

    EXPECT_EQ(target.size(), 100u);
    EXPECT_EQ(target.top(), 99);
    EXPECT_EQ(&target.top(), original_address);

    EXPECT_TRUE(source.empty());

    source.push(42);
    EXPECT_EQ(source.top(), 42);
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
    const int baseline = StackImmovable::alive;

    {
        TStack<StackImmovable> s;

        auto& first = s.emplace(10);
        StackImmovable* address = &first;

        for (int i = 1; i <= 100; ++i)
            s.emplace(i);

        EXPECT_EQ(s.size(), 101u);
        EXPECT_EQ(s.top().value, 100);

        EXPECT_EQ(&first, address);
        EXPECT_EQ(first.value, 10);
        EXPECT_EQ(StackImmovable::alive, baseline + 101);

        s.pop();

        EXPECT_EQ(s.top().value, 99);
        EXPECT_EQ(StackImmovable::alive, baseline + 100);
    }

    EXPECT_EQ(StackImmovable::alive, baseline);
}

//исключения

TEST(TStackTest, emplace_exception_safety) {
    const int baseline = StackImmovable::alive;

    {
        TStack<StackImmovable> s;

        for (int i = 0; i < 100; i++)
            s.emplace(i);

        auto* original_top = &s.top();

        EXPECT_THROW(s.emplace(-1), std::runtime_error);

        EXPECT_EQ(s.size(), 100u);
        EXPECT_EQ(s.top().value, 99);
        EXPECT_EQ(&s.top(), original_top);
        EXPECT_EQ(StackImmovable::alive, baseline + 100);
    }

    EXPECT_EQ(StackImmovable::alive, baseline);
}

TEST(TStackTest, push_exception_safety) {
    StackCopyProbe::reset();

    StackCopyProbe value(42);

    TStack<StackCopyProbe> s;
    s.push(value);

    const int alive_before = StackCopyProbe::alive;

    StackCopyProbe::copies = 0;
    StackCopyProbe::fail_at = 1;

    EXPECT_THROW(s.push(value), std::runtime_error);

    StackCopyProbe::reset();

    EXPECT_EQ(s.size(), 1u);
    EXPECT_EQ(s.top().value, 42);
    EXPECT_EQ(StackCopyProbe::alive, alive_before);
}

TEST(TStackTest, copy_constructor_exception_safety) {
    StackCopyProbe::reset();

    const int baseline = StackCopyProbe::alive;

    {
        StackCopyProbe value(42);
        TStack<StackCopyProbe> source;

        for (int i = 0; i < 80; ++i)
            source.push(value);

        const int alive_before = StackCopyProbe::alive;

        StackCopyProbe::copies = 0;
        StackCopyProbe::fail_at = 35;

        EXPECT_THROW({ TStack<StackCopyProbe> copy(source); }, std::runtime_error);

        StackCopyProbe::reset();

        EXPECT_EQ(source.size(), 80u);
        EXPECT_EQ(source.top().value, 42);
        EXPECT_EQ(StackCopyProbe::alive, alive_before);
    }

    EXPECT_EQ(StackCopyProbe::alive, baseline);
}

TEST(TStackTest, copy_assignment_exception_safety) {
    StackCopyProbe::reset();

    StackCopyProbe value(42);
    StackCopyProbe old_value(99);

    TStack<StackCopyProbe> source;
    for (int i = 0; i < 80; ++i)
        source.push(value);

    TStack<StackCopyProbe> target;
    target.push(old_value);

    const int alive_before = StackCopyProbe::alive;

    StackCopyProbe::copies = 0;
    StackCopyProbe::fail_at = 35;

    EXPECT_THROW(target = source, std::runtime_error);

    StackCopyProbe::reset();

    EXPECT_EQ(target.size(), 1u);
    EXPECT_EQ(target.top().value, 99);

    EXPECT_EQ(source.size(), 80u);
    EXPECT_EQ(source.top().value, 42);

    EXPECT_EQ(StackCopyProbe::alive, alive_before);
}