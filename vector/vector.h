#pragma once
#include <cstddef>
#include <iostream>
#include <cassert>

template <typename T>
struct vector {
    typedef T* iterator;
    typedef T const* const_iterator;

    vector();                               // O(1) nothrow
    vector(vector const&);                  // O(N) strong
    vector& operator=(vector const& other); // O(N) strong

    ~vector();                              // O(N) nothrow

    T& operator[](size_t i);                // O(1) nothrow
    T const& operator[](size_t i) const;    // O(1) nothrow

    T* data();                              // O(1) nothrow
    T const* data() const;                  // O(1) nothrow
    size_t size() const;                    // O(1) nothrow

    T& front();                             // O(1) nothrow
    T const& front() const;                 // O(1) nothrow

    T& back();                              // O(1) nothrow
    T const& back() const;                  // O(1) nothrow
    void push_back(T const&);               // O(1)* strong
    void pop_back();                        // O(1) nothrow

    bool empty() const;                     // O(1) nothrow

    size_t capacity() const;                // O(1) nothrow
    void reserve(size_t);                   // O(N) strong
    void shrink_to_fit();                   // O(N) strong

    void clear();                           // O(N) nothrow

    void swap(vector&);                     // O(1) nothrow

    iterator begin();                       // O(1) nothrow
    iterator end();                         // O(1) nothrow

    const_iterator begin() const;           // O(1) nothrow
    const_iterator end() const;             // O(1) nothrow

    iterator insert(iterator pos, T const&); // O(N) weak
    iterator insert(const_iterator pos, T const&); // O(N) weak

    iterator erase(iterator pos);           // O(N) weak
    iterator erase(const_iterator pos);     // O(N) weak

    iterator erase(iterator first, iterator last); // O(N) weak
    iterator erase(const_iterator first, const_iterator last); // O(N) weak

private:
    vector& newCapacityCopy(size_t, vector const&);
    T* allocate_data(size_t) const;

private:
    size_t size_ = 0;
    size_t capacity_ = 0;
    T* data_ = nullptr;
};

template<typename T>
vector<T>& vector<T>::newCapacityCopy(size_t new_capacity, vector const &copy) {
    vector a;
    if (new_capacity != 0) {
        a.data_ = allocate_data(new_capacity);
        a.capacity_ = new_capacity;
        for (a.size_ = 0; a.size_ < copy.size_; ++a.size_) {
            new(a.data_ + a.size_) T(copy.data_[a.size_]);
        }
    }
    swap(a);
    return *this;
}

template<typename T>
T *vector<T>::allocate_data(size_t size) const {
    return static_cast<T*>(operator new(size * sizeof(T)));
}

template <typename T>
vector<T>::vector() = default;


template<typename T>
vector<T>::~vector() {
    clear();
    operator delete (data_);
}

template<typename T>
vector<T>::vector(vector const &other) {
    newCapacityCopy(std::min(other.capacity_, 2 * other.size_), other);
}

template<typename T>
vector<T> &vector<T>::operator=(vector const &other) {
    if (this != &other) {
        vector(other).swap(*this);
    }
    return *this;
}

template<typename T>
T &vector<T>::operator[](size_t i) {
    assert(i < size_);
    return data_[i];
}

template<typename T>
T const &vector<T>::operator[](size_t i) const {
    assert(i < size_);
    return data_[i];
}

template<typename T>
T *vector<T>::data() {
    return data_;
}

template<typename T>
T const *vector<T>::data() const {
    return data_;
}

template<typename T>
size_t vector<T>::size() const {
    return size_;
}

template<typename T>
T &vector<T>::front() {
    assert(size_ > 0);
    return data_[0];
}

template<typename T>
T const &vector<T>::front() const {
    assert(size_ > 0);
    return data_[0];
}

template<typename T>
T &vector<T>::back() {
    assert(size_ > 0);
    return data_[size_ - 1];
}

template<typename T>
T const &vector<T>::back() const {
    assert(size_ > 0);
    return data_[size_ - 1];
}

template<typename T>
void vector<T>::push_back(const T &elem) {
    if (size_ < capacity_) {
        new(data_ + size_++) T(elem);
    } else {
        vector<T> copy;
        copy.newCapacityCopy(capacity_ * 2 + 1, *this);
        new(copy.data_ + copy.size_++) T(elem);
        swap(copy);
    }
}

template<typename T>
void vector<T>::pop_back() {
    assert(size_ > 0);
    data_[--size_].~T();
}

template<typename T>
bool vector<T>::empty() const {
    return size_ == 0;
}

template<typename T>
size_t vector<T>::capacity() const {
    return capacity_;
}

template<typename T>
void vector<T>::clear() {
    while (size_ > 0)
        pop_back();
}

template<typename T>
void vector<T>::swap(vector &other) {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
}

template<typename T>
void vector<T>::reserve(size_t newCapacity) {
    if (capacity_ >= newCapacity) {
        return;
    }
    newCapacityCopy(newCapacity, *this);
}

template<typename T>
void vector<T>::shrink_to_fit() {
    if (size_ < capacity_) {
        newCapacityCopy(size_, *this);
    }
}

template<typename T>
typename vector<T>::iterator vector<T>::begin() {
    return data_;
}

template<typename T>
typename vector<T>::iterator vector<T>::end() {
    return data_ + size_;
}

template<typename T>
typename vector<T>::const_iterator vector<T>::begin() const {
    return data_;
}

template<typename T>
typename vector<T>::const_iterator vector<T>::end() const {
    return data_ + size_;
}

template<typename T>
typename vector<T>::iterator vector<T>::insert(vector::iterator pos, T const& elem) {
    assert(size_ >= pos - begin() && pos >= begin());
    return insert(static_cast<const_iterator>(pos), elem);
}

template<typename T>
typename vector<T>::iterator vector<T>::insert(vector::const_iterator pos, T const& elem) {
    assert(size_ >= pos - begin() && pos >= begin());
    size_t position = pos - begin();
    push_back(elem);
    for (size_t i = size_ - 1; i > position; i--) {
        std::swap(data_[i], data_[i - 1]);
    }
    return data_ + position;
}

template<typename T>
typename vector<T>::iterator vector<T>::erase(vector::iterator pos) {
    return erase(static_cast<const_iterator>(pos));
}

template<typename T>
typename vector<T>::iterator vector<T>::erase(vector::const_iterator pos) {
    return erase(pos, pos + 1);
}

template<typename T>
typename vector<T>::iterator vector<T>::erase(vector::iterator first, vector::iterator last) {
    return erase(static_cast<const_iterator>(first), static_cast<const_iterator>(last));
}

template<typename T>
typename vector<T>::iterator vector<T>::erase(vector::const_iterator first, vector::const_iterator last) {
    assert(size_ >= last - begin() && first >= begin() && last > first);
    size_t position = first - begin(), diff = last - first;
    for (size_t i = position ; i < size_ - diff; i++) {
        data_[i] = data_[i + diff];
    }
    for (int i = 0; i < diff; i++) {
        pop_back();
    }
    return data_ + position;
}

