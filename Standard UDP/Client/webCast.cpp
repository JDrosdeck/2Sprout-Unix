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
#include <string>
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
#include <iostream>
#include "base64.h"


#define MAXBUFLEN 50000
#define transferPipe "/tmp/transPipe"
#define passPipe "/tmp/pass"

using namespace std;

	
int MYPORT;		//port which the client is bound to 



char *ipAdd;
int sockfd;
struct sockaddr_in my_addr;    // my address information
struct sockaddr_in their_addr; // connector's address information
socklen_t addr_len;
int numbytes;
char buf[MAXBUFLEN];

string secretKey;
string cipher;

string rawPacket;


queue<string> sproutData;

/*
The XOR function will decrypt/Encrypt a packet..For this case it is decrypting
*/
string XOR(string value,string key)
{
    string retval(value);

    short unsigned int klen=key.length();
    short unsigned int vlen=value.length();
    short unsigned int k=0;
    short unsigned int v=0;
    
    for(v;v<vlen;v++)
    {
        retval[v]=value[v]^key[k];
        k=(++k < klen ? k : 0);
    }
    
    return retval;
}


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
    	if ((numbytes = recvfrom(sockfd, (void *) rawPacket.c_str(), MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1)
    	{
        perror("recvfrom\n");
        exit(1);
   		}
	
        ipAdd = inet_ntoa(their_addr.sin_addr);
         
    //	printf("packet is %d bytes long\n",numbytes);
    	buf[numbytes] = '\0';
    //	printf("packet contains \"%s\"\n",rawPacket.c_str());	
    
    
    /*
		Before putting the packet recieved into the queue, decode from base64, then decrpyt with the cipher
		. Check that the secret keys match, then push into the queue.
    */
    

		if(numbytes < 5000 && sproutData.size() < 50000)
		{
   	    	string input = rawPacket.c_str();

			string decoded = base64_decode(input);
				
			printf("Starting Encrpytion...\n");
			string value(decoded);
			string key(cipher);
			value = XOR(decoded,key);
	//		cout << "Decrypted " << value << endl;
	//		cout << value.substr(0,8) << endl;
			if(value.substr(0,8) == secretKey)
			{
				sproutData.push(value.substr(8,value.length())); //pushed the data into the temparary queue
				printf("PUSHED\n");

			} 		

		}
   }
close(sockfd);   
}



/*
	This function will grab the secret key and the cipher from the client application
*/
void* getSecretKey(void *thread_arg)
{
	int fd, ret_val, count, numread;
	char bufpipe[5000];
	string tempMessage;
	while(1)
	{
			fd = open(passPipe, O_RDONLY); //open the pipe for reading

			numread = read(fd,bufpipe, 5000);
			if(numread > 1)
			{
				bufpipe[numread] = '\0';
				printf("%s\n", bufpipe);
				tempMessage = bufpipe;
				memset(bufpipe,'\0',5000 +1);
				
				//The message will come looking like secretKey Cipher (Space in between)
				
				string token;
				string command[2];	//messages passed through can have a maximum of 10 arguments. (should never be more then that)
				istringstream iss(tempMessage);
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
				
				if(command[0] != "" && command[1] != "")
				{
					secretKey = command[0];
					cipher = command[1];
					cout << "SECRET KEY " << secretKey << endl;
					cout << "CIPHER     " << cipher << endl;
				}

			
				
			}
	}
	
	
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


	printf("Succeeded!\n");
	
	while(1)
	{	
	

		if(usleep(1000) == -1)
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
		pthread_t threads[3];
		printf("Starting Threads...\n");
		pthread_create(&threads[0], NULL, getData, NULL);
		pthread_create(&threads[1], NULL, writeToClient, NULL);
		pthread_create(&threads[2], NULL, getSecretKey, NULL);
		
		
		for(i =0; i < 3; i++)
		{
			rc = pthread_join(threads[i], (void **) &status); 
		}
		

}