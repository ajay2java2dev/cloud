
#include "sys/socket.h"
#include "sys/types.h"
#include "signal.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "unistd.h"
#include "arpa/inet.h"
#include "stdarg.h"
#include "errno.h"
#include "fcntl.h"
#include "sys/time.h"
#include "sys/ioctl.h"
#include "netdb.h"


#define SERVER_PORT 80 //standard default HTTP port
#define MAXDATASIZE 4096 // buffer size - max number of bytes we can get at once
#define SA struct sockaddr

void err_n_die (const char *fmt, ...); //handle errors


int main (int argc, char **argv) {
    int sockfd, n;
    int sendbytes;
    struct sockaddr_in servaddr;
    char sendline[MAXDATASIZE];
    char recvline[MAXDATASIZE];


    if (argc != 2)
        err_n_die("usage: %s <server address>", argv[0]);

    //0 = TCP
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        err_n_die("Error while creating the socket");
    }

    //ZERO OUT ADDRESS
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT); //host to network

    //TRANSLATE THE ADDRESS. STRING TO IP
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
        err_n_die("inet_pton error for %s", argv[1]);

    //CONNECT TO SERVER ADDRESS
    if(connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
        err_n_die("connect failed");

    //AT THIS STAGE WE MUST BE CONNECTED AND READY TO SEND A GET HTTP 1.1 with end of my request \r\n\r\n
    sprintf(sendline, "GET / HTTP/1.1\r\n\r\n");
    sendbytes = strlen(sendline);

    if (write (sockfd, sendline, sendbytes) != sendbytes) {
        err_n_die("write_error");
    }

    //RECEIVE THE RESPONSE HERE
    memset (recvline, 0, MAXDATASIZE);

    while (( n = read(sockfd, recvline, MAXDATASIZE - 1)) > 0) {
        printf("%s", recvline);
    }

    if (n < 0) {
        err_n_die("read error");
    }

    exit(0);
}


void err_n_die (const char *fmt, ...) {
    int errno_save;
    va_list ap;

    errno_save = errno;

    va_start(ap,fmt);
    vfprintf(stdout,fmt, ap);
    fprintf(stdout,"\n");
    fflush(stdout);

    if(errno_save != 0) {
        fprintf(stdout, "(errno = %d) : %s\n ", errno_save,
                strerror(errno_save));
        fprintf(stdout,"\n");
        fflush(stdout);
    }
    va_end(ap);


    exit(1);
}


