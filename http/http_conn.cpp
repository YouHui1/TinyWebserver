/* ************************************************************************
> File Name:     http_conn.cpp
> Author:        youhui
> Created Time:  2023年02月03日 星期五 17时47分39秒
> Description:   
 ************************************************************************/
#include "http_conn.h"

int HttpConn::epollfd = -1;
int HttpConn::user_count = 0;
static int counterr = 0;
static int counter2 = 0;
const char *ok_200_title = "OK";
const char *error_400_title = "Bad Request";
const char *error_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
const char *error_403_title = "Forbidden";
const char *error_403_form = "You do not have permission to get file form this server.\n";
const char *error_404_title = "Not Found";
const char *error_404_form = "The requested file was not found on this server.\n";
const char *error_500_title = "Internal Error";
const char *error_500_form = "There was an unusual problem serving the request file.\n";

const char* doc_root = "/home/youhui/web/TinyWebServer/src/prac";

Mutex lock;

void HttpConn::process() {
    HTTP_CODE read_ret = process_read();

    if (read_ret == NO_REQUEST) {
        utils->modfd(epollfd, sockfd, EPOLLIN, config->connect_mode);
        return;
    }

    bool write_ret = process_write(read_ret);
    if (!write_ret) {
        close_conn();
    }
    utils->modfd(epollfd, sockfd, EPOLLOUT, config->connect_mode);
}

char* HttpConn::get_line() {return read_buf + start_line;}

void HttpConn::init(int sockfd_, const sockaddr_in& addr, Utils* u_, Config* c_) {
    utils = u_;
    config = c_;
    sockfd = sockfd_;
    address = addr;

    
    int reuse = 1;
    setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    utils->addfd(epollfd, sockfd_, true, config->connect_mode);
    user_count++;

    init();
}
void HttpConn::init() {
    check_state = CHECK_STATE_REQUESTLINE;
    check_idx = 0;
    start_line = 0;
    read_idx = 0;
    method = GET;
    url = nullptr;
    version = nullptr;
    host = nullptr;
    linger = false;
    content_length = 0;
    content = NULL;
    file_address = NULL;
    write_idx = 0;
    iv_count = 0;
    byte_have_send = 0;
    byte_to_send = 0;
    read_or_write = -1;
    cgi = 0;
    check = 0;
    timer_flag = 0;

    memset(read_buf, 0, sizeof read_buf);
    memset(write_buf, 0, sizeof write_buf);
    memset(file_path, 0, sizeof file_path);
}

void HttpConn::close_conn() {
    if (sockfd != -1) {
        utils->removefd(epollfd, sockfd);
        sockfd = -1;
        user_count--;
    }
}

bool HttpConn::read() {
    if (read_idx >= READ_BUF_SIZE) {
        return false;
    }
    int byte_read = 0;
    if (config->connect_mode == 0) {
        byte_read = recv(sockfd, read_buf + read_idx, READ_BUF_SIZE - read_idx, 0);
        if (byte_read < 0) {
            return false;
        }
        read_idx += byte_read;
    } else {
        while (true) {
            byte_read = recv(sockfd, read_buf + read_idx, READ_BUF_SIZE - read_idx, 0);
            if (byte_read == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                }
                return false;
            } else if (byte_read == 0) {
                return false;
            }
            read_idx += byte_read;
        }
    }
    return true;
}
void HttpConn::unmap() {
    if (file_address) {
        munmap(file_address, file_state.st_size);
        file_address = NULL;
    }
}
bool HttpConn::write() {
    int tmp = 0;
    // 响应报文为空
    if (byte_to_send == 0) {
        utils->modfd(epollfd, sockfd, EPOLLIN, config->connect_mode);
        init();
        return true;
    }
    do {
        tmp = writev(sockfd, iv, iv_count);
        if (tmp < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                utils->modfd(epollfd, sockfd, EPOLLOUT, config->connect_mode);
                return true;
            }
            unmap();
            return false;
        }

        byte_have_send += tmp;
        byte_to_send -= tmp;
        if (byte_have_send >= iv[0].iov_len) {
            iv[0].iov_len = 0;
            iv[1].iov_base = file_address + (byte_have_send - write_idx);
            iv[1].iov_len = byte_to_send; 
        } else {
            iv[0].iov_base = write_buf + byte_have_send;
            iv[0].iov_len = iv[0].iov_len - byte_have_send;
        }

        if (byte_to_send <= 0) {
            unmap();
            utils->modfd(epollfd, sockfd, EPOLLIN, config->connect_mode);
            if (linger) {
                init();
                return true;
            } else {
                return false;
            }
        }
    
    } while (config->connect_mode == 1);
    // if (config->connect_mode == 0) {
    //     tmp = writev(sockfd, iv, iv_count);
    //     if (tmp < 0) {
    //         unmap();
    //         return false;
    //     }

    //     byte_have_send += tmp;
    //     byte_to_send -= tmp;
    //     if (byte_have_send >= iv[0].iov_len) {
    //         iv[0].iov_len = 0;
    //         iv[1].iov_base = file_address + (byte_have_send - write_idx);
    //         iv[1].iov_len = byte_to_send; 
    //     } else {
    //         iv[0].iov_base = write_buf + byte_have_send;
    //         iv[0].iov_len = iv[0].iov_len - byte_have_send;
    //     }

    //     if (byte_to_send <= 0) {
    //         unmap();
    //         utils->modfd(epollfd, sockfd, EPOLLIN, config->connect_mode);
    //         if (linger) {
    //             init();
    //             return true;
    //         } else {
    //             return false;
    //         }
    //     }

    // } else {
        // while (1)
        // {
        //     tmp = writev(sockfd, iv, iv_count);
        //     if (tmp < 0) {
        //         if (errno == EAGAIN || errno == EWOULDBLOCK) {
        //             utils->modfd(epollfd, sockfd, EPOLLOUT, config->connect_mode);
        //             return true;
        //         }
        //         unmap();
        //         return false;
        //     }

        //     byte_have_send += tmp;
        //     byte_to_send -= tmp;
        //     if (byte_have_send >= iv[0].iov_len) {
        //         iv[0].iov_len = 0;
        //         iv[1].iov_base = file_address + (byte_have_send - write_idx);
        //         iv[1].iov_len = byte_to_send; 
        //     } else {
        //         iv[0].iov_base = write_buf + byte_have_send;
        //         iv[0].iov_len = iv[0].iov_len - byte_have_send;
        //     }

        //     if (byte_to_send <= 0) {
        //         unmap();
        //         utils->modfd(epollfd, sockfd, EPOLLIN);
        //         if (linger) {
        //             init();
        //             return true;
        //         } else {
        //             return false;
        //         }
        //     }
        // }

    // }
    
    return true;
}

HTTP_CODE HttpConn::process_read() {
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    char * text = NULL;
    while ((check_state == CHECK_STATE_CONTENT && line_status == LINE_OK) 
    || ((line_status = parse_line()) == LINE_OK))
    {
        // 解析到请求体或解析到一行完整数据
        text = get_line();
        LOG_INFO("%s", text);

        start_line = check_idx;
        
        switch(check_state) {

        case CHECK_STATE_REQUESTLINE:
        {
            ret = parse_request_line(text);

            if (ret == BAD_REQUEST)
                return BAD_REQUEST;
            
            break;
        }
        case CHECK_STATE_HEADER:
        {
            ret = parse_headers(text);
            if (ret == BAD_REQUEST)
                return BAD_REQUEST;
            else if (ret == GET_REQUEST) {
                return do_request();
            }
            break;
        }
        case CHECK_STATE_CONTENT:
        {
            ret = parse_content(text);
            if (ret == GET_REQUEST)
                return do_request();
            line_status = LINE_OPEN;
            break;
        }
        default: return INTERNAL_ERROR;
        }

    }

    return NO_REQUEST;
}
// 解析请求首行, 获取请求方法，目标URL，HTTP版本
HTTP_CODE HttpConn::parse_request_line(char* text) {
    // GET / HTTP/1.1
    url = strpbrk(text, " \t");
    *url++ = '\0';
    // GET\0/ HTTP/1.1
    
    char* method_ = text;
    if (strcasecmp(method_, "GET") == 0) {
        method = GET;
        cgi = 0;
    } else if (strcasecmp(method_, "POST") == 0) {
        // printf("yes%d\n", ++counterr);
        method = POST;
        cgi = 1;
    } else {
        return BAD_REQUEST;
    }
    url += strspn(url, " \t");
    version = strpbrk(url, " \t");
    if (!version) {
        return BAD_REQUEST;
    }
    *version++ = '\0';
    // GET\0/\0HTTP/1.1
    if (strcasecmp(version, "HTTP/1.1") != 0) {
        return BAD_REQUEST;
    }
    if (strncasecmp(url, "http://", 7) == 0) {
        url += 7;
        url = strchr(url, '/');
    } else if (strncasecmp(url, "https://", 8) == 0) {
        url += 8;
        url = strchr(url, '/');
    }
    if (!url || url[0] != '/') {
        return BAD_REQUEST;
    }
    if (strlen(url) == 1) {
        strcat(url, "login.html");
    }
    check_state = CHECK_STATE_HEADER;

    return NO_REQUEST;
}
 // 解析请求头
HTTP_CODE HttpConn::parse_headers(char* text) {
    if (text[0] == '\0') {
        if (content_length != 0) {
            check_state = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        return GET_REQUEST;
    } else if (strncasecmp(text, "Connection:", 11) == 0) {
        text += 11;
        text += strspn(text, " \t");
        if (strcasecmp(text, "keep-alive") == 0) {
            linger = true;
        }
    } else if (strncasecmp(text, "Content-Length:", 15) == 0) {
        text += 15;
        text += strspn(text, " \t");
        content_length = atol(text);
    } else if (strncasecmp(text, "Host:", 5) == 0) {
        text += 5;
        text += strspn(text, " \t");
        host = text;
    } else {
        // printf("待处理: %s\n", text);
    }
    return NO_REQUEST;
}
// 解析请求体
HTTP_CODE HttpConn::parse_content(char* text) {
    if (read_idx >= (content_length + check_idx)) {
        // LOG_DEBUG("content: %s!!!", text);
        text[content_length] = '\0';
        content = text;
        return GET_REQUEST;
    }
    return NO_REQUEST;
}

// 解析一行，判断依据\r\n
LINE_STATUS HttpConn::parse_line() {
    char tmp;

    for (;check_idx < read_idx; ++check_idx) {
        tmp = read_buf[check_idx];
        if (tmp == '\r') {
            if (check_idx + 1 == read_idx) {
                return LINE_OPEN;
            } else if (read_buf[check_idx + 1] == '\n') {
                read_buf[check_idx++] = '\0';
                read_buf[check_idx++] = '\0';
                return LINE_OK; 
            }
            return LINE_BAD;
        } else if (tmp == '\n') {
            if ((check_idx > 1) && (read_buf[check_idx - 1] == '\r')) {
                read_buf[check_idx - 1] = '\0';
                read_buf[check_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;
}
HTTP_CODE HttpConn::do_request() {
    strcpy(file_path, doc_root);
    int len = strlen(doc_root);

    const char* p = strrchr(url, '/');
    if (cgi == 1) {
        // LOG_DEBUG("content: %s\n", content);
        char account[100];
        char password[100];        
        if (strcmp(p, "/register") == 0) {
            size_t i = 8;
            for (; content[i] != '&'; ++i) {
                account[i-8] = content[i];
            }
            account[i-8] = '\0';
            size_t j = 0;
            for (i += 10, j = i; content[j] != '\0'; ++j) {
                password[j-i] = content[j];
            }
            password[j-i] = '\0';
            if (userVerify(account, password, true)) {
                strcpy(url, "/login.html");
            } else {
                strcpy(url, "/registerError.html");
            }
        } else if (strcmp(p, "/login") == 0) {
            size_t i = 8;
            for (; content[i] != '&'; ++i) {
                account[i-8] = content[i];
            }
            account[i-8] = '\0';
            size_t j = 0;
            for (i += 10, j = i; content[j] != '\0'; ++j) {
                password[j-i] = content[j];
            }
            password[j-i] = '\0';

            if (userVerify(account, password, false)) {
                strcpy(url, "/main.html");
            } else {
                strcpy(url, "/loginError.html");
            }
        } else if (strcmp(p, "/???") == 0) {
            strcpy(url, "/newweb.html");
        } else if (strcmp(p, "/picture") == 0) {
            strcpy(url, "/picture.html");
        } else if (strcmp(p, "/video") == 0) {
            strcpy(url, "/video.html");
        }
        
        strncpy(file_path+len, url, FILENAME_LEN - 1 - len);

        
    }
    else {
        if (strcmp(p, "/register?") == 0 || strcmp(p, "/login?") == 0
            || strcmp(p, "/newweb?") == 0) {
            char* tmp = url;
            while(*(tmp+1)) {
                ++tmp;
            }
            strcpy(tmp, ".html\0");
            strncpy(file_path+len, url, FILENAME_LEN - 1 - len);
        }
        else
            strncpy(file_path+len, url, FILENAME_LEN - 1 - len);
    }


    // 请求资源文件信息
    if (stat(file_path, &file_state) < 0) {
        return NO_RESOURCE;
    }
    // 权限是否可读
    if (!(file_state.st_mode & S_IROTH)) {
        return FORBIDDEN_REQUEST;
    }
    // 文件类型判断
    if (S_ISDIR(file_state.st_mode)) {
        return BAD_REQUEST;
    }

    int fd = open(file_path, O_RDONLY);
    file_address = (char*)mmap(0, file_state.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    return FILE_REQUEST;
}

bool HttpConn::userVerify(const char* account, const char* password, bool lr) {
    if (account == "" || password == "") return false;
    LOG_INFO("Verify user:%s, password:%s", account, password);
    MYSQL* sql;
    ConnectionRAII(&sql, SqlConnPool::Instance());
    assert(sql);
    bool flag = false;
    if (lr) flag = true;

    char order[256] = {0};
    MYSQL_FIELD* fields = nullptr;
    MYSQL_RES* res = nullptr;

    snprintf(order, 256, "SELECT username, passwd FROM user WHERE username='%s' LIMIT 1", account);
    if (mysql_query(sql, order)) {
        LOG_ERROR("SELECT error: %s\n", mysql_error(sql));
        mysql_free_result(res);
        return false;
    }
    res = mysql_store_result(sql);
    int num_fields = mysql_num_fields(res);
    fields = mysql_fetch_field(res);
    while (MYSQL_ROW row = mysql_fetch_row(res)) {
        LOG_DEBUG("MYSQL ROW: %s %s", row[0], row[1]);
        std::string pwd(row[1]);
        if (!lr) {
            if (strcmp(pwd.c_str(), password) == 0) flag = true;
            else {
                flag = false;
                LOG_DEBUG("PWD ERROR!");
            }
        } else {
            flag = false;
            LOG_DEBUG("USER USED!");
        }
    }
    mysql_free_result(res);
    if (lr && flag) {
        LOG_DEBUG("REGISTER");
        bzero(order, 256);
        snprintf(order, 256, "INSERT INTO user(username, passwd) VALUES('%s','%s')", account, password);
        LOG_DEBUG("%s", order);
        lock.lock();
        if(mysql_query(sql, order)) { 
            LOG_DEBUG( "INSERT error!");
            flag = false; 
        }
        lock.unlock();
        flag = true;
    }
    SqlConnPool::Instance()->freeConnection(sql);
    LOG_DEBUG( "UserVerify success!!");
    return flag;
}

bool HttpConn::add_response(const char* format, ...) {
    if (write_idx >= WRITE_BUF_SIZE) {
        LOG_ERROR("Out of WRITE_BUF_SIZE");
        return false;
    }
    va_list arg_lst;
    va_start(arg_lst, format);

    int len = vsnprintf(write_buf + write_idx,  WRITE_BUF_SIZE - 1 - write_idx, format, arg_lst);
    if (len >= (WRITE_BUF_SIZE - 1 - write_idx)) {
        va_end(arg_lst);
        LOG_ERROR("Out of WRITE_BUF_SIZE");
        return false;
    }
    write_idx += len;
    va_end(arg_lst);

    LOG_INFO("%s", write_buf);
    return true;
}
bool HttpConn::add_status_line(int status, const char* title) {
    return add_response("%s %d %s\r\n", "HTTP/1.1", status, title);
}
bool HttpConn::add_content_len(int con_len) {
    return add_response("Content-Length:%d\r\n", con_len);
}
bool HttpConn::add_blank_line() {
    return add_response("%s", "\r\n");
}
bool HttpConn::add_content_type() {
    return add_response("Content-Type:%s\r\n", "text/html");
}
bool HttpConn::add_content(const char* content) {
    return add_response("%s", content);
}
bool HttpConn::add_linger() {
    return add_response("Connection:%s\r\n", (linger == true) ? "keep-alive" : "close");
}
bool HttpConn::add_headers(int con_len) {
    return add_content_len(con_len) &&
            add_content_type() &&
            add_blank_line();
}
bool HttpConn::process_write(HTTP_CODE ret) {

    switch(ret) 
    {
        case INTERNAL_ERROR:
        {
            // 状态行
            add_status_line(500, error_500_title);
            // 消息报头
            add_headers(strlen(error_500_form));
            if (!add_content(error_500_form))
                return false;
            break;
        }
        case BAD_REQUEST:
        {
            // 状态行
            add_status_line(404, error_404_title);
            // 消息报头
            add_headers(strlen(error_404_form));
            if (!add_content(error_404_form))
                return false;
            break;
        }
        case FORBIDDEN_REQUEST:
        {
            // 状态行
            add_status_line(403, error_403_title);
            // 消息报头
            add_headers(strlen(error_403_form));
            if (!add_content(error_403_form))
                return false;
            break;
        }
        case FILE_REQUEST:
        {
            add_status_line(200, ok_200_title);
            if (file_state.st_size != 0) {
                add_headers(file_state.st_size);
                // 指向写缓冲区
                iv[0].iov_base = write_buf;
                iv[0].iov_len = write_idx;
                // 
                iv[1].iov_base = file_address;
                iv[1].iov_len = file_state.st_size;
                iv_count = 2;
                byte_to_send = write_idx + file_state.st_size;
                return true;
            } else {
                const char* ok_string = "<html><body></body></html>";
                add_headers(strlen(ok_string));
                if (!add_content(ok_string)) {
                    return false;
                }
            }
        }
        default: return false;
    }
    iv[0].iov_base = write_buf;
    iv[0].iov_len = write_idx;
    iv_count = 1;
    byte_to_send = write_idx;
    return true;
}