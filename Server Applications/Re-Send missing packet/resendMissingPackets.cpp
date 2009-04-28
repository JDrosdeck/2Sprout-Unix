/*
THIS IS AN UNPUBLISHED WORK CONTAINING 2SPROUT INC CONFIDENTIAL AND PROPRIETARY INFORMATION. 
DISCLOSURE, USE, OR REPRODUCTION WITHOUT AUTHORIZATION OF 2SPROUT INC IS STRICTLY PROHIBITED.
*/


/*
resenddataToSend.cpp This application will run in the background. When a post request comes from
a client saying that they are missing a packet django will write to a pipe called /tmp/sproutResendPacket
The format is IP address, Port Number, Date Of missing Packet, Number of missing packet.
*/
#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <queue>
#include <poll.h>

/*
Includes for postgres C library
*/
#include <libpq-fe.h>
#include <iomanip>


using namespace std;

#define MAXBUFLEN 10000
#define SERVERPORT 6050 
#define localpipeIPC "/tmp/sproutResendPacket"


/*
Network declarations
*/
int sockfd;
struct sockaddr_in their_addr; // connector's address information
struct hostent *he;
int numbytes;
char buffer[8192];
int buffer_length;
int sockfd1;
char *ipAdd;
struct sockaddr_in my_addr;    // my address information
struct sockaddr_in their_addr1; // connector's address information
socklen_t addr_len;
int simplePortBytes;
char buf1[MAXBUFLEN];

    
    

PGresult* result;
PGresult* tempResult;



queue<string> dataToSend; //declare the queue
queue<string> missingPacket;

pthread_mutex_t mylock = PTHREAD_MUTEX_INITIALIZER;


/*
Sends the string at the front of the queue to all ip address's 
*/
void* sproutBroadcast(void *thread_arg)
{

//get the next string out of the queue

//string s;
		
 while(1)
 {
 	if(dataToSend.empty())
 	if(usleep(100000) == -1)
	{
		printf("Sleeping Error");
	}
	if(!dataToSend.empty()) //while the queue has items
	{
		
		pthread_mutex_lock(&mylock);
		string s = (dataToSend.front());
		printf("read in: %s\n", s.c_str());
		printf("Popping and sending\n");
		pthread_mutex_unlock(&mylock);
		int i,j;
		if(!result || !(j = PQntuples(result)))
		{
			printf("Strange nothing is here\n");
			PQclear(result);
		}
		for(i =0; i < j; i++)
		{
			if(usleep(100000) == -1)
			{
				printf("Sleeping Error");
			}

			cout << PQgetvalue(result,i,0)<< endl;
			sprintf(buffer,  s.c_str());
			buffer_length = strlen(buffer) + 200;
			he=gethostbyname(PQgetvalue(result,i,0));

    			if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
    			{
        			perror("socket");
        			exit(1);
    			}

    			their_addr.sin_family = AF_INET;     // host byte order
    			their_addr.sin_port = htons(SERVERPORT); // short, network byte order
    			their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    			memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);

    			if ((numbytes = sendto(sockfd, buffer, buffer_length, 0,(struct sockaddr *)&their_addr, sizeof their_addr)) == -1) 
    			{
        			perror("sendto");
        			exit(1);
    			}

  	   		printf("sent %d bytes to %s\n", numbytes, inet_ntoa(their_addr.sin_addr));
			close(sockfd);
		}	
	dataToSend.pop();
	}
}
}

/*
Polls the server for updated information
*/

void* selectMissingPacket(void* thread_arg)
{
 /*
  Run a query against the database and find all the active IP Address's
  */
		PGconn *Conn = PQconnectdb("dbname=twosprout user=prop777#NIQtums password=*7Ur3~v4#");
  

		if (PQstatus(Conn) == CONNECTION_BAD)
  		{
      		fprintf(stderr,"Failed to connect to database\n");
      		fprintf(stderr,"%s\n",PQerrorMessage(Conn));
      		PQfinish(Conn);
      		exit(1);
  		}
		
		while(1)
		{
			if(!missingPacket.empty())
			{		
				printf("Retrieving missing packet from Queue\n");
				//PGconn *Conn = PQconnectdb("host=localhost port=5432 dbname=2sprout user=postgres password=jondii");		
	  			// (Queries)
	    	
	
				/*
				Tokenize the item from the queue by the spaces in the string
				*/
				
				string MissingValues = missingPacket.front();
				missingPacket.pop();
				string token;
				string command[10];	//messages passed through can have a maximum of 10 arguments. (should never be more then that)
				istringstream iss(MissingValues);
				int count1 = 0;


				/*
				This will tokenize the string and figure out what commands and arguments have been passed through from the API
				It will then call the specified functions with their arguments
				*/
				while(getline(iss,token,' '))
				{
					command[count1] = token;
					cout << token << endl;
					count1++;
				}
				
				
				
				if(command[0] != "" && command[1] != "" && command[2] != "" && command[3] != "")
				{	
					
					
					//Generate the string to be executed by the DB
					
					//String will be something like..SELECT packetData from Info where Date='010209', PacketNumber='405968685'
		
		
	    			tempResult =  PQexec(Conn,"SELECT remote_host FROM \"onClient_active\";");

					if (PQresultStatus(tempResult) != PGRES_TUPLES_OK) //successful return of tuples 
					{
		   				fprintf(stderr,"Nothing was found...strange");
		   				PQclear(tempResult);
					}
					else
					{
						int numberOfCLIENTS = PQntuples(tempResult);
						printf("Number of remote Clients:%i\n", numberOfCLIENTS);		
				  
						pthread_mutex_lock(&mylock);
						result = tempResult;
						pthread_mutex_unlock(&mylock);
						PQclear(tempResult);
					}
				}
			}		
		}
}

/*
Opens up the pipe for reading, Pushes all data read from the buffer into the queue
*/



void* localIPC(void *thread_arg)
{
	//Read from named pipe continuously for new data
	int fd, ret_val, count, numread;
	char bufpipe[5000];
	char copyPipe[5000];
	string word;
	
	ret_val = mkfifo(localpipeIPC, 0777); //make the sprout pipe
	if (( ret_val == -1) && (errno != EEXIST)) 
	{
		perror("Error creating named pipe");
		exit(1);
	}	
	while(1)
	{
		fd = open(localpipeIPC, O_RDONLY); //open the pipe for reading
		numread = read(fd,bufpipe, 5000);
		if(numread > 1)
		{
			bufpipe[numread] = '\0';
			printf("%s\n", bufpipe);
			word = bufpipe; 
			memset(bufpipe,'\0',5000 +1);
			dataToSend.push(word);
			close(fd);
		}
	}
}



/*
Sleeps for 10 secounds and then polls the server for updated ip addresses
*/


int main(int argc, char *argv[])
{
	//pollServer();
	int rc, i , status;
	pthread_t threads[3];
	printf("Starting Threads...\n");
	pthread_create(&threads[0], NULL, selectMissingPacket, NULL);
	printf("Polling Thread Started\n");
	pthread_create(&threads[1], NULL, localIPC, NULL);
	printf("PipeThread Started\n");

	pthread_create(&threads[2], NULL, sproutBroadcast,NULL);
	printf("Broadcast Thread Started\n");

	for(i =0; i <3; i++)
	{
		rc = pthread_join(threads[i], (void **) &status); 
	}

return 0;
}


