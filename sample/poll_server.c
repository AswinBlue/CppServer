#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include <poll.h> 
#include <string.h> 
#include <stdlib.h> 
#include <signal.h> 
#include <fcntl.h> 

#include <sys/epoll.h>
#include<netinet/in.h>

#define POLL 0
#define EPOLL 1
#define TYPE 1
 
typedef struct client{ 
        int socket; 
        struct sockaddr_in address; 
        int file; 
        struct client *next; 
        struct client *prev; 
}Client; 
 
Client clients; 
int stdin_state = 0; 
 
void onExit(){ 
        Client* ptr = clients.next; 
        while(ptr != NULL){ 
                char *stopStr = "server has shutdown, stopping program\n"; 
                write(ptr->socket, stopStr, strlen(stopStr)); 
                close(ptr->socket); 
                free(ptr); 
                ptr = ptr->next; 
        } 
} 
 
void sigIntHandler(int pid){ 
        printf("\n==========\n\tstop process\n==========\n\n"); 
        fflush(stdout); 
        stdin_state = 0; 
} 
 
int main(int argc, char** argv) { 
	char buf[BUFSIZ]; 
	int len; 
	char address[20]; 
	char sbuf[BUFSIZ]; 
	int slen; 

	printf("<Commands>\n1. h : show commands\n2. c : connect to device\n3. d : disconnect to device\n4. s : server status\n5. exit : exit program\n\n"); 

	// SIGINT signal handling 
	signal(SIGINT,sigIntHandler); 

	// Client managing Linked_list head 
	clients.socket = 0; // count number of clients 
	clients.next = NULL; 
	clients.prev = NULL; 

	printf("runnig server...\n\n"); 

	// set I/O multiplexing 
#if (TYPE == POLL)
	struct pollfd poll_fds[32]; 
	int fd_cnt = 0; 
	// standard input 
	poll_fds[fd_cnt].fd = STDIN_FILENO; 
	poll_fds[fd_cnt].events = POLLIN; 
	++fd_cnt; 
#elif (TYPE == EPOLL)
    struct epoll_event *event;
#endif

	while(1){
		int ret = poll(poll_fds, fd_cnt, -1); 
		if(ret == -1) { 
			// perror("poll"); 
			// return -1; 
		} 
		else if(ret == 0){ 
			// time out 
			continue; 
		} 
		else{ 
			if(poll_fds[0].revents & POLLIN){ 
				// if command comes from 
				if(fgets(buf,sizeof(buf),stdin)==NULL) { 
					perror("fgets"); 
					continue; 
				} 
				buf[strlen(buf)-1] = '\0'; 

				switch(stdin_state){ 
					case 0: 
						if(strcmp(buf,"h")==0){ 
							printf("<Commands>\n1. h : show commands\n2. c : connect to device\n3. d : disconnect to device\n4. s : server status\n5. exit : exit program\n"); 
						} 
						else if(strcmp(buf,"c")==0){ 
							printf("<connect device> type 'q' to cancel\ntype device address : "); 
							fflush(stdout); 
							stdin_state = 1; 
						} 
						else if(strcmp(buf,"d")==0){ 
							printf("<disconnect device> type 'q' to cancel\nselect device to disconnect\n"); 
							Client *ptr = clients.next; 
							int num = 1; 
							while(ptr != NULL){ 
								printf("%d. %s\n",num,inet_ntoa(ptr->address.sin_addr)); 
								ptr = ptr->next; 
								num++; 
							} 
							fflush(stdout); 
							stdin_state = 2; 
						} 
						else if(strcmp(buf,"s")==0){ 
							printf("<Server Status> show client list\n"); 
							Client *ptr = clients.next; 
							int num = 1; 
							while(ptr != NULL){ 
								printf("%d. %s\n",num,inet_ntoa(ptr->address.sin_addr)); 
								ptr = ptr->next; 
								num++; 
							} 
							fflush(stdout); 
						} 
						else if(strcmp(buf,"exit")==0){ 
							onExit(); 
							return 0; 
						} 
						else{ 
							printf("unknwon command, type 'h' for help\n"); 
							fflush(stdout); 
						} 
						break; 
					case 1: 
						if(strcmp(buf,"q")==0){ 
							stdin_state = 0; 
							break; 
						} 

						printf("\nconnecting Device...\n"); 
						fflush(stdout); 

						// Connect To Device with given Address 
						int csock; 
						csock = socket(PF_INET, SOCK_STREAM, 0); 
						struct sockaddr_in server_addr = {0,}; 
						server_addr.sin_family = AF_INET; 
						server_addr.sin_port = htons(5252); 
						server_addr.sin_addr.s_addr = inet_addr(buf); 
						if(connect(csock,(struct sockaddr*) &server_addr, sizeof(server_addr)) == -1){ 
							printf("connection to client [%s] failed\n",buf); 
							fflush(stdout); 
							stdin_state = 0; 
							break; 
						} 

						// save client info in linked_list 
						Client *newc = (Client*)malloc(sizeof(Client)); 
						newc->address = server_addr; 
						newc->socket = csock; 
						newc->file = open(buf,O_WRONLY|O_APPEND|O_CREAT); 
						newc->next = clients.next; 
						newc->prev = &clients; 
						clients.socket++; 
						clients.next = newc; 

						// print client info 
						printf("[LogServer] new member joined\n  ip : %s\n  port : %d\n  total member : %d\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port), clients.socket); 
						fflush(stdout); 

						// I/O multiplexing 
						poll_fds[fd_cnt].fd = csock; 
						poll_fds[fd_cnt].events = POLLIN; 
						++fd_cnt; 

						stdin_state = 0; 
						break; 
					case 2: 
						if(strcmp(buf,"q")==0){ 
							stdin_state = 0;
							break; 
						} 
						int buf_to_int = atoi(buf); 
						if(!buf_to_int){ 
							printf("unavailable input\n"); 
							fflush(stdout); 
							stdin_state = 0; 
							break; 
						} 
						Client *ptr = clients.next; 
						for(int i=1; i<buf_to_int; i++){ 
							if(ptr == NULL) break; 
							ptr = ptr->next; 
						} 
						if(ptr == NULL){ 
							printf("unavailable input, cancel disconnecting\n"); 
							fflush(stdout); 
							stdin_state = 0; 
							break; 
						} 
						else{ 
							// close socket 
							close(ptr->socket); 
							poll_fds[ptr->socket] = poll_fds[--fd_cnt]; 
							clients.socket--; 

							printf("[LogServer] client has disconnected\n  address : %s\n  port : %d\n  total members : %d\n",inet_ntoa(ptr->address.sin_addr), ntohs(ptr->address.sin_port), clients.socket); 
							// free ptr 
							if(ptr->next != NULL){ 
								ptr->next->prev = ptr->prev; 
								ptr->prev->next = ptr->next; 
							} 
							else{ 
								ptr->prev->next = NULL; 
							} 
							free(ptr); 
						} 
						stdin_state = 0; 
						break; 
				}//->switch 
			} 
			else{ 
				// packet from client 
				for(int i=0; i<fd_cnt; i++){ 
					if(poll_fds[i].revents & POLLIN) { 
						len = read(poll_fds[i].fd, buf, sizeof buf); 
						// len > 0: read success 
						// len == 0: EOF, connection end 
						// len < 0: error, something's wrong 
						if (len == 0) { 
							// disconnect client 
							Client *ptr = clients.next; 
							while(ptr != NULL){ 
								if(ptr->socket == poll_fds[i].fd){ 
									break; 
								} 
								ptr = ptr->next; 
							} 
							if(ptr != NULL){ 
								close(poll_fds[i].fd); 
								close(ptr->file); 
								poll_fds[i] = poll_fds[--fd_cnt]; 
								clients.socket--; 
								printf("[LogServer] client has disconnected\n  address : %s\n  port : %d\n  total members : %d\n",inet_ntoa(ptr->address.sin_addr), ntohs(ptr->address.sin_port), clients.socket); 
								fflush(stdout); 
								// free 
								if(ptr->next != NULL){ 
									ptr->next->prev = ptr->prev; 
									ptr->prev->next = ptr->next; 
								} 
							} 
						} 
						else if (len == -1) { 
							perror("read"); 
						} 
						else{ 
							Client *ptr = clients.next; 
							while(ptr != NULL){ 
								if(ptr->socket == poll_fds[i].fd){ 
									break; 
								} 
								ptr = ptr->next; 
							} 

							// write data to file and stdout 
							if(ptr != NULL){ 
								write(ptr->file, buf, len); 
							} 
							snprintf(sbuf,BUFSIZ,"\n[client %d] :: ",ptr->socket); 
							write(STDOUT_FILENO,sbuf,len); 
							write(STDOUT_FILENO,buf,len); 
						} 
					}//if 
				}//for 
			}//else 
		} 
	}//while 
	onExit(); 
	return 0; 
}
