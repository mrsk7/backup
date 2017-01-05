#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define N_CH	16

int main(int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr, "%s: wrong arguments\n", __func__);
		fprintf(stderr, "%s device bytes_to_read\n", argv[0]);
		exit(1);
	}

	int fd, i, status;
	if ((fd = open(argv[1], O_RDONLY)) <= 0) {
		perror("open");
		exit(1);
	}

	//pid_t pid[N_CH];
	pid_t pid;
	for (i=0; i<N_CH; i++) {
		//pid[i]=fork();
		pid=fork();
		if (!pid) break;
	}

	if (pid) {
		//parent
		printf("\tparent running\n");
		while(i>0) {
			printf("\tparent waiting\n");
			pid = wait(&status);
			printf("\tchild with pid=%ld died\n", (long) pid);
			i--;
		}
		printf("\tparent exiting after all children are dead...\n");
		if (close(fd) < 0) {
			perror("close");
			exit(1);
		}
		return 0;
	}

	// every child
	pid = getpid();
	printf("child running, pid=%ld\n", (long) pid);

	int ret;
	char data[32];
	ret = read(fd, data, atoi(argv[2]));
	if (ret < 0) {
		perror("read");
		exit(1);
	}
	else if (ret == 0) {
		printf("EOF\n");
		exit(1);
	}
	else {
		data[ret]='\0';
		printf("I read %d bytes\n", ret);
		for(i=0;i<ret;i++) {
			printf("%c", data[i]);
		}
		printf("%c\n", '$');
	}

	if (close(fd) < 0) {
		perror("close");
		exit(1);
	}

	printf("child with pid=%ld exiting...\n", (long) pid);
	return 0;
}
