/* ************************************************************************
> File Name:     http_conn.h
> Author:        youhui
> Created Time:  2023年02月03日 星期五 17时29分38秒
> Description:   
 ************************************************************************/
#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <stdlib.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/epoll.h>

class HttpConn {
private:
    int sockfd;
    sockaddr_in address;
    
public:
    static int epollfd;
    static int user_count;

    HttpConn() {}
    ~HttpConn() {}
    void process();
    void init(int sockfd, const sockaddr_in& addr);
    void close_conn();
    bool read(); // 非阻塞读
    bool write(); // 非阻塞写
};

extern void addsig(int sig, void (handler)(int));
extern void addfd(int epollfd, int fd, bool one_shot);
extern void removefd(int epollfd, int fd);
extern void modfd(int epollfd, int fd, int ev);

#endif
