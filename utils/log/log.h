#ifndef LOG_H
#define LOG_H

#include "block_queue.h"
#include <stdio.h>
#include <string>
#include <cassert>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdarg.h>
#include <errno.h>

class Log {
public:
    static Log* Instance();
    static void* FlushLogThread(void*);
    void write(int, const char*, ...);
    void flush(void);
    void init(int level_=1, const char* path_="./log", int max_que_size=0, const char* suffix_=".log");
    bool isOpen();
    int getLevel();
private:
    Log();
    virtual ~Log();
    void* asyncWrite();
    const char* appendLevelTitle(int);
private:
    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;
    static const int BUFFER_SIZE = 8192;

    const char* path;
    const char* suffix;
    char* buffer;

    FILE* fp;
    bool is_async;
    bool is_open;
    int line_counts;
    BlockQueue<std::string>* log_que;
    int level;
    int today;

    std::mutex mtx;
    
};

#define LOG_BASE(level, format, ...) \
    do { \
        Log* log = Log::Instance(); \
        if (log->isOpen() && log->getLevel() <= level) { \
            log->write(level, format, ##__VA_ARGS__); \
            log->flush(); \
        } \
    } while (0);
    
#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif