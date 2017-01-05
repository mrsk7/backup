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
int fun(void * arg) {
    printf("CLONE\n");
    char buf[10];
   // read(0,buf,sizeof(buf));
    int i;
    //for (i=0;i<10;i++) printf("%c",buf[i]);
    printf("\n");
    char c;
    scanf("%c",&c);
    i=3;
    char *a = mmap(NULL,4096,PROT_WRITE|PROT_READ,MAP_SHARED,4,0);
    while (write(i,&c,sizeof(char))<0) {
        perror("open");
    }
    printf("OK OK OK OK OK\n");
    memset(a,atoi(&c),1024*1024*1024);
    return 0;
}

int main() {
    //char * const c[2] = {"riddle",NULL};
   // execve("riddle",c,NULL);
   // int fd=socket(PF_INET,SOCK_STREAM,0);
   // socklen_t len;
   // int newsd;
   // struct sockaddr_in ar;
   // ar.sin_family=AF_INET;
   // ar.sin_port=htons(49842);
   // ar.sin_addr.s_addr=htonl(INADDR_ANY);
   // int bd=bind(fd,(struct sockaddr *) &ar,sizeof(ar));
   // int l=listen(fd,1);
   // len = sizeof(struct sockaddr_in);
   // if ((newsd = accept(fd, (struct sockaddr *)&ar, &len)) < 0) perror("accept");
   // char buf[20];
   // read(newsd,buf,20*sizeof(char));
   // int i;
   // write(1,buf,sizeof(buf));
   // read(0,buf,sizeof(buf));
   // write(newsd,buf,sizeof(buf));
	 pid_t pid = getpid();
	 printf("%d\n",pid);
    char *addr=malloc(STACK);
    if (pid=clone(fun,addr+STACK,SIGCHLD|CLONE_VM|CLONE_FILES,NULL)<0) perror("clone");
    char * const c[2] = {"riddle",NULL};
    execve("riddle",c,NULL);
        
}
