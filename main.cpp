#include "webserver.h"

int main(int argc, char* argv[]) {
    // 0 LT LT 1 ET LT 2 LT ET 3 ET ET
    // 0 siggle_reactor 1 multi_reactor
    int mode = 2;
    int pattern = 0;
    if (argc == 2) {
        mode = atoi(argv[1]);
    } else if (argc == 3) {
        mode = atoi(argv[1]);
        pattern = atoi(argv[2]);
    }
    WebServer webserver(9000, mode, pattern, true,
                        false, 512, 0, 8,
                        8, "localhost", "webserver", "webtest", "123456");
    webserver.start();

    return 0;
}
