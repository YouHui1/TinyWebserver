#ifndef UTILS_H
#define UTILS_H

#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <cstring>
#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>

class Utils
{
private:
    int timeslot;
public:
    static int* pipefd;

public:
    Utils();
    ~Utils();
    void init(int);
    void setnonblocking(int fd);

    void addfd(int epollfd, int fd, bool one_shot, int e_mode=0);
    void addsig(int sig, void (handler)(int), bool restart=true); 
    void removefd(int epollfd, int fd);
    void modfd(int epollfd, int fd, int ev, int e_mode=0);
    void showError(int connfd, const char* info);
    static void sig_handler(int);

};

#endif