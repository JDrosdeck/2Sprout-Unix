/*
THIS IS AN UNPUBLISHED WORK CONTAINING 2SPROUT INC CONFIDENTIAL AND PROPRIETARY INFORMATION. 
DISCLOSURE, USE, OR REPRODUCTION WITHOUT AUTHORIZATION OF 2SPROUT INC IS STRICTLY PROHIBITED.
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
#include <pqxx/pqxx>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <queue>
#include <poll.h>

using namespace std;
using namespace pqxx;
#define MAXBUFLEN 10000

#define SERVERPORT 4950 

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

    
    

result R;

queue<string> sproutFeed; //declare the queue

pthread_mutex_t mylock = PTHREAD_MUTEX_INITIALIZER;


/*
Sends the string at the front of the queue to all ip address's 
*/
void* sproutBroadcast(void *thread_arg)
{

//get the next string out of the queue

result tempResult;
tempResult = R;
//string s;
		
 while(1)
 {
 	if(sproutFeed.empty())
 	if(usleep(100000) == -1)
	{
		printf("Sleeping Error");
	}
	if(!sproutFeed.empty()) //while the queue has items
	{
		
		    pthread_mutex_lock(&mylock);
		string s (sproutFeed.front());
		printf("read in: %s\n", s.c_str());
		printf("Popping and sending\n");
		pthread_mutex_unlock(&mylock);

		for (result::const_iterator r = tempResult.begin(); r != tempResult.end(); ++r)
		{
			if(usleep(100000) == -1)
			{
				printf("Sleeping Error");
			}
			cout << r[0].c_str() << endl;
			sprintf(buffer,  s.c_str());
			buffer_length = strlen(buffer) + 200;
			
			he=gethostbyname(r[0].c_str());

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
		sproutFeed.pop();
		}
}
}

/*
Polls the server for updated information
*/

void pollServer()
{
 /*
  Run a query against the database and find all the active IP Address's
  */
  
  
        connection Conn("dbname=twosprout user=prop777#NIQtums password=*7Ur3~v4#");
		cout << "Connected to " << Conn.dbname() << endl;
 		work Xaction(Conn, "DemoTransaction");
	    try
	    {
	  		// (Queries)
	    	pthread_mutex_lock(&mylock);
	    	R =  Xaction.exec("SELECT remote_host FROM \"onClient_active\";");	
	  		cout << "Found " << R.size() << " Number:" << endl;
			pthread_mutex_unlock(&mylock);
		}
	    catch (...)
	    {
	    }
		Xaction.commit(); // ERROR: Xaction has already aborted!	

}

/*
Opens up the pipe for reading, Pushes all data read from the buffer into the queue
*/

void* localIPC(void *thread_arg)
{



    if ((sockfd1 = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    my_addr.sin_family = AF_INET;         // host byte order
    my_addr.sin_port = htons(4000);     // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    memset(my_addr.sin_zero, '\0', sizeof my_addr.sin_zero);



    if (bind(sockfd1, (struct sockaddr *)&my_addr, sizeof my_addr) == -1) {
        perror("bind");
        exit(1);
    }

 while(1)
 {

    addr_len = sizeof their_addr1;
   
    if ((simplePortBytes = recvfrom(sockfd1, buf1, MAXBUFLEN-1 , 0,
        (struct sockaddr *)&their_addr1, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }
	
	        printf("server: got connection from %s\n", \
            inet_ntoa(their_addr1.sin_addr));
            
            
            ipAdd = inet_ntoa(their_addr1.sin_addr);
            printf("%s\n",ipAdd);
         
            
	
    printf("got packet from %s\n",inet_ntoa(their_addr1.sin_addr));
    printf("packet is %d bytes long\n",simplePortBytes);
    buf1[simplePortBytes] = '\0';
    printf("packet contains \"%s\"\n",buf1);
    string input = buf1;
    pthread_mutex_lock(&mylock);
    sproutFeed.push(input);
    pthread_mutex_unlock(&mylock);
 	
   }
    close(sockfd);
}



/*
Sleeps for 10 secounds and then polls the server for updated ip addresses
*/

void* sleepPoll(void *thread_arg)
{
	while(1)
	{
		printf("Polling Server\n");
		pollServer();
		sleep(15);
	
	}
}

int main(int argc, char *argv[])
{
//	pollServer();
	int rc, i , status;
	pthread_t threads[2];
	printf("Starting Threads...\n");
//	pthread_create(&threads[0], NULL, sleepPoll, NULL);
	printf("Polling Thread Started\n");
	pthread_create(&threads[0], NULL, localIPC, NULL);
	printf("PipeThread Started\n");
	pthread_create(&threads[1], NULL, sproutBroadcast,NULL);
	printf("Broadcast Thread Started\n");

	for(i =0; i <2; i++)
	{
		rc = pthread_join(threads[i], (void **) &status); 
	}

return 0;
}


