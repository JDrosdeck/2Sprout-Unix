/*
THIS IS AN UNPUBLISHED WORK CONTAINING 2SPROUT LLC CONFIDENTIAL AND PROPRIETARY INFORMATION. 
DISCLOSURE, USE, OR REPRODUCTION WITHOUT AUTHORIZATION OF 2SPROUT LLC IS STRICTLY PROHIBITED.
*/






/*
	CQueue is the acts as the actual global queue for the system. All the data that is read in from 
	Autocast is put into this queue. When there is data in the queue the it tries to write to a named pipe
	which the python application will read from and do all the functional work off of.
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <curl/curl.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <queue>
#include <pthread.h>
#include <fstream>
#include <sstream>
#include <signal.h>


#define QueueIncomingPipe "/tmp/CqueueIncoming"
#define QueueOutgoingPipe "/tmp/CqueueOutgoing"

#define MAXBUFLEN 50000

	using namespace std;


queue<string> sproutURLS; //queue which will hold the urls read from autocast



void* readIncomingURLS(void *thread_arg)
{
	
		int fd, ret_val, count, numread;
		char bufpipe[MAXBUFLEN];
		string word;

		ret_val = mkfifo(QueueIncomingPipe, 0777); //make the sprout pipe

		if (( ret_val == -1) && (errno != EEXIST)) 
		{
			perror("Error creating named pipe");
			exit(1);
		}

	//	fd = open(sproutPipe, O_RDONLY); //open the pipe for reading

		while(1)
		{
			fd = open(QueueIncomingPipe, O_RDONLY); //open the pipe for reading

			numread = read(fd,bufpipe, MAXBUFLEN);
			if(numread > 1)
			{
				bufpipe[numread] = '\0';
				printf("%s\n", bufpipe);
				word = bufpipe; 
				sproutURLS.push(word);
				printf("Pushed\n");
				memset(bufpipe,'\0',MAXBUFLEN +1);
				close(fd);
			}
		}
	
}


void* sendOutgoingURLS(void *thread_arg)
{
	int fd, ret_val, count, numread;
	ret_val = mkfifo(QueueOutgoingPipe, 0777); //create the pipe that will be used for transfering data back to user made app
	
	if (( ret_val == -1) && (errno != EEXIST)) 
	{
		perror("Error creating named pipe\n");
		exit(1);
	}
	
	
	while(1)
	{	
	

		if(usleep(1000) == -1)
		{
			printf("Sleeping Error\n");
		}			
			
		if(!sproutURLS.empty())
		{
			
			string s = sproutURLS.front() + "\n";
			sproutURLS.pop();
			fd = open(QueueOutgoingPipe, O_WRONLY); //open the pipe for writing
			write(fd,s.c_str(),strlen(s.c_str())); 	//write the string to the pipe
			close(fd); //close the connection to the pipe
			
			
		}
	}
	
}













int main(int argc, char *argv[])
{
	int rc, i , status;
	pthread_t threads[2];
	printf("Starting Threads...\n");
	pthread_create(&threads[0], NULL, readIncomingURLS, NULL);
	pthread_create(&threads[1], NULL, sendOutgoingURLS, NULL);
	
	
	
	for(i =0; i < 2; i++)
	{
		rc = pthread_join(threads[i], (void **) &status); 
	}
		

}