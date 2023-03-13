#ifndef CONFIG_H
#define CONFIG_H

#include <unistd.h>
#include <cstdio>
#include <stdlib.h>

class Config {
public:
    int pattern;
    int listen_mode;
    int connect_mode;

public:
    Config();
    Config(int argc, char* argv[]);
    ~Config() {}
    int init(int argc, char* argv[]);
};

#endif