/*
 * socket-client.c
 * Simple TCP/IP communication using sockets
 *
 * Vangelis Koukis <vkoukis@cslab.ece.ntua.gr>
 */

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include "socket-common.h"

/* Insist until all of the data has been written */
ssize_t insist_write(int fd, const void *buf, size_t cnt)
{
	ssize_t ret;
	size_t orig_cnt = cnt;
	
	while (cnt > 0) {
	        ret = write(fd, buf, cnt);
	        if (ret < 0)
	                return ret;
	        buf += ret;
	        cnt -= ret;
	}

	return orig_cnt;
}


void server_configuration(int sd,struct sockaddr_in *sa_t) {
	struct sockaddr_in sa = *sa_t;
	memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_port = htons(TCP_PORT);
        sa.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(sd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
                perror("bind");
                exit(1);
        }
        fprintf(stderr, "Bound TCP socket to port %d\n", TCP_PORT);

        /* Listen for incoming connections */
        if (listen(sd, TCP_BACKLOG) < 0) {
                perror("listen");
                exit(1);
        }
}

void client_configuration(struct sockaddr_in *sa,int port,struct hostent *hp) {
	sa->sin_family = AF_INET;
        sa->sin_port = htons(port);
        memcpy(&(sa->sin_addr.s_addr), hp->h_addr, sizeof(struct in_addr));
}

int main(int argc, char *argv[])
{
	int sd, port,newsd;
	ssize_t n;
	socklen_t len;
	char buf[100];
	char *hostname;
	struct hostent *hp;
	struct sockaddr_in sa;
	fd_set rfds;
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 50;
	char addrstr[INET_ADDRSTRLEN];

	if (argc != 3) {
		fprintf(stderr, "Usage: %s hostname port\n", argv[0]);
		exit(1);
	}
	hostname = argv[1];
	port = atoi(argv[2]); /* Needs better error checking */

	/* Create TCP/IP socket, used as main chat channel */
	if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}
	fprintf(stderr, "Created TCP socket\n");
	
	/* Look up remote hostname on DNS */
	if ( !(hp = gethostbyname(hostname))) {
		printf("DNS lookup failed for host %s\n", hostname);
		exit(1);
	}

	/* Connect to remote TCP port */
	fprintf(stderr, "Connecting to remote host... "); fflush(stderr);
	client_configuration(&sa,port,hp);
	if (connect(sd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
			fprintf(stderr, "Failed to connect to remote host. Now trying to host new chat session\n");
			perror("connect");
			exit(1);
	}
	else fprintf(stderr, "Connected\n");
}
