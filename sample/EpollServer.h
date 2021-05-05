#include "MySQL.h"
#include <boost/thread.hpp>

#define MAX_FD 1024
#define LISTEN_QUEUE 16

class Server {
private :
    int epoll_fd;
    int ssock;
    int connected_client;
    int client_fd[MAX_FD];
    boost::thread m_server_thread;
    bool is_running;
    MySQL* db;
public:
    ~Server();
    Server();
    int setup(char* addr, int port); // 'addr' can be NULL
    int start();
    int _start();
    int stop();
    int addClient(int csock);
    int removeClient(int csock);
    int handleClientPacket(struct epoll_event* event);
    int updateDB();
};
