#include "timer.h"

using std::make_shared;

void cb_func(client_data* user_data) {
    epoll_ctl(HttpConn::epollfd, EPOLL_CTL_DEL, user_data->socketfd, 0);
    assert(user_data);
    close(user_data->socketfd);
    --HttpConn::user_count;
}

Timer::Timer() {

}
TimerList::TimerList() {
    // v_head = new Timer();
    // v_head->prev = nullptr;
    // v_head->next = nullptr;
    // tail = v_head;
}

TimerList::~TimerList() {
    // Timer* tmp = tail;
    // while (tail != v_head) {
    //     tail = tail->prev;
    //     delete tmp;
    //     tmp = tail;
    // }
    // delete v_head;
    // v_head = nullptr;
    // tail = nullptr;
}
void TimerList::sift_down(int index) {
    if (index < 0) return;
    if (index * 2 + 1 >= timer_list.size() || index * 2 + 2 >= timer_list.size()) return;
    int l = index * 2 + 1;
    int r = index * 2 + 2;
    int i = l;
    if (l >= timer_list.size() || r < timer_list.size() && timer_list[l] > timer_list[r]) i = r;
    if (timer_list[index] > timer_list[i]) {
        std::swap(timer_list[index], timer_list[i]);
        dict[timer_list[index]] = i;
        dict[timer_list[i]] = index;
        sift_down(i);
    }
}
void TimerList::sift_up(int index) {
    if (index <= 0 || index >= timer_list.size()) return;
    int p = (index - 1) / 2;
    if (timer_list[index] < timer_list[p]) {
        std::swap(timer_list[index], timer_list[p]);
        dict[timer_list[index]] = p;
        dict[timer_list[p]] = index;
        sift_up(p);
    }
}
// void TimerList::add(Timer* timer, Timer* head_) {
//     Timer* prev = head_;
//     if (prev == NULL) return;
//     Timer* cur = prev->next;
//     while (cur) {
//         if (cur->next == cur) {
//             Timer* cur2 = tail;
//             while (cur2->prev != cur || cur2 == v_head) {
//                 cur2 = cur2->prev;
//             }
//             if (cur2 == v_head) {
//                 LOG_ERROR("LOOP");
//                 exit(-1);
//             } else {
//                 cur->next = cur2;
//             }
//         }
//         if (timer->expire < cur->expire) {
//             prev->next = timer;
//             timer->next = cur;
//             cur->prev = timer;
//             timer->prev = prev;
//             break;
//         }
//         prev = cur;
//         cur = cur->next;
//     }
//     if (!cur) {
//         prev->next = timer;
//         timer->prev = prev;
//         timer->next = NULL;
//         tail = timer;
//     }
//     return;
// }

void TimerList::add_timer(etimer_t timer) {
    if (timer == nullptr) return;
    if (dict.count(timer) == 0) {
        int i = timer_list.size();
        timer_list.push_back(timer);
        dict[timer] = i;
        sift_up(i);
    } else {
        int i = dict[timer];
        sift_down(i);
        sift_up(i);
    }
}

void TimerList::del_timer(etimer_t timer) {
    if (dict.count(timer) == 1) {
        timer_list.clear();
        return;
    };
    int i = dict[timer];
    int n = timer_list.size() - 1;
    std::swap(timer_list[i], timer_list[n]);
    timer_list.pop_back();
    dict.erase(timer);
    if (!timer_list.empty()) {
        sift_down(i);
        sift_up(i);
    }
}

void TimerList::adjust_timer(etimer_t timer) {
    if (dict.count(timer) == 0) return;
    int i = dict[timer];
    sift_down(i);
    sift_up(i);

    return;
}

void TimerList::tick() {
    if (timer_list.empty()) return;
    auto cur_time = Clock::now();
    // 删除所有超时的定时器
    while (!timer_list.empty()) {
        etimer_t cur = timer_list.front();
        if (std::chrono::duration_cast<MS>(cur->expire - cur_time).count() > 0) {
            break;
        }
        cur->cb_func(cur->user_data);
        del_timer(cur);
        LOG_DEBUG("close: %d \n", cur->user_data->socketfd);
    }
}

void timer_handler(TimerList* timerlist, int timeslot) {
    timerlist->tick();
    alarm(timeslot);
}
