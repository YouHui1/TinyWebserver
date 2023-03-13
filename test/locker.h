/* ************************************************************************
> File Name:     lock.h
> Author:        youhui
> Created Time:  2023年02月02日 星期四 22时11分00秒
> Description:   
 ************************************************************************/
#ifndef LOCKER_H
#define LOCKER_H

#include <pthread.h>
#include <semaphore.h>
#include <exception>

class Semaphore {
private:
    sem_t s;
public:
    Semaphore() {
        if (sem_init(&s, 0, 0) != 0) {
            throw std::exception();
        }
    }
    ~Semaphore() {sem_destroy(&s);}
    bool wait() {return sem_wait(&s) == 0;}
    bool post() {return sem_post(&s) == 0;}
};

class Mutex {
private:
    pthread_mutex_t m;
public:
    Mutex() {
        if (pthread_mutex_init(&m, NULL) != 0) {
            throw std::exception();
        }
    }
    ~Mutex() {pthread_mutex_destroy(&m);}
    bool lock() {return pthread_mutex_lock(&m) == 0;}
    bool unlock() {return pthread_mutex_unlock(&m) == 0;}
    pthread_mutex_t *get() {return &m;}
};

class Cond {
private:
    pthread_cond_t c;
public:
    Cond() {
        if (pthread_cond_init(&c, NULL) != 0) {
            throw std::exception();
        }
    }
    ~Cond() {pthread_cond_destroy(&c);}
    bool wait(pthread_mutex_t *m_) {
        int ret = 0;
        ret = pthread_cond_wait(&c, m_);
        return ret == 0;
    }
    bool timewait(pthread_mutex_t *m_, struct timespec t_) {
        int ret = 0;
        ret = pthread_cond_timedwait(&c, m_, &t_);
        return ret == 0;
    }
    bool signal() {return pthread_cond_signal(&c) == 0;}
    bool broadcast() {return pthread_cond_broadcast(&c) == 0;}
};
#endif
