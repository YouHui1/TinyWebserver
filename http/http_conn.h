/* ************************************************************************
> File Name:     http_conn.h
> Author:        youhui
> Created Time:  2023年02月03日 星期五 17时29分38秒
> Description:   
 ************************************************************************/
#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <cstring>
#include <stdlib.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <signal.h>
#include <stdarg.h>

#include "../utils/utils.h"
#include "../utils/config.h"
#include "../utils/log/log.h"
#include "../sql/sqlpool.h"
#include "../locker/locker.h"

#define READ_BUF_SIZE 2048
#define WRITE_BUF_SIZE 1024
#define FILENAME_LEN 200


enum METHOD {
    GET = 0,
    POST,
    HEAD,
    PUT,
    DELETE,
    TRACE,
    OPTIONS,
    CONNECT,
    PATH
};
enum CHECK_STATE {
    CHECK_STATE_REQUESTLINE = 0,
    CHECK_STATE_HEADER,
    CHECK_STATE_CONTENT
};
enum HTTP_CODE {
    NO_REQUEST = 0,
    GET_REQUEST,
    BAD_REQUEST,
    NO_RESOURCE,
    FORBIDDEN_REQUEST,
    FILE_REQUEST,
    INTERNAL_ERROR,
    CLOSED_CONNECTION
};
enum LINE_STATUS {
    LINE_OK = 0,
    LINE_BAD,
    LINE_OPEN
};


class HttpConn {
private:
    int sockfd;
    sockaddr_in address;
    
    int read_idx;  // 指向read_buf数据末尾的下一个字节
    char read_buf[READ_BUF_SIZE]; // 读缓冲区
    int check_idx; // 当前正在分析的字符在读缓冲区的位置
    int start_line; // 当前正在解析行的起始位置
    int content_length; // 请求体长度
    CHECK_STATE check_state; // 状态机当前所处状态
    char* url; // 请求目标文件文件名，资源地址
    char* version; // 协议版本
    METHOD method; // 请求方法
    char* host; // 主机名
    bool linger; // HTTP请求是否要保持连接
    char* content; // 请求体内容

    char write_buf[WRITE_BUF_SIZE]; // 写缓冲区
    struct stat file_state;
    char* file_address; // 文件地址
    struct iovec iv[2];
    int iv_count;
    int write_idx; // 写缓冲区的长度
    char file_path[FILENAME_LEN];

    int byte_to_send; // 需要发送的字节数
    int byte_have_send; // 已经发送的字节数
    unsigned cgi;


private:
    char* get_line();
    HTTP_CODE do_request();

    bool add_response(const char*, ...);
    bool add_status_line(int, const char*);
    bool add_headers(int);
    bool add_content_len(int);
    bool add_content_type();
    bool add_content(const char*);
    bool add_linger();
    bool add_blank_line();
    
    bool userVerify(const char* user, const char* password, bool lr);

public:
    Config* config;
    Utils* utils;
    static int epollfd;
    static int user_count;
    int read_or_write; // 响应读写检测
    int timer_flag;
    int check;
    Mutex lock;

public:
    HttpConn() {}
    ~HttpConn() {}
    void process();
    void init(int, const sockaddr_in&, Utils*, Config*);
    void init();
    void close_conn();
    void unmap();
    bool read(); // 非阻塞读
    bool write(); // 非阻塞写

    HTTP_CODE process_read();
    bool process_write(HTTP_CODE);
    HTTP_CODE parse_request_line(char*); // 解析请求首行
    HTTP_CODE parse_headers(char*); // 解析请求头
    HTTP_CODE parse_content(char*); // 解析请求体

    LINE_STATUS parse_line();
};


#endif
