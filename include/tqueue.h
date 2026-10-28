#pragma once
#include <stdexcept>

//TDeque на кольцевом буффере:

const size_t FACTOR = 2;

template<typename T>
class TDeque {
private:
    T* data;
    size_t n;
    size_t capacity;
    size_t head;

    //выделяем память без создания объектов
    static T* allocate_storage(size_t cap) { //можно без static
        if (cap == 0) return nullptr;
        if (cap > size_t(-1) / sizeof(T)) throw std::length_error("bol'shoy capacity"); //проверка на переполнение

        void* raw = ::operator new(sizeof(T) * cap, std::align_val_t{ alignof(T) });
        T* tmp = static_cast<T*>(raw);
        return tmp;
    }

    //очищаем
    static void deallocate_storage(T* ptr) noexcept {
        ::operator delete(ptr, std::align_val_t{ alignof(T) });
    }

    //удаляем объекты
    void destroy_elements() noexcept {
        for (size_t i = 0; i < n; i++)
            data[(i + head) % capacity].~T();
        n = 0;
        head = 0;
    }

public:
    TDeque() : data(nullptr), n(0), capacity(0), head(0) {}
    TDeque(size_t capacity_) : data(allocate_storage(capacity_)), n(0), capacity(capacity_), head(0) {}
    TDeque(const TDeque& d) : data(nullptr), n(0), capacity(0), head(0) {
        T* tmp = allocate_storage(d.capacity);
        size_t constructed = 0;
        try {
            for (size_t i = 0; i < d.n; i++) {
                //tmp[i] = d.data[(d.head + i) % d.capacity]; тут вызывался оператор = для tmp[i] но теперь там пусто, поэтому используем конструктор копирования:
                ::new (static_cast<void*>(tmp + i)) T(d.data[(d.head + i) % d.capacity]);
                constructed += 1;
            }
        }
        catch (...) {
            for (size_t i = 0; i < constructed; i++) //уничтожаем constructed элементов, а потом очищаем память. раньше эти 2 шага делал delete[]
                tmp[i].~T();
            deallocate_storage(tmp);
            throw;
        }
        data = tmp;
        n = d.n;
        capacity = d.capacity;
    }
    TDeque(TDeque&& d) noexcept : data(d.data), n(d.n), capacity(d.capacity), head(d.head) {
        d.data = nullptr;
        d.n = 0;
        d.capacity = 0;
        d.head = 0;
    }
    ~TDeque() { 
        //delete[] data; теперь вручную:
        destroy_elements();
        deallocate_storage(data);
    }

    TDeque& operator=(const TDeque& d) {
        if (this == &d) return *this;

        T* tmp = allocate_storage(d.capacity);
        size_t constructed = 0;
        try {
            for (size_t i = 0; i < d.n; i++) {
                ::new (static_cast<void*>(tmp + i)) T(d.data[(d.head + i) % d.capacity]);
                constructed += 1;
            }
        }
        catch (...) {
            for (size_t i = 0; i < constructed; i++)
                tmp[i].~T();
            deallocate_storage(tmp);
            throw;
        }
        destroy_elements();
        deallocate_storage(data);

        data = tmp;
        n = d.n;
        capacity = d.capacity;
        head = 0;

        return *this;
    }

    TDeque& operator=(TDeque&& d) noexcept {
        if (this == &d) return *this;

        destroy_elements();
        deallocate_storage(data);

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
            if (capacity > size_t(-1) / FACTOR) throw std::length_error("capacity perepolnilsya");
            size_t new_cap = capacity == 0 ? 1 : capacity * FACTOR;
            T* tmp = allocate_storage(new_cap); //заранее подготавливаем память прямо тут чтобы проверить 2 случая: push_back(q.front()) чтобы не записать ссылку на удалённую память и data[n] = a может бросить throw, тогда мы зря бы использовали resize
            size_t constructed = 0;
            try {
                for (size_t i = 0; i < n; i++) {
                    ::new (static_cast<void*>(tmp + i)) T(data[(head + i) % capacity]); //tmp[i] = data[(head + i) % capacity];
                    constructed += 1;
                }
                ::new (static_cast<void*>(tmp + n)) T(a); //tmp[n] = a;
            }
            catch (...) {
                for (size_t i = 0; i < constructed; i++)
                    tmp[i].~T();
                deallocate_storage(tmp);
                throw;
            }
            size_t n_ = n;
            destroy_elements();
            n = n_;
            deallocate_storage(data);

            data = tmp;
            capacity = new_cap;
            head = 0;
        }
        else ::new (static_cast<void*>(data + (head + n) % capacity)) T(a); //data[(head + n) % capacity] = a;
        n += 1;
    }

    void push_front(const T& a) {
        if (n >= capacity) {
            if (capacity > size_t(-1) / FACTOR) throw std::length_error("capacity perepolnilsya");
            size_t new_cap = capacity == 0 ? 1 : capacity * FACTOR;
            T* tmp = allocate_storage(new_cap);
            size_t constructed = 0;
            try {
                ::new (static_cast<void*>(tmp)) T(a);
                constructed += 1;
                for (size_t i = 0; i < n; i++) {
                    ::new (static_cast<void*>(tmp + i + 1)) T(data[(head + i) % capacity]);
                    constructed += 1;
                }
            }
            catch (...) {
                for (size_t i = 0; i < constructed; i++)
                    tmp[i].~T();
                deallocate_storage(tmp);
                throw;
            }
            size_t n_ = n;
            destroy_elements();
            n = n_;
            deallocate_storage(data);

            data = tmp;
            capacity = new_cap;
            head = 0;
        }
        else {
            size_t newHead = head == 0 ? capacity - 1 : head - 1;
            ::new (static_cast<void*>(data + newHead)) T(a);
            head = newHead;
        }
        n += 1;
    }

    void pop_back() {
        if (n == 0) return;
        data[(head + n - 1) % capacity].~T();
        n -= 1;
    }

    void pop_front() {
        if (n == 0) return;
        data[head].~T();
        head = (n - 1) == 0 || (head + 1) == capacity ? 0 : head + 1;
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
        destroy_elements();
        deallocate_storage(data);
        data = nullptr;
        capacity = 0;
    }
};

//Deque на блоках с push O(1) и без ограничений на T:

const size_t BLOCK_SIZE = 32;

template<typename T>
class Deque {
private:
    struct Block {
        T* data;
        Block* prev;
        Block* next;

        Block() : data(allocate_storage(BLOCK_SIZE)), prev(nullptr), next(nullptr) { }
        ~Block() { deallocate_storage(data); } //только очистка, а объекты T удаляет deque

        Block(const Block&) = delete;
        Block& operator=(const Block&) = delete;
    };

    Block* first;
    Block* last;

    size_t first_elem;
    size_t last_elem;
    size_t n;

    //выделяем память без создания объектов
    static T* allocate_storage(size_t cap) { //можно без static
        if (cap == 0) return nullptr;
        if (cap > size_t(-1) / sizeof(T)) throw std::length_error("bol'shoy capacity"); //проверка на переполнение

        void* raw = ::operator new(sizeof(T) * cap, std::align_val_t{ alignof(T) });
        T* tmp = static_cast<T*>(raw);
        return tmp;
    }

    //очищаем
    static void deallocate_storage(T* ptr) noexcept {
        ::operator delete(ptr, std::align_val_t{ alignof(T) });
    }

public:
    Deque() : first(nullptr), last(nullptr), first_elem(0), last_elem(0), n(0) {}
};