#include <iostream>
#include "Server.h"
#include "Logger.h"
#include "main.h"
#include "Asio.h"

#define PORT 3600
using namespace std;

// global variables
Server server;

bool init() {
    // log setting
    Logger *logger = new Logger(LOG_LEVEL_DEBUG);
    LOG_DEBUG("Logger setting success");

    // launch server
    LOG_DEBUG("launch server");
    if (server.setup(NULL, PORT) < 0){
        LOG_ERROR("Server initialization failed!");
        return false;
    }
    // TODO : for test, remove this
    server.updateDB();

    if (server.start()) {
        LOG_WARN("Server start failed!");
        return false;
    }
    return true;
}

bool test() {
    Asio a();

}

int main(int argc, char* argv[]) {
    /*
    if (init()) {
        return -1;
    }
    */
    test();
    return 0;
}
