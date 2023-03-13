#include "webserver.h"

WebServer::WebServer(int port_, int event_mode_, int pattern_, bool linger_,
                    bool log_open_, int log_que_size_, int log_level,
                    int thread_num, int conn_num, 
                    std::string host_, std::string db_, std::string user_, std::string pwd_):
    utils(new Utils()), users(new HttpConn[MAX_FD]), user_timers(new client_data[MAX_FD]),
    threadpool(new ThreadPool<HttpConn>(thread_num)), timerlist(new TimerList()), config(new Config()),
    stop_server(false), timeout(false), openLinger(linger_)
{
    utils->addsig(SIGPIPE, SIG_IGN);
    Utils::pipefd = pipefd;
    SqlConnPool::Instance()->init(host_, db_, user_, pwd_, conn_num);
    // 

    port = port_;
    config->listen_mode = event_mode_ % 2;
    config->connect_mode = event_mode_ / 2;
    config->pattern = pattern_;
    //
    if (log_open_) {
        Log::Instance()->init(log_level, "./log", 512, ".log");
        if (stop_server) {LOG_ERROR("========== Server Init Error! ==========");}
        else {
            LOG_INFO("========== Server Log Init ==========");
            LOG_INFO("Port: %d", port);
            LOG_INFO("Listen mode: %s, Connect mode: %s", 
                    (config->listen_mode == 0 ? "LT" : "ET"),
                    (config->connect_mode == 0 ? "LT" : "ET"));
            LOG_INFO("Pattern: %s", (config->pattern == 0 ? "Proactor" : "Reactor"));
            LOG_INFO("Logsys level: %d", log_level);
            LOG_INFO("Threadpool num: %d", thread_num);
        }
    }

    if (!socketInit()) stop_server = true;
    if (!timerInit()) stop_server = true;
    signalInit();
}

WebServer::~WebServer() {
    close(epollfd);
    close(listenfd);
    close(pipefd[0]);
    close(pipefd[1]);
    delete[] users;
    delete[] user_timers;
    delete timerlist;
    delete threadpool;
    delete utils;
    delete config;
    SqlConnPool::Instance()->destroy();
    LOG_INFO("========== Server Closed ==========");
}

bool WebServer::socketInit() {
    int ret;
    if(port > 65535 || port < 1024) {
        LOG_ERROR("Port: %d error!",  port);
        return false;
    }
    struct linger optLinger = {0}; 
    if (openLinger) { // 优雅关闭
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }

    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(port);

    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        LOG_ERROR("Create socket error!")
        return false;
    }

    ret = setsockopt(listenfd, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if (ret < 0) {
        close(listenfd);
        LOG_ERROR("Set linger error!");
        return false;
    }

    int reuse = 1;
    ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (ret < 0) {
        close(listenfd);
        LOG_ERROR("Set reuse error!");
        return false;
    }

    ret = bind(listenfd, (struct sockaddr*) &saddr, sizeof(saddr));
    if (ret < 0) {
        close(listenfd);
        LOG_ERROR("Bind port: %d error!", port);
        return false;
    }
    ret = listen(listenfd, 5);
    if (ret == -1) {
        close(listenfd);
        LOG_ERROR("Listen port: %d error!", port);
        return false;
    }

    epollfd = epoll_create(50);
    if (epollfd == -1) {
        close(listenfd);
        LOG_ERROR("Create epoll error!");
        return false;
    }
    utils->addfd(epollfd, listenfd, false, config->listen_mode);
    HttpConn::epollfd = epollfd;
    return true;
}
void WebServer::signalInit() {
    utils->addsig(SIGPIPE, SIG_IGN);
    utils->addsig(SIGALRM, utils->sig_handler, false);
    utils->addsig(SIGTERM, utils->sig_handler, false);
}
bool WebServer::timerInit() {
    utils->init(TIMESLOT);
    int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
    if (ret == -1) {
        LOG_ERROR("Set pipe error!");
        return false;
    }
    utils->setnonblocking(pipefd[1]);
    utils->addfd(epollfd, pipefd[0], false, 0);
    Utils::pipefd = pipefd;
    alarm(TIMESLOT);
    return true;
}


void WebServer::start() {
    if (!stop_server) LOG_INFO("========== Server Start ==========");
    while (!stop_server) {
        int num = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if ((num < 0) && (errno != EINTR)) {
            LOG_ERROR("Epoll failure");
            break;
        }
        for (int i = 0; i < num; ++i) {
            int sockfd = events[i].data.fd;
            if (sockfd == listenfd) {
                LOG_DEBUG("LISTEN");
                listenProcessing();
            } else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                LOG_DEBUG("CLOSE");
                closeTimer(user_timers[sockfd]);
            } else if ((events[i].events & EPOLLIN) && (sockfd == pipefd[0])) {
                if (!signalProcessing()) {
                    LOG_ERROR("Signal");
                }
            } else if (events[i].events & EPOLLIN) {
                LOG_DEBUG("READ");
                // printf("yes1 %d\n", ++count1);
                readProcessing(sockfd);                
                // printf("yes2 %d\n", ++count2);
        } else if (events[i].events & EPOLLOUT) {
                LOG_DEBUG("WRITE");
                writeProcessing(sockfd);
            } else {
                LOG_ERROR("Unexpected event");
            }
        }
        if (timeout) {
            // timer_handler_(timerlist, TIMESLOT);
            timer_handler(timerlist, TIMESLOT);
            LOG_INFO("Timer tick");
            timeout = false;
        }
    }
}

void WebServer::closeTimer(client_data& c_) {
    Timer* timer = c_.timer;
    timer->cb_func(&c_);
    if (timer) {
        timerlist->del_timer(timer);
    }
    LOG_INFO("Close fd: %d", c_.socketfd);
    return;
}
void WebServer::adjustTimer(Timer* timer) {
    if (timer) {
        time_t cur = time(NULL);
        timer->expire = cur + 3 * TIMESLOT;
        timerlist->adjust_timer(timer);
        LOG_INFO("Adjust timer once");
    }
    return;
}

void WebServer::setTimer(client_data& c_, struct sockaddr_in& c_addr, int& cfd) {
    Timer* timer = new Timer();
    // printf("setTimer%d\n", count1);
    user_timers[cfd].address = c_addr;
    user_timers[cfd].socketfd = cfd;
    timer->user_data = &user_timers[cfd];
    timer->cb_func = cb_func;
    time_t cur = time(NULL);
    user_timers[cfd].timer = timer;
    timerlist->add_timer(timer);
    LOG_INFO("Set timer, fd: %d", c_.socketfd);
    return;
}

void WebServer::listenProcessing() {
    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);
    do {
        int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_len);
        if (connfd <= 0) {
            // if (errno != EAGAIN && errno != EWOULDBLOCK)
            //     LOG_ERROR("Accept error");
            return;
        }
        if (HttpConn::user_count >= MAX_FD) {
            utils->showError(connfd, "Server busy");
            LOG_WARN("Clients is full!");
            return;
        }
        users[connfd].init(connfd, client_address, utils, config);
        setTimer(user_timers[connfd], client_address, connfd);
    } while(config->listen_mode == 1);
}

bool WebServer::signalProcessing() {
    int ret = 0;
    int sig;
    char signals[1024];
    ret = recv(pipefd[0], signals, sizeof(signals), 0);
    if (ret == -1) return false;
    else if (ret == 0) return false;
    else {
        for (int j = 0; j < ret; ++j) {
            switch (signals[j])
            {
            case SIGALRM:
                timeout = true;
                break;
            case SIGTERM:
                stop_server = true;
                break;
            default:
                break;
            }
        }
    }
    return true;
}

void WebServer::readProcessing(int& sockfd) {
    if (config->pattern == 0) {
        if (users[sockfd].read()) {
            threadpool->append(users + sockfd);
            adjustTimer(user_timers[sockfd].timer);
            // printf("yes3 %d\n", ++count3);
        } else {
            closeTimer(user_timers[sockfd]);
        }

    } else {
        adjustTimer(user_timers[sockfd].timer); 
        users[sockfd].read_or_write = 0;
        threadpool->append(users + sockfd);
        while (true) {
            if (users[sockfd].check == 1) {
                if (users[sockfd].timer_flag == 1) {
                    closeTimer(user_timers[sockfd]);
                    users[sockfd].timer_flag = 0;
                }
                users[sockfd].check = 0;
                break;
            }
        }
    }

}

void WebServer::writeProcessing(int& sockfd) {
    if (config->pattern == 0) {
        if (users[sockfd].write()) {
            adjustTimer(user_timers[sockfd].timer);
        } else {
            closeTimer(user_timers[sockfd]);
        }
    } else {
        adjustTimer(user_timers[sockfd].timer);
        users[sockfd].read_or_write = 1;
        threadpool->append(users + sockfd);
        while (true) {
            if (users[sockfd].check == 1) {
                if (users[sockfd].timer_flag == 1) {
                    closeTimer(user_timers[sockfd]);
                    users[sockfd].timer_flag = 0;
                }
                users[sockfd].check = 0;
                break;
            }
        }
        
    }
}

