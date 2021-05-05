#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <sys/epoll.h>
#include <netinet/in.h>

#include "../Packet.h"
// #include <iostream>
#include <stdio.h>
#include <cstring>

int main(int argc, char* argv[]) {
    int csock;
    csock = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr = {0,};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(3600);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(connect(csock,(struct sockaddr*) &server_addr, sizeof(server_addr)) == -1){
        perror("connection to server failed");
    }
    
    Packet p;
    p.opcode = 1;
    strcpy(p.data, "testId is not exist");

    char* data = (char*) &p;

    printf("%ld\n", sizeof(p) );

    write(csock, data, sizeof(p));
}
