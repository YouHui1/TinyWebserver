#ifndef TIMER_H
#define TIMER_H

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <queue>
#include <unordered_map>
#include <time.h>
#include <chrono>
#include <memory>
#include "../utils/utils.h"
#include "../http/http_conn.h"



class Timer;

using _time_t = std::chrono::steady_clock::time_point;
using etimer_t = std::shared_ptr<Timer>;
using MS = std::chrono::milliseconds;
using Clock = std::chrono::steady_clock;

struct client_data {
    int socketfd;
    sockaddr_in address;
    etimer_t timer;
};

class Timer {
public:
    _time_t expire;
    client_data* user_data;
public:
    Timer();
    ~Timer() {}
    void (* cb_func)(client_data*);
    bool operator< (const Timer& other) {
        return expire < other.expire;
    }
    bool operator> (const Timer& other) {
        return expire > other.expire;
    }
};

class TimerList
{
private:
    // Timer* v_head;
    // Timer* tail;
    std::vector<etimer_t> timer_list;
    std::unordered_map<etimer_t, int> dict;
public:
    TimerList();
    ~TimerList();

    void add_timer(etimer_t);
    void del_timer(etimer_t);
    void adjust_timer(etimer_t);

    void tick();
private:
    // void add(etimer_t, etimer_t);
    void sift_down(int index);
    void sift_up(int index);
};

void cb_func(client_data*);
void timer_handler(TimerList*, int);

#endif
