#pragma once
#include <stdexcept>
#include "tdeque.h"

template<typename T>
class TStack {
private:
    Deque<T> data;

public:
    TStack() = default;

    void push(const T& a) {
        data.push_back(a);
    }
    void push(T&& a) {
        data.push_back(std::move(a));
    }

    template<typename... Args>
    T& emplace(Args&&... args) {
        return data.emplace_back(std::forward<Args>(args)...);
    }

    void pop() {
        data.pop_back();
    }

    T& top() {
        return data.back();
    }
    const T& top() const {
        return data.back();
    }

    void clear() {
        data.clear();
    }

    bool empty() const {
        return data.empty();
    }
    size_t size() const {
        return data.size();
    }
};
