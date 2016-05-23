//Defines and other information for chat client and server. Linux ubuntu.
#ifndef _SCHAT_LOCAL_H_
#define _SCHAT_LOCAL_H

#include <iostream>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>;

#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
// #include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <list>
// #include <time.h>
#include <stdlib.h>
#include <cstring>

// Default buffer size
#define BUFFER_SIZE 3100

// Default port
#define SERVER_PORT 3100

// Chat server ip # you should change it to your own server ip address
#define SERVER_HOST "127.0.0.1"

// Default timeout
#define EPOLL_RUN_TIMEOUT -1

// Count of connections that we are planning to handle
#define EPOLL_SIZE 10000

// First welcome message from server
#define STR_WELCOME "Welcome Client! You ID is: #%d"

// Format of message population
#define STR_MESSAGE "Client №%d>> %s"

// Warning message if you alone in server
#define STR_NOONE_CONNECTED "There are no connection to server except you!"

// Commad to exit
#define CMD_EXIT "EXIT"

// Macros - exit in any error (eval < 0) case
#define CHK(eval) if(eval < 0) {perror("eval"); exit(-1);}

// Macros - same as above, but save the result(res) of expression(eval)
#define CHK2(res, eval) if((res = eval) < 0) {perror("eval"); exit(-1);}

// Preliminary declaration of functions
int setnonblocking(int sockfd);
int handle_message(int new_fd);
int sendall(int s, char *buf, int len);

#endif
