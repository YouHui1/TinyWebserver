#include "config.h"

Config::Config() {
    listen_mode = 0;
    connect_mode = 0;
    pattern = 0;
}
Config::Config(int argc, char* argv[]) {
    listen_mode = 0;
    connect_mode = 0;
    pattern = 0;
    init(argc, argv);
}
int Config::init(int argc, char* argv[]) {
    const char* opt = "e:m:";
    int o;
    int epoll_mode;
    while ((o = getopt(argc, argv, opt)) != -1) {
        switch (o)
        {
        case 'e':
            epoll_mode = atoi(optarg);
            listen_mode = epoll_mode % 2;
            connect_mode = epoll_mode / 2;
            break;
        case 'm':
            pattern = atoi(optarg);
            break;
        case '?':
            return -1;
            break;
        
        default:
            return -1;
            break;
        }
    }
    return 0;
}