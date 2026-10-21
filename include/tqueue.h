#pragma once
#include <stdexcept>

const size_t FACTOR = 2;

template<typename T>
class TDeque {
private:
    T* data;
    size_t n;
    size_t capacity;
    size_t head;

    void resize(size_t cap) {
        assert(cap >= n);
        T* tmp = cap == 0 ? nullptr : new T[cap]();
        for (size_t i = 0; i < n; i++)
            tmp[i] = data[(head + i) % capacity];
        delete[] data;
        data = tmp;
        capacity = cap;
        head = 0;
    }

public:
    TDeque() : data(nullptr), n(0), capacity(0), head(0) {}
    TDeque(size_t capacity_) : data(capacity_ == 0 ? nullptr : new T[capacity_]()), n(0), capacity(capacity_), head(0) {}
    TDeque(const TDeque& d) : data(d.capacity != 0 ? new T[d.capacity]() : nullptr), n(d.n), capacity(d.capacity), head(0) {
        for (size_t i = 0; i < n; i++)
            data[i] = d.data[(d.head + i) % d.capacity];
    }
    ~TDeque() { delete[] data; }

    TDeque& operator=(const TDeque& d) {
        if (this == &d) return *this;

        T* tmp = d.capacity == 0 ? nullptr : new T[d.capacity]();
        for (size_t i = 0; i < d.n; i++)
            tmp[i] = d.data[(d.head + i) % d.capacity];

        delete[] data;
        data = tmp;
        n = d.n;
        capacity = d.capacity;
        head = 0;

        return *this;
    }

    void push_back(const T& a) {
        if (n >= capacity) {
            T tmp = a;
            resize(capacity == 0 ? 1 : capacity * FACTOR);
            data[n] = tmp;
        } 
        else data[(head + n) % capacity] = a;
        n += 1;
    }

    void push_front(const T& a) {
        if (n >= capacity) {
            T tmp = a;
            resize(capacity == 0 ? 1 : capacity * FACTOR);
            data[capacity - 1] = tmp;
            head = capacity - 1;
        }
        else {
            size_t newHead = head == 0 ? capacity - 1 : head - 1;
            data[newHead] = a;
            head = newHead;
        }
        n += 1;
    }

    void pop_back() {
        if (n == 0) return;
        n -= 1;
        if (n * FACTOR * FACTOR <= capacity) resize(n * FACTOR); //освобождаем память при сильном уменьшении
    }

    void pop_front() {
        if (n == 0) return;
        head = (head + 1) == capacity ? 0 : head + 1;
        n -= 1;
        if (n * FACTOR * FACTOR <= capacity) resize(n * FACTOR); //освобождаем память при сильном уменьшении
    }

    T& back() {
        if (n == 0) throw std::out_of_range("n = 0");
        return data[(head + n - 1) % capacity];
    }

    const T& back() const {
        if (n == 0) throw std::out_of_range("n = 0");
        return data[(head + n - 1) % capacity];
    }

    T& front() {
        if (n == 0) throw std::out_of_range("n = 0");
        return data[head];
    }

    const T& front() const {
        if (n == 0) throw std::out_of_range("n = 0");
        return data[head];
    }

    bool empty() const { return n == 0; }

    size_t size() const { return n; }

    size_t getCapacity() const { return capacity; }

    void clear() {
        delete[] data;
        data = nullptr;
        n = 0;
        capacity = 0;
        head = 0;
    }
};