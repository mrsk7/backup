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
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include "socket-common.h"
#include <crypto/cryptodev.h>

#define DATA_SIZE       256
#define BLOCK_SIZE      16
#define KEY_SIZE        16  /* AES128 */

ssize_t length(unsigned char *buf)  {
	ssize_t ret;
	int i=0;
	while (buf[i]!='\n') i++;
	ret = i;
}

ssize_t insist_read(int fd, void *buf, size_t cnt)
{
        ssize_t ret;
        size_t orig_cnt = cnt;

        while (cnt > 0) {
                ret = read(fd, buf, cnt);
                if (ret < 0)
                        return ret;
                buf += ret;
                cnt -= ret;
        }

        return orig_cnt;
}


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

int client_init(struct sockaddr_in *sa_client, int port, struct hostent *hp) {
	int sd;
	if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
                perror("socket");
                exit(1);
        }
	//fprintf(stderr, "Created TCP socket\n");
	sa_client->sin_family = AF_INET;
        sa_client->sin_port = htons(port);
        memcpy(&(sa_client->sin_addr.s_addr), hp->h_addr, sizeof(struct in_addr));
	return sd;
}

int server_init(struct sockaddr_in *sa_server) {
	int sd;
	if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
                perror("socket");
                exit(1);
        }
	memset(sa_server, 0, sizeof(*sa_server));
        sa_server->sin_family = AF_INET;
        sa_server->sin_port = htons(TCP_PORT);
        sa_server->sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(sd, (struct sockaddr *)sa_server, sizeof(*sa_server)) < 0) {
                perror("bind");
                exit(1);
        }
        //fprintf(stderr, "Bound TCP socket to port %d\n", TCP_PORT);

        /* Listen for incoming connections */
        if (listen(sd, TCP_BACKLOG) < 0) {
                perror("listen");
                exit(1);
        }
	return sd;
}




int main(int argc, char *argv[])
{
	int sd,cfd,port,newsd,flag,i;
	ssize_t n;
	socklen_t len;
	char *hostname;
	struct hostent *hp;
	struct sockaddr_in sa,sa_server;
	fd_set rfds;
        struct timeval tv;
	char addrstr[INET_ADDRSTRLEN];

	if (argc != 3) {
		fprintf(stderr, "Usage: %s hostname port\n", argv[0]);
		exit(1);
	}

	struct session_op sess;
        struct crypt_op cryp;
        struct {
                unsigned char   in[DATA_SIZE],
                                encrypted[DATA_SIZE],
                                decrypted[DATA_SIZE],
                                iv[BLOCK_SIZE],
				key[KEY_SIZE];
        } data;
	/*Opening new cryptodev file descriptor*/
	cfd = open("/dev/crypto", O_RDWR);
        if (cfd < 0) {
                perror("open(/dev/crypto)");
                return 1;
        }

	/*Setting up basic crypting session parameters*/
        memset(&sess, 0, sizeof(sess));
        memset(&cryp, 0, sizeof(cryp));
	for (i=0;i<BLOCK_SIZE;i++) data.iv[i]='0';
	for (i=0;i<KEY_SIZE;i++) data.key[i]=i;
	sess.cipher = CRYPTO_AES_CBC;
        sess.keylen = sizeof(data.key);
        sess.key = data.key;
	if (ioctl(cfd, CIOCGSESSION, &sess)) {
                perror("ioctl(CIOCGSESSION)");
                return 1;
        }
	cryp.ses = sess.ses;
        cryp.len = sizeof(data.in);
        cryp.iv = data.iv;

	hostname = argv[1];
	port = atoi(argv[2]); /* Needs better error checking */

	/* Create TCP/IP socket, used as main chat channel */
	/* Look up remote hostname on DNS */
	if ( !(hp = gethostbyname(hostname))) {
		printf("DNS lookup failed for host %s\n", hostname);
		exit(1);
	}
        sd = client_init(&sa,port,hp);  
	/* Setting up client configuration */
	/* Connect to remote TCP port */
	fprintf(stderr, "Connecting to remote host... "); fflush(stderr);
	while (1) {
		if (connect(sd, (struct sockaddr *) &sa, sizeof(sa))< 0) {
			//fprintf(stderr, "Failed to connect to remote host: host not there. Now trying to host new chat session\n");
			if (close(sd) < 0)
                                        perror("close");
			sd = server_init(&sa_server);
			tv.tv_sec = 3;
		        tv.tv_usec = 0;
			FD_SET(sd,&rfds);
			flag = 1;
			len = sizeof(struct sockaddr_in);
			if (select(sd+1,&rfds,NULL,NULL,&tv)<0) {
				perror("select");
				exit(1);
			}
			if (FD_ISSET(sd,&rfds)) {
	                	if (( newsd = accept(sd, (struct sockaddr *)&sa_server, &len)) < 0) perror("accept");
				else goto hosting;
			}
			else {
				//fprintf(stderr, "Server has no clients. Trying again as a client\n");
				if (close(sd) < 0)
        				perror("close");
				sd=client_init(&sa,port,hp);
			}	
		}	
		else break;
	}
	fprintf(stderr, "Connected as client.\n");

	/* Read answer and write it to standard output */
        for (;;) {
		tv.tv_sec = 0;
	        tv.tv_usec = 0;
        	FD_SET(sd,&rfds);
	        FD_SET(0,&rfds);
        	if (select(sd + 1,&rfds, NULL, NULL, &tv) < 0) 
                        perror("Select\n");
                if (FD_ISSET(sd,&rfds)) {
                        n = read(sd, data.encrypted, sizeof(data.encrypted));
                        if (n <= 0) {
                                if (n < 0)
                                        perror("read from remote peer failed");
                                else
                                        fprintf(stderr, "Peer went away\n");
                                break;
                        }
			cryp.op = COP_DECRYPT;
			cryp.src = data.encrypted;
			cryp.dst = data.decrypted;
			if (ioctl(cfd, CIOCCRYPT, &cryp)) {
         		       perror("ioctl(CIOCCRYPT)");
		                return 1;
		        }
			i=0;
                        while (data.decrypted[i]!='\n') i++;
			write(1,"Peer: ",sizeof("Peer: "));
	                if (insist_write(1, data.decrypted, i+1) != i+1) {
                                perror("write to stdout failed");
                                break;
                        }

		}
                if (FD_ISSET(0,&rfds)) {
			n = read(0,data.in,sizeof(data.in));
			cryp.op = COP_ENCRYPT;
			cryp.src = data.in;
			cryp.dst = data.encrypted; 
			if (ioctl(cfd, CIOCCRYPT, &cryp)) {
                               perror("ioctl(CIOCCRYPT1)");
                                return 1;
                        }
                        if (insist_write(sd, data.encrypted, sizeof(data.encrypted)) != sizeof(data.encrypted)) {
                                perror("write to remote peer failed");
                                break;
                        }
                }
	}
        fprintf(stderr, "\nDone.\n");
	goto out;


hosting:
	fprintf(stderr,"Connected as host\n");
	if (!inet_ntop(AF_INET, &sa_server.sin_addr, addrstr, sizeof(addrstr))) {
                        perror("could not format IP address");
                        exit(1);
        }
        fprintf(stderr, "Incoming connection from %s:%d\n",addrstr, ntohs(sa_server.sin_port));
	FD_ZERO(&rfds);
	for (;;) {
		tv.tv_sec = 0;
	        tv.tv_usec = 0;
		FD_SET(newsd,&rfds);
		FD_SET(0,&rfds);
		if (select(newsd + 1,&rfds, NULL, NULL, &tv) < 0) 
			perror("Select\n");
		if (FD_ISSET(newsd,&rfds)) {
			n = read(newsd, data.encrypted, sizeof(data.encrypted));
        	        if (n <= 0) {
               			if (n < 0)
		        	        perror("read from remote peer failed");
	                	else
			                fprintf(stderr, "Peer went away\n");
		                break;
                	}
			cryp.op = COP_DECRYPT;
                        cryp.src = data.encrypted;
                        cryp.dst = data.decrypted;
			if (ioctl(cfd, CIOCCRYPT, &cryp)) {
                               perror("ioctl(CIOCCRYPT)");
                                return 1;
			}
			i=0;
			while (data.decrypted[i]!='\n') i++;
			write(1,"Peer: ",sizeof("Peer: "));
			if (insist_write(1, data.decrypted, i+1) != i+1) {
                                perror("write to stdout failed");
                                break;
                        }
		}
		if (FD_ISSET(0,&rfds)) {
			n = read(0,data.in,sizeof(data.in));
			cryp.op = COP_ENCRYPT;
                        cryp.src = data.in;
                        cryp.dst = data.encrypted;
			if (ioctl(cfd, CIOCCRYPT, &cryp)) {
                               perror("ioctl(CIOCCRYPT2)");
                                return 1;
                        }
                	if (insist_write(newsd, data.encrypted,sizeof(data.encrypted)) != sizeof(data.encrypted)) {
        	        	perror("write to remote peer failed");
		                break;
        	        }
		}
        }
out:	
	if (ioctl(cfd, CIOCFSESSION, &sess.ses)) {
                perror("ioctl(CIOCFSESSION)");
                return 1;
        }
	if (close(sd) < 0)
                perror("close");	
	if (close(newsd) < 0)
        	perror("close");
	if (close(cfd) < 0)
		perror("close");

	return 0;


}
