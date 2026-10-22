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
        try {
            for (size_t i = 0; i < n; i++)
                tmp[i] = data[(head + i) % capacity];
        }
        catch (...) {
            delete[] tmp; //вдруг у странного типа T что то пойдёт не так при =
            throw;
        }
        delete[] data;
        data = tmp;
        capacity = cap;
        head = 0;
    }

public:
    TDeque() : data(nullptr), n(0), capacity(0), head(0) {}
    TDeque(size_t capacity_) : data(capacity_ == 0 ? nullptr : new T[capacity_]()), n(0), capacity(capacity_), head(0) {}
    TDeque(const TDeque& d) : data(nullptr), n(d.n), capacity(d.capacity), head(0) {
        T* tmp = d.capacity == 0 ? nullptr : new T[d.capacity]();
        try {
            for (size_t i = 0; i < n; i++)
                tmp[i] = d.data[(d.head + i) % d.capacity];
        }
        catch (...) {
            delete[] tmp;
            throw;
        }
        data = tmp;
    }
    TDeque(TDeque&& d) noexcept {
        data = d.data;
        n = d.n;
        capacity = d.capacity;
        head = d.head;

        d.data = nullptr;
        d.n = 0;
        d.capacity = 0;
        d.head = 0;
    }
    ~TDeque() { delete[] data; }

    TDeque& operator=(const TDeque& d) {
        if (this == &d) return *this;

        T* tmp = d.capacity == 0 ? nullptr : new T[d.capacity]();
        try {
            for (size_t i = 0; i < d.n; i++)
                tmp[i] = d.data[(d.head + i) % d.capacity];
        }
        catch (...) {
            delete[] tmp;
            throw;
        }

        delete[] data;
        data = tmp;
        n = d.n;
        capacity = d.capacity;
        head = 0;

        return *this;
    }

    TDeque& operator=(TDeque&& d) noexcept {
        if (this == &d) return *this;

        delete[] data;

        data = d.data;
        n = d.n;
        capacity = d.capacity;
        head = d.head;

        d.data = nullptr;
        d.n = 0;
        d.capacity = 0;
        d.head = 0;

        return *this;
    }

    void push_back(const T& a) {
        if (n >= capacity) {
            size_t new_cap = capacity == 0 ? 1 : capacity * FACTOR;
            T* tmp = new_cap == 0 ? nullptr : new T[new_cap](); //заранее подготавливаем память прямо тут чтобы проверить 2 случая: push_back(q.front()) чтобы не записать ссылку на удалённую память и data[n] = a может бросить throw, тогда мы зря бы использовали resize
            try {
                for (size_t i = 0; i < n; i++)
                    tmp[i] = data[(head + i) % capacity];
                tmp[n] = a;
            }
            catch (...) {
                delete[] tmp;
                throw;
            }
            delete[] data;
            data = tmp;
            capacity = new_cap;
            head = 0;
        } 
        else data[(head + n) % capacity] = a;
        n += 1;
    }

    void push_front(const T& a) {
        if (n >= capacity) {
            size_t new_cap = capacity == 0 ? 1 : capacity * FACTOR;
            T* tmp = new_cap == 0 ? nullptr : new T[new_cap]();
            try {
                tmp[0] = a;
                for (size_t i = 0; i < n; i++)
                    tmp[i + 1] = data[(head + i) % capacity];
            }
            catch (...) {
                delete[] tmp;
                throw;
            }
            delete[] data;
            data = tmp;
            capacity = new_cap;
            head = 0;
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
        if (n == 1) { clear(); return; } //потому что если n==1 будет resize(0) а там assert(0>=n)
        if ((n - 1) <= capacity / (FACTOR * FACTOR)) resize((n - 1) * FACTOR); //освобождаем память при сильном уменьшении. не пишу new_n * FACTOR * FACTOR >= capacity потому что может быть переполнение size_t. и важен порядок чтобы не изменить n если будет икслючение в resize
        n -= 1;
    }

    void pop_front() {
        if (n == 0) return;
        if (n == 1) { clear(); return; }
        if ((n - 1) <= capacity / (FACTOR * FACTOR)) resize((n - 1) * FACTOR); //освобождаем память при сильном уменьшении
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