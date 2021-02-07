#include "MySQL.h"

#define MAX_FD 1024
#define LISTEN_QUEUE 16

class Server {
private :
    int epoll_fd;
    int ssock;
    int connected_client;
    int client_fd[MAX_FD];
    MySQL* db;
public:
    ~Server();
    Server();
    int setup(char* addr, int port); // 'addr' can be NULL
    int start();
    int stop();
    int addClient(int csock);
    int removeClient(int csock);
    int handleClientPacket(struct epoll_event* event);
    int updateDB();
};
