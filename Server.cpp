#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <cstring>
#include <iostream>

#include "User.h"
#include "Server.h"

using namespace std;

Server::~Server() {
    this->stop();
}

Server::Server() {
    // init variables
    this->epoll_fd = -1;
    this->ssock = -1;
    this->connected_client = 0;
    this->client_fd[MAX_FD] = {0, };
}

int Server::setup(char* addr, int port) {
    // set epoll for multiplexing
    epoll_fd = epoll_create(1);
    if (epoll_fd == -1) {
        perror("epoll_create error");
        return -1;
    }
    // create socket
    ssock = socket(PF_INET, SOCK_STREAM, 0);        // TCP/IP
    if (ssock == -1) {
        perror("socket create error");
        return -1;
    }

    // set socket for connection
    struct sockaddr_in server_addr = {0, };
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (addr == NULL) {
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        server_addr.sin_addr.s_addr = inet_addr(addr);
    }
    // set socket option
    int option = 1;
    setsockopt(ssock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof option);
    // bind socket info
    if (bind(ssock, (struct sockaddr *)&server_addr, sizeof server_addr) == -1) {
        perror("bind error");
        return -1;
    }
    // listen
    if (listen(ssock, LISTEN_QUEUE) == -1) {
        perror("lieten error");
        return -1;
    }
    connected_client = 0;

    // add ssock epoll_fd
    struct epoll_event e;
    e.events = EPOLLIN;
    e.data.fd = ssock;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ssock, &e) < 0) {
        perror("epoll_ctl add server socket error");
        return -1;
    }
}

int Server::start() {
    struct epoll_event events[MAX_FD] = {0, };
    // infinite loop
    while (true) {
        // wait event forever
        int event_num = epoll_wait(epoll_fd, events, MAX_FD, -1);
        if (event_num == -1) {
            perror("epoll_wait error");
            return -1;
        }

        // handle events
        for (int i = 0; i < event_num; i++) {
            if (events[i].data.fd == ssock) {
                // accept new client socket
                struct sockaddr_in client_addr = {0, };
                socklen_t caddr_len = sizeof client_addr;
                int csock = accept(ssock, (struct sockaddr *)&client_addr, &caddr_len);

                if (csock == -1) {
                    perror("accept error");
                    continue;
                }

                if (this->addClient(csock) == -1) {
                    perror("client connect from listener failed");
                }
            }
            else {
                // packet from client
                if (handleClientPacket(&(events[i])) == -1) {
                    perror("client packet handling failed");
                }
            }
        } // -> for
    } // -> while
}

int Server::stop() {
    close(ssock);
    for (int i = 0; i < MAX_FD; i++) {
        if (client_fd[i] > 0) {
            close(i);
        }
    }
    close(epoll_fd);
}

int Server::addClient(int csock) {
    // add to epoll
    struct epoll_event e;
    e.events = EPOLLIN;
    e.data.fd = csock;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, csock, &e) < 0) {
        perror("epoll_ctl add client error");
        return -1;
    }
    cout << "new client joined : (" << csock << ")\n";
    client_fd[csock] = 1;
    connected_client++;
}

int Server::removeClient(int csock) {
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, csock, NULL) < 0) {
        perror("epoll_ctl remove client error");
        return -1;
    }
    cout << "client leaved : (" << csock << ")\n";
    client_fd[csock] = 0;
    connected_client--;
}

int Server::handleClientPacket(struct epoll_event *event){
    char buff[BUFSIZ] = {0, };

    int len = read(event->data.fd, buff, BUFSIZ);
    if (len < 0) {
        perror("read error");
    }
    else if (len == 0) {
        removeClient(event->data.fd);
    }

    cout << "from client(" << event->data.fd << ") : " << buff << "\n";
    // TODO : need to be coded
}
