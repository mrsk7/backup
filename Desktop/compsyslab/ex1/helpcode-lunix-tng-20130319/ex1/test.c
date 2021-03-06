#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr, "%s: wrong arguments\n", __func__);
		fprintf(stderr, "%s device bytes_to_read\n", argv[0]);
		exit(1);
	}

	int fd;
	if ((fd = open(argv[1], O_RDONLY)) <= 0) {
		perror("open");
		exit(1);
	}

	int ret, i;
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

	return 0;
}
	
	
	
