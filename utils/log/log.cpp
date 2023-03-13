#include "log.h"

Log::Log() {
    is_async = false;
    line_counts = 0;
    today = -1;
    level = -1;
    log_que = nullptr;
    fp = nullptr;
    buffer = nullptr;
}
Log::~Log() {
    if (buffer != nullptr) delete[] buffer;
    while (!log_que->empty()) {
        log_que->flush();
    }
    log_que->close();
    if (fp != nullptr) {
        std::lock_guard<std::mutex> locker(mtx);
        flush();
        fclose(fp);
    }
}
Log* Log::Instance() {
    static Log instance;
    return &instance;
}
int Log::getLevel() {
    std::lock_guard<std::mutex> locker(mtx);
    return level;
}
bool Log::isOpen() {
    return is_open;
}
void Log::init(int level_, const char* path_, int max_que_size, const char* suffix_) {
    is_open = true;
    level = level_;
    if (max_que_size > 0) {
        is_async = true;
        log_que = new BlockQueue<std::string>(max_que_size);
        pthread_t tid;
        pthread_create(&tid, NULL, FlushLogThread, NULL);
    } else {
        is_async = false;
    }

    time_t timer = time(NULL);
    struct tm* sysTime = localtime(&timer);
    struct tm t = *sysTime;
    path = path_;
    suffix = suffix_;
    char file_name[LOG_NAME_LEN] = {0};
    buffer = new char[BUFFER_SIZE];
    snprintf(file_name, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s", 
        path, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix);
    today = t.tm_mday;

    {
        std::lock_guard<std::mutex> lock(mtx);
        fp = fopen(file_name, "a");
        if (fp == nullptr) {
            mkdir(path, 0777);
            fp = fopen(file_name, "a");    
        }
        if (fp == NULL) {
            printf("%s\n", file_name);
            perror("error");
        }
        assert(fp != nullptr);
    }
}

void Log::write(int level, const char* format, ...) {
    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);
    time_t tSec = now.tv_sec;
    struct tm* sysTime = localtime(&tSec);
    struct tm t = *sysTime;
    va_list vaList;
    const char* levelTitle = appendLevelTitle(level);
    std::string log_str;

    if (today != t.tm_mday || (line_counts && (line_counts % MAX_LINES == 0))) {
        std::unique_lock<std::mutex> locker(mtx);
        locker.unlock();

        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
        
        if (today != t.tm_mday) {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", path, tail, suffix);
            today = t.tm_mday;
            line_counts = 0;
        } else {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", path, tail, (line_counts / MAX_LINES), suffix);
        }
        locker.lock();
        flush();
        fclose(fp);
        fp = fopen(newFile, "a");
        assert(fp != nullptr);
    }

    {
        std::unique_lock<std::mutex> locker(mtx);
        ++line_counts;
        int n = snprintf(buffer, BUFFER_SIZE - 1, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s", 
                        t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, 
                        t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec, levelTitle);
        va_start(vaList, format);
        int m = vsnprintf(buffer + n, BUFFER_SIZE - 1, format, vaList);
        buffer[n + m] = '\n';
        buffer[n + m + 1] = '\0';
        log_str = buffer;

        if (is_async && log_que && !log_que->full()) {
            log_que->push_back(log_str);
        } else {
            fputs(log_str.c_str(), fp);
        }
        
    }
}

const char* Log::appendLevelTitle(int level) {
    switch (level) {
    case 0:
        return "[DEBUG]:";
    case 1:
        return "[INFO]:";
    case 2:
        return "[WARN]:";
    case 3:
        return "[ERROR]:";
    default:
        return "[INFO]:";
    }
    return "";
}

void Log::flush() {
    if (is_async) {
        log_que->flush();
    }
    fflush(fp);
}

void* Log::asyncWrite() {
    std::string str = "";
    while (log_que->pop(str)) {
        std::lock_guard<std::mutex> locker(mtx);
        fputs(str.c_str(), fp);
    }
    return NULL;
}
void* Log::FlushLogThread(void* args) {
    Log::Instance()->asyncWrite();
    return NULL;
}