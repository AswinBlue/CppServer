#include <iostream>
#include "Server.h"
#include "Logger.h"
#include "MySQL.h"

#define PORT 3600
using namespace std;

bool init() {
    // log setting
    Logger *logger = new Logger();
    LOG_DEBUG("Logger setting success");

    // connect to DB
    MySQL db("RPG:Lge123$%@localhost");

    // launch server
    Server server;
    if (server.setup(NULL, PORT) < 0){
        LOG_ERROR("Server initialization failed!");
        return false;
    }
    if (server.start()) {
        LOG_WARN("Server start failed!");
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (init()) {
        return -1;
    }
    return 0;
}
