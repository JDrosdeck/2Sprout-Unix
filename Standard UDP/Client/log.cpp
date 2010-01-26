#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <time.h>
#include <pthread.h>


pthread_mutex_t fileLock = PTHREAD_MUTEX_INITIALIZER;



char* getTime();
using namespace std;

char Date[18];
void logFile(string textToLog, string level)
{
	ofstream file("/tmp/2Sprout.log", ios::app);
	if(!file.is_open())
	{
		printf("Log file could not be opened");	
	}
	else
	{
		//get the date and time
		pthread_mutex_lock(&fileLock);
		char* date = getTime();
		file << level << ": "<< date << " " << textToLog << endl;
		pthread_mutex_unlock(&fileLock);
		
		file.close();
	}
	
}

char* getTime()
{
	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	bzero(Date, sizeof(Date));
	strftime(Date,18, "%d/%m/%y->%I:%M:%S", timeinfo );	
	return Date;
	

}