#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <cstring>
#include <iostream>
#include <list>

#include "User.h"
#include "Server.h"
#include "Packet.h"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include "Logger.h"

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
    this->is_running = false;
}

int Server::setup(char* addr, int port) {
    // connect to DB
    LOG_DEBUG("DB setting start");
    this->db = new MySQL("localhost", 33060, "RPG", "Lge123$%");
    LOG_DEBUG("DB setting success");

    // set epoll for multiplexing
    epoll_fd = epoll_create(1);
    if (epoll_fd == -1) {
        LOG_ERROR("epoll_create error");
        return -1;
    }
    LOG_DEBUG("EPOLL");
    // create socket
    ssock = socket(PF_INET, SOCK_STREAM, 0);        // TCP/IP
    if (ssock == -1) {
        LOG_ERROR("socket create error");
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
        LOG_ERROR("bind error");
        return -1;
    }
    // listen
    if (listen(ssock, LISTEN_QUEUE) == -1) {
        LOG_ERROR("lieten error");
        return -1;
    }
    connected_client = 0;

    // add ssock epoll_fd
    struct epoll_event e;
    e.events = EPOLLIN;
    e.data.fd = ssock;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ssock, &e) < 0) {
        LOG_ERROR("epoll_ctl add server socket error");
        return -1;
    }
    return 0;
}
int Server::start() {
    if (this->is_running) {
        LOG_WARN("epoll server is already running");
        return -1;
    }
    this-> is_running = true;
    LOG_DEBUG("start listening with thread");
    this->m_server_thread = boost::thread(boost::bind(&Server::_start, this));
    //LOG_DEBUG(m_server_thread.get_id());
    return 0;
}
int Server::_start() {
    struct epoll_event events[MAX_FD] = {0, };
    // infinite loop
    while (this->is_running) {
        // wait event forever
        int event_num = epoll_wait(epoll_fd, events, MAX_FD, -1);
        if (event_num == -1) {
            LOG_ERROR("epoll_wait error");
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
                    LOG_ERROR("accept error");
                    continue;
                }

                if (this->addClient(csock) == -1) {
                    LOG_ERROR("client connect from listener failed");
                }
            }
            else {
                // packet from client
                if (handleClientPacket(&(events[i])) == -1) {
                    LOG_ERROR("client packet handling failed");
                }
            }
        } // -> for
    } // -> while
    return 0;
}

int Server::stop() {
    if (!(this->is_running)) {
        LOG_WARN("cannot stop, epoll server is not running");
        return -1;
    }
    // close server, no more listening
    close(ssock);
    // close all client socket
    for (int i = 0; i < MAX_FD; i++) {
        if (client_fd[i] > 0) {
            close(i);
        }
    }
    close(epoll_fd);
    // change status
    this->is_running = false;
    // join thread
    this->m_server_thread.join();

    return 0;
}

int Server::addClient(int csock) {
    // add to epoll
    struct epoll_event e;
    e.events = EPOLLIN;
    e.data.fd = csock;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, csock, &e) < 0) {
        LOG_ERROR("epoll_ctl add client error");
        return -1;
    }
    cout << "new client joined : (" << csock << ")\n";
    client_fd[csock] = 1;
    connected_client++;
}

int Server::removeClient(int csock) {
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, csock, NULL) < 0) {
        LOG_ERROR("epoll_ctl remove client error");
        return -1;
    }
    cout << "client leaved : (" << csock << ")\n";
    client_fd[csock] = 0;
    connected_client--;
    return 0;
}

int Server::handleClientPacket(struct epoll_event *event) {
    // TODO : 버퍼 사이즈가 더 커져야 할 수도 있음
    // TODO : 데이터 받을 때마다 공간 정의하면 시간 더 걸리지 않나?
    char buff[BUFSIZ] = {0, };

    int len = read(event->data.fd, buff, BUFSIZ);
    if (len < 0) {
        LOG_ERROR("read error");
        return -1;
    }
    else if (len == 0) {
        removeClient(event->data.fd);
        return 0;
    }

    Packet *packet = (Packet*) buff;

    if (packet->opcode == USER_ID) {
        cout << "[from client(" << event->data.fd << ")] user id : " << "\n";
        for (int i = 0; i < 1024; i++) {
            cout << (packet->data)[i];
        }
        cout << "\n";
    }
    else if (packet->opcode == POSITION_INFO) {
        cout << "[from client(" << event->data.fd << ")] current position : " << "\n";

    }

    // for DEBUG
    cout << "\n[[";
    for (int i = 0; i < BUFSIZ; i++) {
        cout << buff[i];
    }
    cout << "]]\n";

    // cout << "from client(" << event->data.fd << ") : " << buff << "\n";
    // TODO : need to be coded
    return 0;
}

// TODO : this is test version, need to build up
int Server::updateDB() {
    LOG_INFO("updateDB");
    list<std::string> lst;
//    mysqlx::DbDoc doc(R"({"user": "a", "score": 1, "time": 1, "stage": 1})");
    
//        R"({"user": "b", "score": 100, "time": 1, "stage": 1}))",
//        R"({"user": "c", "score": 100, "time": 10, "stage": 1})",
//        R"({"user": "d", "score": 100, "time": 1, "stage": 3})");
    lst.push_back(R"({"user": "a", "score": 1, "time": 1, "stage": 1})");
    lst.push_back(R"({"user": "c", "score": 100, "time": 10, "stage": 1})");
    lst.push_back(R"({"user": "d", "score": 100, "time": 1, "stage": 3})");
    this->db->upload("RPG", "scoreboard", lst);
    mysqlx::DocResult docs = this->db->find("RPG", "scoreboard", "true");
    for (mysqlx::DbDoc doc : docs) {
        // LOG_DEBUG(doc.get_json());
        // TODO : find way to print with spdlog api
        std::cout << doc << std::endl;
        for (mysqlx::Field fld : doc) {
            // LOG_DEBUG("field: ", fld, "doc[field]: ", doc[fld]);
            std::cout << fld << " " << doc[fld] << std::endl;
        }
    }
    return 0;
}

