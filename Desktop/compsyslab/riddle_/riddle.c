#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int main() {
    char *argv[2]={"riddle",NULL};
    pid_t pid;
    int fd = open("test",O_CREAT,S_IRWXU);
    int fd2 = dup2(fd,99);
    execve("riddle",argv,NULL);
    return 0;
}
