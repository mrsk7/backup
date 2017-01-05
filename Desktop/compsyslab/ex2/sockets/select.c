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


int main(void) {	
	int n;
	char buf[100];
	fd_set rfds;
        struct timeval tv;
for (;;) {
		tv.tv_sec = 0;
	        tv.tv_usec = 0;
	        FD_SET(0,&rfds);
		if (select(1,&rfds, NULL, NULL, &tv) < 0)
	                perror("Select\n");
	        if (FD_ISSET(0,&rfds)) {
			fprintf(stderr, "Input inturrupt occured\n");
	        	n = read(0,buf,sizeof(buf));
			write(1,buf,n);
	                /*if (insist_write(1, buf, n) != n) {
	                	perror("write to remote peer failed");
	                        exit(1);
	                }*/
	        }
        }
        fprintf(stderr, "\nDone.\n");
        return 0;
}

