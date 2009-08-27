
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
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
#include <string.h>
#include <cstdlib>
#include "testDBConnection.h"
#include "htmlGrabber.h"
#include "UPnP.h"
#include "strOperations.h"
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


int MYPORT = 0;		//port which the client is bound to 

bool useDatabase = true;

/*
Declarations for networking code
*/
char *ipAdd;
int sockfd;
struct sockaddr_in my_addr;    // my address information
struct sockaddr_in their_addr; // connector's address information
socklen_t addr_len;
int numbytes;
char buf[maxPipe];


queue<string> sproutFeed; //this is the queue where the approved data is located
queue<string> unprocessedData; //this is the queue for data that has yet been tested
vector<int> packetsRecieved;	//Stores any new packet number that comes in
vector<int> packetsRecievedDay2;
vector<int> packetsMissed;	//Stores the numbers of missed packets
vector<int> packetsMissedDay2;

bool dateRecieved = false;


/*
variables for holding data read in from the configuration files
*/
string apiKey;
string useUPNP;
string database;
string host;
string port; 
string dbname;
string user;
string pass;
string table;
string col;
string connectionString;

	
/*
strings used to keep track of the current date and the date of the next day
*/
string currentDate = "";
string nextDate = "";


string cipher; //used to decode the message
string updatedPassword;

string oldCipher;
string oldPassword;

int sleeptime = 0;
int msqid;




pthread_mutex_t mylock = PTHREAD_MUTEX_INITIALIZER;


using namespace std;

void getData();
void catch_sigpipe(int sig_num);
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
void catch_int(int sig_num);
bool registerClient();
void showHelp();
void showVersion();
bool disconnect();
