
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <curl/curl.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <queue>
#include <vector>
#include <algorithm>
#include <pthread.h>
#include <fstream>
#include <sstream>
#include <signal.h>
#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include "testDBConnection.h"
/*
Includes For UPNP Library
*/
#include "upnp/miniwget.h"
#include "upnp/miniupnpc.h"
#include "upnp/upnpcommands.h"
#include "upnp/upnperrors.h"
/*
Includes for md5 summing and base64
*/
#include "base64.h"
/*
Includes for postgres C library
*/
#include <libpq-fe.h>
#include <iomanip>
/*
Includes for mysql C library
*/
#include <my_global.h> 
#include <my_sys.h> 
#include <mysql.h> 


#define sproutPipe "/tmp/2sprout"	//pipe that takes in api calls from the user made application
#define maxPipe		50000			
#define MSGSZ     50000
#define version "1.0.1"

using namespace std;

void getData();
void catch_sigpipe(int sig_num);
string XOR(string value,string key);
void* announce(void *thread_arg);
int closeAnnounce();
void* castListener(void *thread_arg);
void* checkPacketReliability(void *thread_arg);
void* checkLostPacketsDay2(void *thread_arg);
void* replaceLostPacketsDay2(void *thread_arg);
void* checkLostPackets(void *thread_arg);
void* insertToDb(void *thread_arg);
int readConfig(string path);
void* getFeed(void *thread_arg);
void* createAndReadPipe(void *thread_arg);
void catch_int(int sig_num);
static void forwardPort(struct UPNPUrls * urls,struct IGDdatas * data, const char * iaddr,const char * iport,const char * eport,const char * proto);
void setUPNP(char* port);
bool registerClient();
void showHelp();
void showVersion();
static int writer(char *data, size_t size, size_t nmemb,std::string *writerData);
