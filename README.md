# TinyWebserver

## 主要实现
* 线程池，工作队列（deque）
* socket连接（epoll ET，LT模式；模拟proactor，reactor）
* HTTP报文解析（状态机）
* 定时器（升序双向链表）
* 数据库连接池（单例模式）
* 同步/异步日志（阻塞队列）
* webserver封装
