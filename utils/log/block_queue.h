#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <deque>
#include <mutex>
#include <condition_variable>
#include <cassert>

template <typename T>
class BlockQueue {
public:
    explicit BlockQueue(int maxSize = 1000);
    ~BlockQueue();
    void push_back(const T&);
    void push_front(const T&);
    bool pop(T&);
    bool pop(T&, int);
    bool empty();
    bool full();
    T front();
    T back();
    void flush();
    void close();
    void clear();
    size_t get_size();
    size_t get_capacity();
    
private:
    std::deque<T> deq;
    size_t capacity;
    std::mutex mtx;
    std::condition_variable p_cond;
    std::condition_variable c_cond;
    bool isClosed;
};

template <typename T>
BlockQueue<T>::BlockQueue(int maxsize) : capacity(maxsize) {
    assert(maxsize > 0);
    isClosed = false;
}
template <typename T>
void BlockQueue<T>::close() {
    {
        // 在此作用域销毁lock_guard
        std::lock_guard<std::mutex> lock(mtx);
        deq.clear();
        isClosed = true;
    }
    p_cond.notify_all();
    c_cond.notify_all();
}
template <typename T>
BlockQueue<T>::~BlockQueue() {
    close();
}
template <typename T>
void BlockQueue<T>::push_back(const T& item) {
    std::unique_lock<std::mutex> lock(mtx);
    while (deq.size() >= capacity) {
        p_cond.wait(lock);
    }
    deq.push_back(item);
    c_cond.notify_one();
}
template <typename T>
void BlockQueue<T>::push_front(const T& item) {
    std::unique_lock<std::mutex> lock(mtx);
    while (deq.size() >= capacity) {
        p_cond.wait(lock);
    }
    deq.push_front(item);
    c_cond.notify_one();
}
template <typename T>
bool BlockQueue<T>::pop(T& item) {
    std::unique_lock<std::mutex> lock(mtx);
    while (deq.empty()) {
        c_cond.wait(lock);
        if (isClosed) {
            return false;
        }
    }
    item = deq.front();
    deq.pop_front();
    p_cond.notify_one();
    return true;
}
template <typename T>
bool BlockQueue<T>::pop(T& item, int timeout) {
    std::unique_lock<std::mutex> lock(mtx);
    while (deq.empty()) {
        if (c_cond.wait_for(lock, std::chrono::seconds(timeout)) 
                == std::cv_status::timeout) {
            return false;
        }
        if (isClosed) {
            return false;
        }
    }
    item = deq.front();
    deq.pop_front();
    p_cond.notify_one();
    return true;
}
template <typename T>
bool BlockQueue<T>::full() {
    std::lock_guard<std::mutex> lock(mtx);
    return deq.size() >= capacity;
}
template <typename T>
bool BlockQueue<T>::empty() {
    std::lock_guard<std::mutex> lock(mtx);
    return deq.empty();
}
template <typename T>
void BlockQueue<T>::clear() {
    std::lock_guard<std::mutex> lock(mtx);
    deq.clear();
}
template <typename T>
size_t BlockQueue<T>::get_size() {
    std::lock_guard<std::mutex> lock(mtx);
    return deq.size();
}
template <typename T>
size_t BlockQueue<T>::get_capacity() {
    std::lock_guard<std::mutex> lock(mtx);
    return capacity;
}
template <typename T>
T BlockQueue<T>::front() {
    std::lock_guard<std::mutex> lock(mtx);
    return deq.front();
}
template <typename T>
T BlockQueue<T>::back() {
    std::lock_guard<std::mutex> lock(mtx);
    return deq.back();
}
template <typename T>
void BlockQueue<T>::flush() {
    c_cond.notify_one();
}

#endif