#include "timer.h"


void cb_func(client_data* user_data) {
    epoll_ctl(HttpConn::epollfd, EPOLL_CTL_DEL, user_data->socketfd, 0);
    assert(user_data);
    close(user_data->socketfd);
    --HttpConn::user_count;
}

Timer::Timer() {
    prev = nullptr;
    next = nullptr;
}
TimerList::TimerList() {
    v_head = new Timer();
    v_head->prev = nullptr;
    v_head->next = nullptr;
    tail = v_head;
}

TimerList::~TimerList() {
    Timer* tmp = tail;
    while (tail != v_head) {
        tail = tail->prev;
        delete tmp;
        tmp = tail;
    }
    delete v_head;
    v_head = nullptr;
    tail = nullptr;
}
void TimerList::add(Timer* timer, Timer* head_) {
    Timer* prev = head_;
    if (prev == NULL) return;
    Timer* cur = prev->next;
    while (cur) {
        if (cur->next == cur) {
            Timer* cur2 = tail;
            while (cur2->prev != cur || cur2 == v_head) {
                cur2 = cur2->prev;
            }
            if (cur2 == v_head) {
                LOG_ERROR("LOOP");
                exit(-1);
            } else {
                cur->next = cur2;
            }
        }
        if (timer->expire < cur->expire) {
            prev->next = timer;
            timer->next = cur;
            cur->prev = timer;
            timer->prev = prev;
            break;
        }
        prev = cur;
        cur = cur->next;
    }
    if (!cur) {
        prev->next = timer;
        timer->prev = prev;
        timer->next = NULL;
        tail = timer;
    }
    return;
}

void TimerList::add_timer(Timer* timer) {
    if (!timer) return;
    if (v_head == tail) {
        v_head->next = timer;
        timer->prev = v_head;
        tail = timer;
        return;
    }
    add(timer, v_head);
}

void TimerList::del_timer(Timer* timer) {
    if (!timer) return;
    if (timer == tail) {
        tail = tail->prev;
        delete timer;
        tail->next = NULL;
        return;
    }
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    delete timer;
    return;
}

void TimerList::adjust_timer(Timer* timer) {
    if (!timer) return;
    Timer* cur = timer->next;
    if (cur == NULL || (timer->expire < cur->expire)) return;
    if (timer == v_head) return;

    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;

    timer->next = NULL;
    timer->prev = NULL;
    add(timer, cur);

    return;
}

void TimerList::tick() {
    if (v_head == tail) return;

    time_t cur_time = time(NULL);
    Timer* cur = v_head->next;
    // 删除所有超时的定时器
    while (cur) {
        if (cur_time < cur->expire) {
            break;
        }

        cur->cb_func(cur->user_data);

        v_head->next = cur->next;
        delete cur;
        cur = v_head->next;
    }
}

void timer_handler(TimerList* timerlist, int timeslot) {
    timerlist->tick();
    alarm(timeslot);
}