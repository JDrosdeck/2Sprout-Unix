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



 void* get2sproutFeed(void *thread_arg)
{
	printf("Calling getFeed\n");
	getFeed();


}
 
 
 void* get2sproutItem(void *thread_arg)
{
	printf("waiting for the queue");
	while(1)
	{
	char* s = getNextItem();
	printf("%s", s);
	}

}


int main()
{
startFeed();

int rc, i , status;
pthread_t threads[2];
printf("Starting Threads...\n");
pthread_create(&threads[0], NULL, get2sproutFeed, NULL);
pthread_create(&threads[1], NULL, get2sproutItem, NULL);


	for(i =0; i < 2; i++)
	{
		rc = pthread_join(threads[i], (void **) &status); 
	}


}






