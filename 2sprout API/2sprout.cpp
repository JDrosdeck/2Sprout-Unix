/*THIS IS AN UNPUBLISHED WORK CONTAINING 2SPROUT INC CONFIDENTIAL AND PROPRIETARY INFORMATION. DISCLOSURE, USE, OR REPRODUCTION WITHOUT AUTHORIZATION OF 2SPROUT INC IS STRICTLY PROHIBITED.*/#include <stdio.h>#include <stdlib.h>#include <unistd.h>#include <errno.h>#include <string.h>#include <iostream>#include <ctype.h>#include <sys/stat.h>#include <fcntl.h>#include <fstream>#include <sys/types.h>#include <unistd.h>#include <cstdlib>#define sproutPipe "/tmp/2sprout"int startFeed(){		int fd,n;	char buffer[50];	n = sprintf(buffer, "startFeed");	fd = open(sproutPipe, O_WRONLY);	write(fd, buffer, strlen(buffer));	close(fd);	return 1;}int stopFeed(){	int fd,n;	char buffer[50];	n = sprintf(buffer, "stopFeed");	fd = open(sproutPipe, O_WRONLY);	write(fd, buffer, strlen(buffer));	close(fd);	return 1;}int getFeed(){	int fd,n;	char buffer[50];	n = sprintf(buffer, "getFeed");	fd = open(sproutPipe, O_WRONLY);	write(fd, buffer, strlen(buffer));	printf("getFeed api call complete");	close(fd);	return 1;}