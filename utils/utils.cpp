#include "utils.h"

Utils::Utils() {}
Utils::~Utils() {}

int* Utils::pipefd = 0;

void Utils::init(int t_) {
    timeslot = t_;
}

void Utils::sig_handler(int sig) {
    int save_errno = errno;
    int msg = sig;
    send(pipefd[1], (char*)&msg, 1, 0);
    errno = save_errno;
}

void Utils::setnonblocking(int fd) {
    int old_flag = fcntl(fd, F_GETFL);
    int new_flag = old_flag | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_flag);
}
void Utils::addsig(int sig, void (handler)(int), bool restart) {
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = handler;
    if (restart)
        sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    sigaction(sig, &sa, NULL);  
}

void Utils::addfd(int epollfd, int fd, bool one_shot, int e_mode) {
    epoll_event event;
    event.data.fd = fd;
    if (e_mode == 1)
        event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
    else
        event.events = EPOLLIN | EPOLLRDHUP;
    if (one_shot) {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void Utils::removefd(int epollfd, int fd) {
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

void Utils::modfd(int epollfd, int fd, int ev, int e_mode) {
    epoll_event event;
    event.data.fd = fd;
    if (e_mode == 1)
        event.events = ev | EPOLLONESHOT | EPOLLRDHUP | EPOLLET;
    else
        event.events = ev | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}
void Utils::showError(int connfd, const char *info)
{
    send(connfd, info, strlen(info), 0);
    close(connfd);
}
