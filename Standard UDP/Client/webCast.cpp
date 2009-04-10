/*
THIS IS AN UNPUBLISHED WORK CONTAINING 2SPROUT INC CONFIDENTIAL AND PROPRIETARY INFORMATION. 
DISCLOSURE, USE, OR REPRODUCTION WITHOUT AUTHORIZATION OF 2SPROUT INC IS STRICTLY PROHIBITED.
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <curl/curl.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <queue>
#include <pthread.h>
#include <fstream>
#include <sstream>
#include <signal.h>
#include <sys/file.h>

#define MAXBUFLEN 50000
#define transferPipe "/tmp/transPipe"	
int MYPORT;		//port which the client is bound to 



char *ipAdd;
int sockfd;
struct sockaddr_in my_addr;    // my address information
struct sockaddr_in their_addr; // connector's address information
socklen_t addr_len;
int numbytes;
char buf[MAXBUFLEN];

using namespace std;

queue<string> sproutData;


void* getData(void *thread_arg)
{ 	     
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
    {
        perror("socket\n");
        exit(1);
    }

    my_addr.sin_family = AF_INET;         // host byte order
    my_addr.sin_port = htons(MYPORT);     // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    memset(my_addr.sin_zero, '\0', sizeof my_addr.sin_zero);


    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof my_addr) == -1) 
    {
        perror("bind\n");
        exit(1);
    }


	printf("Entering while\n"); 
	
 	while(1)
 	{
 
    	addr_len = sizeof their_addr;
	//	printf("Recieving\n");
   
    	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1)
    	{
        perror("recvfrom\n");
        exit(1);
   		}
	
	  //  printf("server: got connection from %s\n",inet_ntoa(their_addr.sin_addr));
        ipAdd = inet_ntoa(their_addr.sin_addr);
      //  printf("%s\n",ipAdd);
         
	 //   printf("got packet from %s\n",inet_ntoa(their_addr.sin_addr));
    	printf("packet is %d bytes long\n",numbytes);
    	buf[numbytes] = '\0';
    //	printf("packet contains \"%s\"\n",buf);	
    
    
    /*
    Before putting the item in the queue read the packet number and place it in the packet Array
    Check for the md5 hashsum before stripping the packet number from the message.
    */
    
    //Put information into queue

		if(numbytes < 5000 && sproutData.size() < 10000)
		{
   	    string input = buf;
		sproutData.push(input); //pused the data into the temparary queue
 		}
   }
       close(sockfd);

   
}



void* writeToClient(void *thread_arg)
{
	
	
	printf("Waiting to transmit Packets\n");
	int fd, ret_val, count, numread;

	printf("creating pipe for data transmission\n");

	
	ret_val = mkfifo(transferPipe, 0777); //create the pipe that will be used for transfering data back to user made app
	
	if (( ret_val == -1) && (errno != EEXIST)) 
	{
		perror("Error creating named pipe\n");
		exit(1);
	}

 	if(flock(fd,LOCK_EX) == -1) {

	fprintf(stderr, "flock(fd,LOCK_EX): %s (%i)\n", strerror(errno), errno);
	exit(0);

	} 
	
	printf("Succeeded!\n");
	
	while(1)
	{	
	

			if(usleep(100000) == -1)
			{
				printf("Sleeping Error\n");
			}			
			
		if(!sproutData.empty())
		{
			
			//Need to check for the size of the string, and make sure its not larger then our max string length
			
			printf("NOT EMPTY 67576576 \n");
			cout << "size " << sproutData.size() << endl;
			//start of critcal section
			string s = sproutData.front() + "\n";
			printf("Size of queue: %i", sproutData.size());
			//check the packet here
			sproutData.pop();
			//end of critical section
			fd = open(transferPipe, O_WRONLY); //open the pipe for writing
			write(fd,s.c_str(),strlen(s.c_str())); 	//write the string to the pipe
			close(fd); //close the connection to the pipe
		}
	}	
	
	if (flock(fd,LOCK_UN)==-1) 
	{
	fprintf(stderr, "flock(fd,LOCK_UN): %s (%i)\n", strerror(errno), errno);

	exit(0);

	}
}








int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		char port1[] = "4950";
		argv[1] = port1;	
	}
	
	//check to see if the port number is less then 1024 (reserved ports)
	
	if(atoi(argv[1]) <= 1024)
	{
		printf("Port Number is system reserved: Must be greater then 1024\n");
		exit(1);	
	}
	
		
		MYPORT = atoi(argv[1]);
		int rc, i , status;
		pthread_t threads[2];
		printf("Starting Threads...\n");
		pthread_create(&threads[0], NULL, getData, NULL);
		pthread_create(&threads[1], NULL, writeToClient, NULL);
		
		
		
		for(i =0; i < 2; i++)
		{
			rc = pthread_join(threads[i], (void **) &status); 
		}
		

}