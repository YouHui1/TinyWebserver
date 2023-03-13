#ifndef SQLPOOL_H
#define SQLPOOL_H

#include <iostream>
#include <mysql/mysql.h>
#include "../locker/locker.h"
#include "../utils/log/log.h"
#include <list>
#include <string>

class SqlConnPool {
private:
    int max_conn; // 最大连接数量
    int free_conn; // 可用连接数
    int cur_conn; // 当前已连接数

    std::string host;
    std::string user;
    std::string password;
    std::string db;

    std::list<MYSQL*> sql_pool;
    Mutex lst_mutex; // 数据库连接池互斥锁
    Semaphore sql_sem; // 数据库连接信号

public:
    static SqlConnPool* Instance();
    void init(std::string host_, std::string db_, std::string user_, std::string password_, 
            int maxnum, int port=3306);
    MYSQL* getConnection();
    bool freeConnection(MYSQL*);
    void destroy();
private:
    SqlConnPool();
    ~SqlConnPool();

};

class ConnectionRAII {
public:
    ConnectionRAII(MYSQL** con, SqlConnPool* connPool);
    ~ConnectionRAII();
private:
    MYSQL* conRAII;
    SqlConnPool* poolRAII;
};

#endif