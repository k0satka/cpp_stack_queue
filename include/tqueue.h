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
        std::unique_ptr<T[]> tmp(cap == 0 ? nullptr : new T[cap]()); //избавляемся для утечки памяти странных типов T
        for (size_t i = 0; i < n; i++)
            tmp[i] = data[(head + i) % capacity];
        delete[] data;
        data = tmp.release();
        capacity = cap;
        head = 0;
    }

public:
    TDeque() : data(nullptr), n(0), capacity(0), head(0) {}
    TDeque(size_t capacity_) : data(capacity_ == 0 ? nullptr : new T[capacity_]()), n(0), capacity(capacity_), head(0) {}
    TDeque(const TDeque& d) : data(nullptr), n(d.n), capacity(d.capacity), head(0) {
        std::unique_ptr<T[]> tmp(d.capacity == 0 ? nullptr : new T[d.capacity]());
        for (size_t i = 0; i < n; i++)
            tmp[i] = d.data[(d.head + i) % d.capacity];
        data = tmp.release();
    }
    ~TDeque() { delete[] data; }

    TDeque& operator=(const TDeque& d) {
        if (this == &d) return *this;

        std::unique_ptr<T[]> tmp(d.capacity == 0 ? nullptr : new T[d.capacity]());
        for (size_t i = 0; i < d.n; i++)
            tmp[i] = d.data[(d.head + i) % d.capacity];

        delete[] data;
        data = tmp.release();
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
        if (n * FACTOR * FACTOR <= capacity) resize(n * FACTOR); //освобождаем память при сильном уменьшении
        n -= 1;
    }

    void pop_front() {
        if (n == 0) return;
        if (n * FACTOR * FACTOR <= capacity) resize(n * FACTOR); //освобождаем память при сильном уменьшении
        head = (head + 1) == capacity ? 0 : head + 1;
        n -= 1;
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

//для проверки_______________________________________________________

struct Tracked {
    inline static int alive = 0;

    Tracked() {
        ++alive;
    }

    Tracked(const Tracked&) {
        ++alive;
    }

    ~Tracked() {
        --alive;
    }
};

struct ThrowOnCopy {
    inline static int alive = 0;
    inline static bool throw_on_assign = false;

    int value;

    ThrowOnCopy(int v = 0) : value(v) {
        ++alive;
    }

    ThrowOnCopy(const ThrowOnCopy& other) : value(other.value) {
        ++alive;
    }

    ~ThrowOnCopy() {
        --alive;
    }

    ThrowOnCopy& operator=(const ThrowOnCopy& other) {
        if (throw_on_assign) throw std::runtime_error("error");
        value = other.value;
        return *this;
    }
};