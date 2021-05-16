#include <iostream>
#include "Server.h"
#include "Logger.h"

#define PORT 3600

// open server with given port

bool InitInfra() {
    // log setting
    logger::Logger(LOG_LEVEL_TRACE);
    LOG_DEBUG("Logger setting success");
}

int main(int argc, char* argv[]) {
    // init Infrastructure
    if (InitInfra()) {
        return -1;
    }

    Server server(PORT);
    // start socket
    if (!server.StartServerSocket(-1, true)) {
        return -1;
    }
    // handle user data
    if (!server.StartUserDataBroadcasting()) {
        return -1;
    }

    while (true) {
        // sleep infinite
        LOG_DEBUG("[SERVER] time passed (+60)");
        sleep(60);
    }

    return 0;
}
