%module sprout


%{
extern int startFeed(int portNumber);
extern int stopFeed(int portNumber);
extern int getFeed();
extern char* getNextItem();

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
#include <sys/types.h>
#include <unistd.h>
#include <queue>

#define sproutPipe "/tmp/2sprout"
#define feedPipe "/tmp/feedPipe"
#define maxPipe		255
#define maxFeedPipe		1000



using namespace std;


%}

extern int startFeed(int portNumber);
extern int stopFeed(int portNumber);
extern char* getNextItem();
extern int getFeed();

