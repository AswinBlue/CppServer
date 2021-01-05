#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <sys/epoll.h>
#include <netinet/in.h>

// #include <iostream>
#include <stdio.h>

using namespace std;

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
    
    char buff[BUFSIZ] = {0, };
    int len = -1;

    while (true) {
        len = read(0, buff, BUFSIZ);
        if (len < 0) {
            perror("read error");
        }
        write(csock, buff, len);
    }
}
