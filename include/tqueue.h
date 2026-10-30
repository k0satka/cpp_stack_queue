#pragma once
#include <stdexcept>
#include "tdeque.h"

template<typename T>
class TQueue {
private:
    Deque<T> data;

public:
    TQueue() = default;

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
        data.pop_front();
    }

    T& front() {
        return data.front();
    }
    const T& front() const {
        return data.front();
    }

    T& back() {
        return data.back();
    }
    const T& back() const {
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