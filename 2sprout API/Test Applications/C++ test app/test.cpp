#include "2sprout.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <pthread.h>

using namespace std;

#define feedPipe "/tmp/2sproutAPI"

 void* get2sproutFeed(void *thread_arg)
{
	printf("Calling getFeed\n");
	getFeed();
	int fd1, numread;
	char bufpipe[4];
	//string s;
	int x = 0;
	
		while(1)
		{
			fd1 = open(feedPipe, O_RDONLY);
			numread = read(fd1,bufpipe, 4);
			if(numread > 1)
			{
				printf("READ: %i\n", numread);
				x++;
				printf("X: %i\n", x);
				bufpipe[numread] = '\0';
				string temp = bufpipe;
				int pos = temp.find("^");
				if(pos != string::npos)
				{
					temp = temp.substr(0, pos);
				}
				
			
				
				printf("EMPTY bufPipe?: %s\n", temp.c_str());
			
			
				int sizeOfString = atoi(bufpipe);
				cout << "NEXT STRING BYTES: " << sizeOfString << endl;
				char feedWord[sizeOfString];
				int numRead1 = read(fd1, feedWord, sizeOfString);
				if(numRead1 > 1)
				{
					printf("READ IN : %s", feedWord);
					feedWord[sizeOfString] = '\0';
					memset(feedWord, '\0', sizeOfString);
				}
				
				memset(bufpipe, '\0', 4);
				close(fd1);
			}


		}


}
 


int main()
{

int rc, i , status;
pthread_t threads[2];
printf("Starting Threads...\n");
pthread_create(&threads[0], NULL, get2sproutFeed, NULL);


	for(i =0; i < 1; i++)
	{
		rc = pthread_join(threads[i], (void **) &status); 
	}


}






