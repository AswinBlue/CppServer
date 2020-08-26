#include <sys/types.h> 
#include <sys/socket.h> 
#include <stdio.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include <poll.h> 
#include <string.h> 
#include <stdlib.h> 
#include <signal.h> 
 
int ssock; 
  
int main(int argc, char** argv) { 
    char buf[BUFSIZ]; 
    int len; 
    char address[20]; 

    // 1. create server socket 
    ssock = socket(PF_INET, SOCK_STREAM, 0);        // TCP/IP 
    if (ssock == -1) { 
        perror("socket"); 
        return -1; 
    } 

    // 2. fill info 
    struct sockaddr_in saddr = { 0, }; // initialize as 0s 
    saddr.sin_family = AF_INET;     // use IP protocol 
    saddr.sin_port = htons(5252);   // host to network for short type 

    /*choose what you want*/ 
    //saddr.sin_addr.s_addr = inet_addr("YOUR_ADDRESS"); 
    saddr.sin_addr.s_addr = INADDR_ANY; 

    // set option 
    int option = 1; 
    setsockopt(ssock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof option); 

    // 3. bind info with socket 
    int ret = bind(ssock, (struct sockaddr *)&saddr, sizeof saddr); 
    if (ret == -1) { 
        perror("bind"); 
        return -1; 
    } 

    // 4. set maximum queue size as 5 
    ret = listen(ssock, 5); 
    if (ret == -1) { 
        perror("listen"); 
        return -1; 
    } 

    printf("runnig...\n\n"); 

    // 6. if server socket accepted new client 
    struct sockaddr_in caddr = {0, };       // initialize client address as 0s 
    socklen_t socklen = sizeof caddr; 
    int csock = accept(ssock, (struct sockaddr*)&caddr, &socklen); 
    if (csock == -1) { 
        perror("accept"); 
        return -1; 
    } 

    while(1){ 
        write(csock,"hello world",12); 
        sleep(10); 
    } 

    close(ssock); 
    close(csock); 
    return 0; 
}
