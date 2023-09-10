/* ************************************************************************
> File Name:     webserver.h
> Author:        youhui
> Created Time:  2023年02月03日 星期五 15时03分04秒
> Description:
 ************************************************************************/
#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <signal.h>
#include <errno.h>
#include "http/http_conn.h"
#include "threadpool/threadpool.h"
#include "utils/config.h"
#include "timer/timer.h"
#include "utils/log/log.h"
#include "sql/sqlpool.h"

#define MAX_FD 65535 // 最大文件描述符个数
#define MAX_EVENT_NUMBER 10000 // 监听的最大事件个数
#define TIMESLOT 5 // 最小超时单位

using _time_t = std::chrono::steady_clock::time_point;
using Clock = std::chrono::steady_clock;
using etimer_t = std::shared_ptr<Timer>;
using MS = std::chrono::milliseconds;

class WebServer {
private:
    bool socketInit();
    void signalInit();
    bool timerInit();
    void listenProcessing();
    bool signalProcessing();
    void readProcessing(int&);
    void writeProcessing(int&);

    void adjustTimer(etimer_t);
    void setTimer(client_data&, struct sockaddr_in&, int&);
    void closeTimer(client_data&);


public:
    int listenfd;
    struct sockaddr_in saddr;
    int port;

    int epollfd;
    epoll_event events[MAX_EVENT_NUMBER];

    int pipefd[2];

    bool stop_server;
    bool openLinger;
    bool timeout;

    Config* config;
    Utils* utils;
    ThreadPool<HttpConn>* threadpool;
    HttpConn* users;
    client_data* user_timers;
    TimerList* timerlist;
public:
    WebServer();
    WebServer(int port_, int event_mode_, int pattern_, bool linger_,
            bool log_open_, int log_que_size_, int log_level_,
            int thread_num, int sql_num,
            std::string host_, std::string db_, std::string user_, std::string pwd_);
    ~WebServer();

    void start();
};

#endif
