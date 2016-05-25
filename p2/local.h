//Defines and other information for chat client and server. Linux ubuntu.
#ifndef _SCHAT_LOCAL_H_
#define _SCHAT_LOCAL_H

#include <iostream>

#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <list>
#include <stdlib.h>
#include <cstring>

// Default buffer size
#define BUFFER_SIZE 4096

// Default port
#define SERVER_PORT 3100

// Chat server ip # you should change it to your own server ip address
#define SERVER_HOST "127.0.0.1"

// Default timeout
#define EPOLL_RUN_TIMEOUT -1

// Count of connections that we are planning to handle
#define EPOLL_SIZE 8192

// First welcome message from server
#define STR_WELCOME "Welcome Client, #: #%d"

// Format of message population
#define STR_MESSAGE "Client №%d>> %s"

// Warning message if you alone in server
#define STR_NOONE_CONNECTED "You are the only one. (There are no connection to server except you!)ы"

// Commad to exit
#define CMD_EXIT "EXIT"

// Macros - exit in any error (eval < 0) case
#define CHK(eval) if(eval < 0) {perror("eval"); exit(-1);}

// Macros - same as above, but save the result(res) of expression(eval)
#define CHK2(res, eval) if((res = eval) < 0) {perror("eval"); exit(-1);}


//===================================================
// Preliminary declaration of functions
int setnonblocking(int sockfd);
int handle_message(int new_fd);
int sendall(int s, char *buf, int len);

#endif

// Setup nonblocking socket
int setnonblocking(int sockfd)
{
   CHK(fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK));
   return 0;
}

// Send full message

int sendall(int s, char *buf, int len)
{
    int total = 0;        
    int bytesleft = len; 
    int n;

    while(total < len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }


    return n==-1?-1:0; // In case of error it return -1
} 