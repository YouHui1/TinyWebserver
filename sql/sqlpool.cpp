#include "sqlpool.h"

void SqlConnPool::init(std::string host_, std::string db_, std::string user_, std::string password_, int maxnum, int port) {
    host = host_;
    db = db_;
    user = user_;
    password = password_;
    cur_conn = 0;
    for (size_t i = 0; i < maxnum; ++i) {
        MYSQL* con = nullptr;
        con = mysql_init(con);
        if (con == NULL) {
            LOG_ERROR("MySQL Error");
            exit(-1);
        }

        mysql_real_connect(con, host.c_str(), user.c_str(), password.c_str(), db.c_str(), port, NULL, 0);
        if (con == NULL) {
            LOG_ERROR("MySQL Error");
            exit(-1);
        }

        sql_pool.push_back(con);
        ++free_conn;
    }
    sql_sem = Semaphore(free_conn);
    max_conn = free_conn;
}

MYSQL* SqlConnPool::getConnection() {
    MYSQL* con = NULL;
    if (sql_pool.size() == 0) return NULL;
    sql_sem.wait();
    
    lst_mutex.lock();

    con = sql_pool.front();
    sql_pool.pop_front();
    free_conn--;
    cur_conn++;
    
    lst_mutex.unlock();
    return con;
}

bool SqlConnPool::freeConnection(MYSQL* con) {
    if (con == nullptr) {
        return false;
    }
    lst_mutex.lock();

    sql_pool.push_back(con);
    ++free_conn;
    --cur_conn;

    lst_mutex.unlock();
    sql_sem.post();
    return true;
}

void SqlConnPool::destroy() {
    lst_mutex.lock();
    if (sql_pool.size() > 0) {
        std::list<MYSQL*>::iterator it;

        for (it = sql_pool.begin(); it != sql_pool.end(); ++it) {
            MYSQL* con = *it;
            mysql_close(con);
        }
        cur_conn = 0;
        free_conn = 0;

        sql_pool.clear();
    }
    lst_mutex.unlock();
    return;
}

SqlConnPool::SqlConnPool() {
    cur_conn = 0;
    free_conn = 0;
}
SqlConnPool::~SqlConnPool() {
    destroy();
}
SqlConnPool* SqlConnPool::Instance() {
    static SqlConnPool connPool;
    return &connPool;
}
ConnectionRAII::ConnectionRAII(MYSQL** con, SqlConnPool* connPool) {
    
    *con = connPool->getConnection();
    
    conRAII = *con;
    poolRAII = connPool;
}
ConnectionRAII::~ConnectionRAII() {
    poolRAII->freeConnection(conRAII);
}