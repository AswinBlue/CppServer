#include <iostream>
#include "Server.h"
using namespace std;

int main(int argc, char* argv[]) {
    Server server;
    if (server.setup(NULL,1232) < 0){
        return -1;
    }
    if (server.start()) {
        return -1;
    }

    return 0;
}
