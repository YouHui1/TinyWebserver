#ifndef TIMER_H
#define TIMER_H

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <queue>
#include <time.h>
#include "../utils/utils.h"
#include "../http/http_conn.h"

class Timer;

struct client_data {
    int socketfd;
    sockaddr_in address;
    Timer* timer;
};
class Timer {
private:

public:
    time_t expire;
    client_data* user_data;
    Timer* prev;
    Timer* next;
public:
    Timer();
    ~Timer() {}
    void (* cb_func)(client_data*);
};

class TimerList
{
private:
    Timer* v_head;
    Timer* tail;
public:
    TimerList();
    ~TimerList();

    void add_timer(Timer*);
    void del_timer(Timer*);
    void adjust_timer(Timer*);

    void tick();
private:
    void add(Timer*, Timer*);
};

void cb_func(client_data*);
void timer_handler(TimerList*, int);
#endif