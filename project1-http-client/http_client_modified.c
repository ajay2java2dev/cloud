
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


#define PORT "80" //standard default HTTP port
#define MAXDATASIZE 4096 // buffer size - max number of bytes we can get at once
#define SA struct sockaddr

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main (int argc, char **argv) {
    int sockfd, numbytes, n;
    int sendbytes;
    struct sockaddr_in servaddr;
    char sendline[MAXDATASIZE];
    char recvline[MAXDATASIZE];
    int rv;
    struct addrinfo hints, *servinfo, *p;
    char buf[MAXDATASIZE];
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) {
    	fprintf(stderr,"usage: http_client http://hostname[:port]/path_to_file\n");
	exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // Unspecified family
    hints.ai_socktype = SOCK_STREAM; // used for HTTP. Other one is SOCK_DRAM
    

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
	if ((sockfd = socket(p->ai_family, p->ai_socktype,
	  p->ai_protocol)) == -1) {
	  perror("client: socket");
	  continue;
        }

     	if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
	  close(sockfd);
	  perror("client: connect");
	  continue;
      }

      break;
    }

    if (p == NULL) {
	fprintf(stderr, "client: failed to connect\n");
	return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof s);

    sprintf(sendline, "GET / HTTP/1.0\r\n\r\n");
    sendbytes = strlen(sendline);

    if (write (sockfd, sendline, sendbytes) != sendbytes) {
        fprintf(stderr, "write_error");
    }

    memset (recvline, 0, MAXDATASIZE);

    while (( n = read(sockfd, recvline, MAXDATASIZE - 1)) > 0) {
        printf("%s", recvline);
    }

    if(n < 0) {
	perror("read error");
    }

    freeaddrinfo(servinfo); // all done with this structure

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	 perror("recv");
	 exit(1);
    }

    buf[numbytes] = '\0';

    close (sockfd);
    return 0;
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



