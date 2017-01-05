#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

int main() {
    DIR *dir=opendir(".");
    struct dirent *entry;
    if (!dir) {
        printf("%s",strerror(errno));
        exit(EXIT_FAILURE);
    }
    while (1) {
        entry=readdir(dir);
        if (!entry) break;
        printf("%s\n",entry->d_name);
    }
    if (closedir(dir)!=0) {
        printf("%s",strerror(errno));
        exit(EXIT_FAILURE);
    }
    return 0;
}
        
