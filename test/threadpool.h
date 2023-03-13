/* ************************************************************************
> File Name:     threadpool.h
> Author:        youhui
> Created Time:  2023年02月03日 星期五 10时26分03秒
> Description:   
 ************************************************************************/
#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <deque>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "locker.h"

template <typename T>
class ThreadPool {
private:
    int thread_number; // 线程池里的线程数
    int max_requests; // 最大请求数
    pthread_t* threads; // 线程池
    std::deque<T*> request_que; // 请求队列
    Mutex que_mutex; // 请求队列互斥锁
    Semaphore task_sem; // 任务响应数
    bool stop; // 线程池结束标志

private:
    void run();
    static void* worker(void*);
public:
    ThreadPool(int num = 8, int max_num = 10000);
    ~ThreadPool();
    bool append(T*);
};

template<typename T>
ThreadPool<T>::ThreadPool(int num, int max_num) {
    if (num <= 0 || max_num <= 0) {
        throw std::exception();
    }
    thread_number = num;
    max_requests = max_num;
    stop = false;
    threads = new pthread_t[thread_number];
    if (!threads)
        throw std::exception();
    for (int i = 0; i < thread_number; ++i) {
       if (pthread_create(&threads[i], NULL, worker, this) != 0) {
           delete[] threads;
           throw std::exception();
       }
       if (pthread_detach(threads[i])) {
           delete[] threads;
           throw std::exception();
       }
       // printf("yes%d\n", i);
    }
}

template<typename T>
ThreadPool<T>::~ThreadPool() {
    delete[] threads;
    stop = true;
}

template<typename T>
bool ThreadPool<T>::append(T* task_) {
    que_mutex.lock();
    if (request_que.size() >= max_requests) {
        que_mutex.unlock();
        return false;
    }
    request_que.push_back(task_);
    que_mutex.unlock();
    task_sem.post();
    return true;
}

template<typename T>
void* ThreadPool<T>::worker(void* args) {
    ThreadPool* threadpool = (ThreadPool*) args;
    threadpool->run();
    return threadpool;
}

template<typename T>
void ThreadPool<T>::run() {
    while (!stop) {
        task_sem.wait();
        que_mutex.lock();
        if (request_que.empty()) {
            que_mutex.unlock();
            continue;
        }
        T* task = request_que.front();
        request_que.pop_front();
        que_mutex.unlock();
        if (task == nullptr)
            continue;
        
        task->process();
        // printf("yes\n");
    }
}
#endif
