/* ************************************************************************
> File Name:     main.cpp
> Author:        youhui
> Created Time:  2023年02月03日 星期五 15时21分08秒
> Description:   
 ************************************************************************/
#include <cstdio>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <signal.h>
#include <errno.h>
#include "http_conn.h"
#include "threadpool.h"

#define MAX_FD 65535 // 最大文件描述符个数
#define MAX_EVENT_NUMBER 10000 // 监听的最大事件个数



int main() {
    // SIGPIPE
    addsig(SIGPIPE, SIG_IGN);

    // 创建线程池，初始化线程池
    ThreadPool<HttpConn>* pool = NULL;
    try {
        pool = new ThreadPool<HttpConn>; 
    } catch(...) {
        exit(-1);
    }
    
    // 创建一个数组用于·保存所有的客户端信息
    HttpConn* users = new HttpConn[MAX_FD];

    int lfd = socket(PF_INET, SOCK_STREAM, 0);
    if (lfd == -1) {
        perror("socket");
        exit(-1);
    }
    int reuse = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(9000);
    
    int ret = bind(lfd, (struct sockaddr*)&saddr, sizeof(saddr));
    if (ret == -1) {
        perror("bind");
        exit(-1);
    }
    ret = listen(lfd, 5);
    if (ret == -1) {
        perror("listen");
        exit(-1);
    }
    
    epoll_event events[MAX_EVENT_NUMBER];
    
    int epfd = epoll_create(50);
    if (epfd == -1) {
        perror("epoll_create");
        exit(-1);
    }
    
    addfd(epfd, lfd, false);
    HttpConn::epollfd = epfd;
    
    while(true) {
        int num = epoll_wait(epfd, events, MAX_EVENT_NUMBER, -1);
        if ((num < 0) && (errno != EINTR)) {
            printf("epoll failure\n");
            break;
        }
        // 循环遍历事件数组
        for (int i = 0; i < num; ++i) {
            int sockfd = events[i].data.fd;
            if (sockfd == lfd) {
                struct sockaddr_in client_address;
                socklen_t client_len = sizeof(client_address);
                int connfd = accept(lfd, (struct sockaddr*)&client_address, &client_len);
                if(HttpConn::user_count >= MAX_FD) {
                    close(connfd);
                    continue;
                }
                users[connfd].init(connfd, client_address);

            } else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                users[sockfd].close_conn();
            } else if(events[i].events & EPOLLIN ){
                if (users[sockfd].read()) {
                    //一次性把所有数据读完
                    pool->append(users + sockfd);
                } else {
                    users[sockfd].close_conn();
                }
            } else if(events[i].events & EPOLLOUT) {
                if (!users[sockfd].write()) {
                    // 一次性写完所有数据
                    users[sockfd].close_conn();
                }
            }
        }
    }
    close(epfd);
    close(lfd);
    delete[] users;
    delete pool;
    return 0;
}
