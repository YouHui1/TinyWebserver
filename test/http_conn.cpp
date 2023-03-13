/* ************************************************************************
> File Name:     http_conn.cpp
> Author:        youhui
> Created Time:  2023年02月03日 星期五 17时47分39秒
> Description:   
 ************************************************************************/
#include "http_conn.h"

int HttpConn::epollfd = -1;
int HttpConn::user_count = 0;

void HttpConn::process() {
    printf("test\n");
}

void HttpConn::init(int sockfd_, const sockaddr_in& addr) {
    sockfd = sockfd_;
    address = addr;
    
    int reuse = 1;
    setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    addfd(epollfd, sockfd_, true);
    user_count++;
    
}
void HttpConn::close_conn() {
    if (sockfd != -1) {
        removefd(epollfd, sockfd);
        sockfd = -1;
        user_count--;
    }
}

bool HttpConn::read() {
    printf("read\n");
    return true;
}

bool HttpConn::write() {
    printf("write\n");
    return true;
}

int setnonblocking(int fd) {
    int old_flag = fcntl(fd, F_GETFL);
    int new_flag = old_flag | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_flag);
    return old_flag;
}
void addsig(int sig, void (handler)(int)) {
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = handler;
    sigfillset(&sa.sa_mask);
    sigaction(sig, &sa, NULL);
}

void addfd(int epollfd, int fd, bool one_shot) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLRDHUP;
    if (one_shot) {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void removefd(int epollfd, int fd) {
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

void modfd(int epollfd, int fd, int ev) {
    epoll_event event;
    event.data.fd = fd;
    event.events = ev | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}
