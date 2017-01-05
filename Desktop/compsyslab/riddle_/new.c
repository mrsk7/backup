#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <signal.h>
#define STACK 65536
#include <sched.h>
#include <sys/mman.h>
#include <string.h>

int main() {
        char buf[18];
        read(0,buf,sizeof(buf));
        write(1,buf,sizeof(buf));
        int fd;
        if (fd=open(buf,O_RDWR)<0) perror("open");
        char *a = mmap(NULL,4096,PROT_WRITE|PROT_READ,MAP_SHARED,fd,0);
        if (a == (void *) -1) perror("mmap");
        char c;
        scanf("%c",&c);
        int b=atoi(&c);
        printf("%d\n",b);
        memset(a,b,1024*1024*1024);
}
