#include <iostream>
#include "Server.h"
#include "Logger.h"
#include "main.h"
#include <boost/thread.hpp>
#include <unistd.h>

#define PORT 3600
using namespace std;

bool initInfra() {
    // log setting
    Logger *logger = new Logger(LOG_LEVEL_DEBUG);
    LOG_DEBUG("Logger setting success");

}

void startNetwork() {
    // open server with given port
    Server server(3600);

    // launch server
    LOG_DEBUG("Set server socket");
    if (!server.Start()) {
        LOG_ERROR("Server initialization failed!");
    }
    // start listening
    while (true) {
        server.Update(-1);
    }
}

int main(int argc, char* argv[]) {
    if (initInfra()) {
        return -1;
    }

    boost::thread network_thread = boost::thread(boost::bind(&startNetwork));

    while (true) {
        // sleep infinite
        cout << "[SERVER] main sleep count +\n";
        sleep(60);
    }
    network_thread.join();
    return 0;
}
