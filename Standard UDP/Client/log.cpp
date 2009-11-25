#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <time.h>

char* getTime();
using namespace std;

char Date[16];
void logFile(string textToLog)
{
	ofstream file("/tmp/2SproutClientLog.txt", ios::app);
	if(!file.is_open())
	{
		printf("Log file could not be opened");	
	}
	else
	{
		//get the date and time
		char* date = getTime();
		file << date << " " << textToLog << endl;
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
	strftime(Date,16, "%d%m%y->%I:%M:%S", timeinfo );
	return Date;
	

}