#include "sprout.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sys/stat.h>
#include <pthread.h>

using namespace std;

 void* get2sproutFeed(void *thread_arg)
{

	while(1)
	{
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
rc = pthread_join(threads[0], (void **) &status); 
	


}






