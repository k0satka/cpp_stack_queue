#pragma once
#include <stdexcept>

const size_t FACTOR = 2;

template<typename T>
class TStack {
private:
	T* data;
	size_t n;
	size_t capacity;

	void resize(size_t cap) {
		assert(cap >= n);
		T* tmp = cap == 0 ? nullptr : new T[cap]();
		if (n != 0) std::copy(data, data + n, tmp);
		capacity = cap;
		delete[] data;
		data = tmp;
	}

public:
	TStack() : data(nullptr), n(0), capacity(0) {}
	TStack(size_t capacity_) : data(capacity_ == 0 ? nullptr : new T[capacity_]()), n(0), capacity(capacity_) {}
	TStack(const TStack& s) : data(s.capacity != 0 ? new T[s.capacity]() : nullptr), n(s.n), capacity(s.capacity) {
		if (s.n != 0) std::copy(s.data, s.data + s.n, data);
	}
	~TStack() { delete[] data; }

	TStack& operator=(const TStack& s) {
		if (this == &s) return *this;

		T* tmp = s.capacity == 0 ? nullptr : new T[s.capacity]();
		if (s.n != 0) std::copy(s.data, s.data + s.n, tmp);

		delete[] data;
		data = tmp;
		n = s.n;
		capacity = s.capacity;
		
		return *this;
	}

	void push(const T& a) {
		if (n >= capacity) {
			T tmp = a;
			resize(capacity == 0 ? 1 : capacity * FACTOR);
			data[n] = tmp;
		}
		else data[n] = a;
		n += 1;
	}

	void pop() {
		if (n == 0) return;
		n -= 1;
		if (n * FACTOR * FACTOR <= capacity) resize(n * FACTOR); //освобождаем память при сильном уменьшении
	}

	T& top() { 
		if (n == 0) throw std::out_of_range("stack empty");
		return data[n - 1];
	}

	const T& top() const {
		if (n == 0) throw std::out_of_range("stack empty");
		return data[n - 1];
	}

	bool empty() const { return n == 0; }

	size_t size() const { return n; }

	size_t getCapacity() const { return capacity; }

	void clear() {
		delete[] data;
		data = nullptr;
		n = 0;
		capacity = 0;
	}
};