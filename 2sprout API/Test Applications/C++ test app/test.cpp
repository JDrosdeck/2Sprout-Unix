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
	getFeed();
	int x = 0;
	while(1)
	{
		printf("X: %i\n", x++ );
		char *test = getSproutItem();
		printf("%s", test);
		
	}
}
 


int main()
{

int rc, i , status;
pthread_t threads[1];
printf("Starting Threads...\n");
pthread_create(&threads[0], NULL, get2sproutFeed, NULL);


	for(i =0; i < 1; i++)
	{
		rc = pthread_join(threads[i], (void **) &status); 
	}


}






