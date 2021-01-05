#include <iostream>
#include "Server.h"
#include "Logger.h"

#define PORT 3600
using namespace std;

int main(int argc, char* argv[]) {
    // log setting
    Logger *logger = new Logger();
    LOG_DEBUG("Logger setting success");

    // launch server
    Server server;
    if (server.setup(NULL, PORT) < 0){
        LOG_ERROR("Server initialization failed!");
        return -1;
    }
    if (server.start()) {
        LOG_WARN("Server start failed!");
        return -1;
    }
    return 0;
}
