
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
#include <vector>
#include <algorithm>
#include <pthread.h>
#include <fstream>
#include <sstream>
#include <signal.h>
#include <stdexcept>
#include "md5.h"
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




using namespace std;

#define feedPipe "/tmp/2sproutAPI"		//pipe that feeds data back through the api to the user made application
#define sproutPipe "/tmp/2sprout"	//pipe that takes in api calls from the user made application
#define transferPipe "/tmp/transPipe" //pipe used to transfer data from the cast app into the client
#define passPipe "/tmp/pass"

#define maxPipe		50000			

//Used for md5 sum
#define MD5_CTX MD5_CTX
#define MDInit MD5Init
#define MDUpdate MD5Update
#define MDFinal MD5Final


string url;
int MYPORT;		//port which the client is bound to 

//getData vars

bool useDatabase = true;
bool apiReadyToRecieve;

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


void getData();

queue<string> sproutFeed; //this is the queue where the approved data is located
queue<string> unprocessedData; //this is the queue for data that has yet been tested

pthread_mutex_t mylock = PTHREAD_MUTEX_INITIALIZER;
string connectionString;


vector<int> packetsRecieved;	//Stores any new packet number that comes in
vector<int> packetsRecievedDay2;

vector<int> packetsMissed;	//Stores the numbers of missed packets
vector<int> packetsMissedDay2;

vector<int> reSentMissedPackets;	//Stores the packet numbers of packets that have been sent to replace missed packets
vector<int> reSentMissedPacketsDay2;
bool dateRecieved = false;


//Used to hold database information
string database;
string host;
string port; 
string dbname;
string user;
string pass;
string table;
string col;
	
static MYSQL *conn;


string secretKey = "what"; //The Initial secretKey should be gatherd by a call made to 2sprout and gotten from our page
string currentDate = "";
string nextDate = "";

string cipher; //used to decode the message
//
//  libcurl variables for error strings and returned data
bool getFeedBool = false;

static char errorBuffer[CURL_ERROR_SIZE];
static std::string buffer;
//
//  libcurl write callback function
//
static int writer(char *data, size_t size, size_t nmemb,
                  std::string *writerData)
{
  if (writerData == NULL)
    return 0;

  writerData->append(data, size*nmemb);
  return size * nmemb;
}


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
/*
startFeed takes in one argument which is the string, the argument is not used right now.
The libcurl library is used to contact 2sprout.com over http using tcp protocol, for reliability.
By being directed to the webpage the ipaddress is added to the database, and feeds are then sent
to that particular client
*/
void* announce(void *thread_arg)
{	
		
		CURL *curl;
		CURLcode res;
		char Portbuffer[10];
		sprintf(Portbuffer, "%i", MYPORT);
	
		string url = "www.2sprout.com/onClient/?port=";
		url += Portbuffer;
		cout << url << endl;
		while(1)
		{
		curl = curl_easy_init();
    	if (curl == NULL)
    	{
    		fprintf(stderr, "Failed to create CURL connection\n");
    		exit(EXIT_FAILURE);
  		}

  		res = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
  		if (res != CURLE_OK)
  		{
    		fprintf(stderr, "Failed to set error buffer [%d]\n", res);
    		return false;
  		}

  		res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  		if (res != CURLE_OK)
  		{
    		fprintf(stderr, "Failed to set URL [%s]\n", errorBuffer);
    		return false;
  		}

 		res = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  		if (res != CURLE_OK)
  		{
    		fprintf(stderr, "Failed to set redirect option [%s]\n", errorBuffer);
    		return false;
  		}

  		res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
  		if (res != CURLE_OK)
  		{
    		fprintf(stderr, "Failed to set writer [%s]\n", errorBuffer);
    		return false;
  		}

  		res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
  		if (res != CURLE_OK)
  		{
    		fprintf(stderr, "Failed to set write data [%s]\n", errorBuffer);
    		return false;
  		}

  		res = curl_easy_perform(curl);
  		curl_easy_cleanup(curl);
  		cout << buffer << endl;

		//Convert the buffer from base64
  
  		string decoded = base64_decode(buffer);
  		cout << "Decoded: " <<  decoded << endl;
  		//XOR with the secret cypher
		string value(decoded);
		string key("23qwefwl;dg24t");
		value = XOR(decoded,key);
		cout << "Decrypted: " << value << endl;

		//parse the buffer Password^sleepTime

		string token;
		string section[3];	
		istringstream iss(value);
		int count1 = 0;

		while(getline(iss,token,'^'))
		{
			section[count1] = token;
			cout << token << endl;
			count1++;
		}

		if(section[0] != "" && section[1] != "" && section[2] != "")
		{
			cipher = section[2];
			string updatedPassword = section[0];
			int fd, ret_val, count, numread;
			ret_val = mkfifo(passPipe, 0777); //create the pipe that will be used for transfering data back to user made app

			if (( ret_val == -1) && (errno != EEXIST)) 
			{
				perror("Error creating named pipe\n");
				exit(1);
			}
	
			char buffer1[50];
			string messageToPass = section[0] + " " + section[2];
			int n = sprintf(buffer1, messageToPass.c_str());

			fd = open(passPipe, O_WRONLY);

			write(fd, buffer1, strlen(buffer1));
			close(fd);

			cout << "sleeping for:" << section[1] << endl;
			buffer.clear();
			sleep(atoi(section[1].c_str()));
	}
}
}




/*
This will allow users using the API to stop recieving packets. It relays a POST to the server containing its port number
it will then delete the proper value from the database based on the ip address it gathers and its port number
*/
int closeAnnounce()
{
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();
	if(curl)
	{
		url = "http://2sprout.com/client/close/?port=";
		url += MYPORT;
		curl_easy_setopt(curl,CURLOPT_URL, url.c_str());
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
	
	printf("\n"); //Will hold message from server when client is closed 
	return 1;
}

/*

*/
void* castListener(void *thread_arg)
{ 	     
	int i;
	char buffer[10];
	sprintf(buffer, "%i", MYPORT);
	
	string command = "./cast ";
	//command += buffer;
	//command += " &";
	
	printf("Command is %s\n", command.c_str());
	//i = system(command.c_str());
	
	pid_t pID = fork();
	if(pID == 0)
	{
		
		execl("cast","cast", buffer);
		
	}
	else if(pID > 0)
	{

		//Once the command has been started We can listen on a pipe to recieve data that has been recieved
		int fd1, numread;
		char bufpipe[maxPipe];
	
		while(1)
		{
			fd1 = open(transferPipe, O_RDONLY);
			numread = read(fd1,bufpipe, maxPipe);
			if(numread > 1)
			{
				bufpipe[numread] = '\0';
		//		printf("Recieved %s from Feed Pipe\n", bufpipe);
				//find the actual size 
				pthread_mutex_lock(&mylock);
				unprocessedData.push(bufpipe);
				pthread_mutex_unlock(&mylock);
				
				memset(bufpipe,'\0',maxPipe +1);
				close(fd1);
			}
		}
	}

}







/*
checkPacketReliablity() will check the validity of each packet and make sure that it is actually part of a 
2sprout broadcast.

format is as follows:

md5^secretKey^date^packetNumber^2sproutString


Multiple threads will run going through the unprocessed queue
Checking first the MD5 sum.
Then the secret Key 
If it passes then push it into the sproutQueue to put into the database
*/
void* checkPacketReliability(void *thread_arg)
{
	int x = 0;
	int y=0;
	while(1)
	{
		pthread_mutex_lock(&mylock);
		if(unprocessedData.empty())
 		{
			pthread_mutex_unlock(&mylock);
		
 				if(usleep(100000) == -1)
				{
					printf("Sleeping Error");
				}
		}
		else
		{
			pthread_mutex_unlock(&mylock);
		}
		pthread_mutex_lock(&mylock);
		if(!unprocessedData.empty()) //while the queue has items
		{
			x++;
			printf("X: %i\n", x);
			//start of critical section
			string s = unprocessedData.front();
			unprocessedData.pop();
			pthread_mutex_unlock(&mylock);	
			
			//end of critial section
			
			//Tokenize the string
			
			string token;
			string section[6]; //a standard sproutcast should be made up of only 6 distince sections	
			istringstream iss(s);
			int count1 = 0;

			
			while(getline(iss,token,'^'))
			{
				section[count1] = token;
				cout << token << endl;
				count1++;
			}
		
			
			if((section[0]  != "") && section[1] != "" && section[2] != "" && section[3] != "" && section[4] != "" && section[5] != "" ) //make sure we have all the parts
			{	
				string CastMinusMD5 = section[1] + "^" + section[2] + "^" + section[3] + "^" + section[4] + "^" + section[5]; //generate the origional string to grab the MD5 sum from	
				
			
				//check the md5 sum
				
				try
				{
				CastMinusMD5.erase(CastMinusMD5.find('\n'));
				}
				catch(out_of_range& e)
				{
					cout << "out of range " << e.what() << "\n";
				}
				catch(exception& e)
				{
					cout << "some of exception: " << e.what() << "\n";
				}
				
				
				string checkMd5 = MD5String(CastMinusMD5); //get the value of the MD5 string
				//printf("MD5 SUM IS: %s", checkMd5.c_str());
				if(checkMd5 == section[0]) //The MD5 Sum is the same so data integrety is OK
				{
				
					//printf("MD5 Just Fine! PASSED\n");
		  		
					//check the secret key
					if(section[1] == secretKey)
					{
						//printf("Secret Key just fine! PASSED\n");
						
						
						
						if(currentDate == "")
						{
							printf("Current Day initially set\n");
							currentDate = section[2];
						}
						
						if(section[2] != currentDate && nextDate == "")
						{
							printf("next Date initially set\n");
							nextDate = section[2];
						}
						
						if(section[2] == currentDate && section[4] != "0")
						{
							//printf("CurrentDate has been matched Pushing...\n");
							reSentMissedPackets.push_back(atoi(section[3].c_str()));
						}
						if(section[2] == currentDate && section[4] == "0")
						{
							packetsRecieved.push_back(atoi(section[3].c_str()));
							printf("NOT REPLACING PACKET pusing back packet number\n");	
								
						}
						if(section[2] == nextDate && section[4] != "0")
						{
							reSentMissedPacketsDay2.push_back(atoi(section[3].c_str()));
						}
						
						if(section[2] == nextDate && section[4] == "0")
						{
							printf("NextDate has been matched Pushing...\n");
							packetsRecievedDay2.push_back(atoi(section[3].c_str()));
							printf("NOT REPLACING PACKET pusing back packet number\n");	
						}
						
						if(section[2] != currentDate && section[2] != nextDate && dateRecieved == false)
						{
							if(!packetsMissed.empty())
								packetsMissed.clear();
							if(!packetsRecieved.empty())
								packetsRecieved.clear();
							
							printf("new date found, current date being overwritten\n");
							currentDate = section[2];
							packetsRecieved.push_back(atoi(section[3].c_str()));
							dateRecieved = true;	
						}
						else if(section[2] != currentDate && section[2] != nextDate && dateRecieved == true)
						{
							if(!packetsMissedDay2.empty())
								packetsMissedDay2.clear();
							if(!packetsRecievedDay2.empty())
								packetsRecievedDay2.clear();
							
							printf("new date found next date being overwritten\n");	
							currentDate = section[2];
							packetsRecieved.push_back(atoi(section[3].c_str()));
							dateRecieved = false;							
						}
										
					//if this passes add the packet number to the array of recieved packet numbers
			
					//Add only the message to the sproutQueue
					printf("Packet OK!!!!!!!!\n");
					pthread_mutex_lock(&mylock);
							y++;
							printf("Y: %i\n", y);
		 			sproutFeed.push(section[5]);
					pthread_mutex_unlock(&mylock);	
					}
					else{
					printf("Secret Key FAILED\n");}
				}
				else{
				printf("MD5 sum FAILED\n");}
			}
			else{
			printf("Not all data recieved FAIL!\n");}
		
		}
		else
		{
		pthread_mutex_unlock(&mylock);
		}
				
	}	
	
}




void* checkLostPacketsDay2(void *thread_arg)
{
	vector<int> tempVector;
	//vector<int> brandNewPacket; //this stores packets #'s that we have not been searching for and need to check missing packets based on
	
	
	while(1)
	{
			printf("******************************\n");	
				
			sleep(15);
			int sizeOfRecievedDay2 = (int) packetsRecievedDay2.size();

					//if(!packetsRecieved.empty())

			if(sizeOfRecievedDay2 > 1)
			{
			printf("******************************\n");	
			printf("Getting Ready to check packets\n");
			printf("******************************\n");	
			
			//start of critical section
			pthread_mutex_lock(&mylock);
			tempVector = packetsRecievedDay2;
			//end critial section
			printf("******************************\n");	
			printf("Clearing old Vector\n");
			printf("******************************\n");	
			packetsRecievedDay2.clear();		
			pthread_mutex_unlock(&mylock);
			
			//sort the tempVector
			printf("******************************\n");	
			printf("Sorting the Vector\n");
			printf("******************************\n");
			sort(tempVector.begin(),tempVector.end());
			
			int sizeOfTempVector = (int) tempVector.size();


			/*
				The Next two lines are for in in between recieving one section of packets
				and recieving the next section some packets may be lost
				say you recieved packets..1,2,3,4,7 and the missing packets were calculated
				and then the next set came in as 9,10,11. This will catch the missing packets
				between 7 and 9. I'm so awesome.
			*/

			int lastPacket = tempVector[sizeOfTempVector-1];
			packetsRecievedDay2.push_back(lastPacket);
			
			int sizeOfVector = (int) packetsMissedDay2.size();
					
			//Check the recieved packets against what was missing
					
			
			//Figure out which are the missing packets
			int sizeOfNewPackets = (int) tempVector.size();
					
			int i;
			printf("******************************\n");	
			printf("Calculating lost packets\n");
			printf("******************************\n");
			
			cout << "size of new packets: " << sizeOfNewPackets << endl;
			int remainder;
			for (i = 0; i < sizeOfNewPackets-1; i++)
			{
				cout << "packets: " <<  tempVector[i+1] << " " << tempVector[i] << endl;
				remainder = tempVector[i+1] - tempVector[i];
		
				
				if(( remainder != 1)) //if they are not sequential
				{
					printf("******************************\n");	
					printf("There are missing packets\n");
					printf("******************************\n");
					
					cout << "remainder: " << remainder << endl;
					int j;
					for(j = 1; j < remainder; j ++)
					{
						//add these values to the missing packet vector
						cout << "pushing packet " << tempVector[i] + j <<endl;
					 	packetsMissedDay2.push_back(tempVector[i] + j);
					
					}
							
				}
				cout << i << endl;
						
			}
			tempVector.clear(); //empty this vector
						
		}
		
				printf("******************************\n");	
				printf("No Missing packets WOOHOO\n");
				printf("******************************\n");
			//even if we havent recieved any new packets, every 15 secounds go back and request the old ones
		
			/*
			int sizeOfLostPackets = (int)packetsMissed.size();	
			curl = curl_easy_init(); //initialize curl
			int lostPacket;
			int l;
			for(l = 0; l < sizeOfLostPackets; l++)
			{
				lostPacket = packetsMissed[l];
				if(curl) 
				{	
    				url = "http://2sprout.com/lostPacket/?port="; //access this webpage to be added to the database
    				url += lostPacket;
   					curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    				// Perform the request, res will get the return code/
    				res = curl_easy_perform(curl);
    				if(usleep(100000) == -1)
					{
						printf("Sleeping Error");
					}	
    			}
    			curl_easy_cleanup(curl);
			}
			*/	
		
						
	}
	
}


void* replaceLostPacketsDay2(void *thread_arg)
{
	while(1)
	{
		sleep(20);
	pthread_mutex_lock(&mylock);
	
		if(!packetsMissedDay2.empty() && !reSentMissedPacketsDay2.empty()) //there have been missed packets
		{
			printf("******************************\n");	
			printf("There have been missing packets\n");
			printf("******************************\n");
			//search for missed packets for anything new that may have come in
		    vector<int>::iterator searchMissingPackets;
					int sizeOfVector = (int) packetsMissed.size();
					
		   	int loop;
		
			for(loop = 0; loop < sizeOfVector; loop++)
			{
				int match[] = {packetsMissedDay2[loop]};
				
				cout << "looking for match: " << match[0] << endl;
				

				printf("Searching for missing packet in newley recived packets\n");
				cout << "before Search " << reSentMissedPacketsDay2.size() << endl;
				

				if(reSentMissedPacketsDay2.empty() == false)
				{
					
					searchMissingPackets = reSentMissedPacketsDay2.begin();
					while(searchMissingPackets != reSentMissedPacketsDay2.end())
					{
						cout << "missingPacket: " << *searchMissingPackets << " Loop: " << *match << endl;
						if(*searchMissingPackets == *match)
						{
							printf("Found missing packet\n");
							reSentMissedPacketsDay2.erase(find( packetsMissedDay2.begin(), packetsMissedDay2.end(), *match) );
							packetsMissedDay2.erase(find( packetsMissedDay2.begin(), packetsMissedDay2.end(), *match) ); 						
							--loop;
							break;
						}
						else
						{
							++searchMissingPackets;
						}
					}
					
				}		
			}
			reSentMissedPacketsDay2.clear();
		
		}
		
		pthread_mutex_unlock(&mylock);
		if(!packetsMissedDay2.empty())
		{
				//even if we havent recieved any new packets, every 15 secounds go back and request the old ones

				/*
				int sizeOfLostPackets = (int)packetsMissedDay2.size();	
				curl = curl_easy_init(); //initialize curl
				int lostPacket;
				int l;
				for(l = 0; l < sizeOfLostPackets; l++)
				{
					lostPacket = packetsMissedDay2[l];
					if(curl) 
					{	
	    				url = "http://2sprout.com/lostPacket/?port="; //access this webpage to be added to the database
	    				url += lostPacket;
	   					curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	    				// Perform the request, res will get the return code/
	    				res = curl_easy_perform(curl);
	    				if(usleep(100000) == -1)
						{
							printf("Sleeping Error");
						}	
	    			}
	    			curl_easy_cleanup(curl);
				}
				*/
			
		}
}
}






/*
Number will come in looking like month/day/year/packetNumber
*/


void* checkLostPackets(void *thread_arg)
{

	printf("CHECK LOST PACKETS\n");
		vector<int> tempVector;
	//	vector<int> brandNewPacket; //this stores packets #'s that we have not been searching for and need to check missing packets based on


		while(1)
		{
				printf("******************************\n");	

				sleep(15);
				int sizeOfRecieved = (int) packetsRecieved.size();

				//if(!packetsRecieved.empty())

				if(sizeOfRecieved > 1)
				{
				printf("******************************\n");	
				printf("Getting Ready to check packets\n");
				printf("******************************\n");	

				//start of critical section
				pthread_mutex_lock(&mylock);
				tempVector = packetsRecieved;
				//end critial section
				printf("******************************\n");	
				printf("Clearing old Vector\n");
				printf("******************************\n");	
				packetsRecieved.clear();		
				pthread_mutex_unlock(&mylock);

				//sort the tempVector
				printf("******************************\n");	
				printf("Sorting the Vector\n");
				printf("******************************\n");
				sort(tempVector.begin(),tempVector.end());


				int sizeOfTempVector = (int) tempVector.size();


				/*
					The Next two lines are for in in between recieving one section of packets
					and recieving the next section some packets may be lost
					say you recieved packets..1,2,3,4,7 and the missing packets were calculated
					and then the next set came in as 9,10,11. This will catch the missing packets
					between 7 and 9. I'm so awesome.
				*/

				int lastPacket = tempVector[sizeOfTempVector-1];
				packetsRecieved.push_back(lastPacket);


				int sizeOfVector = (int) packetsMissed.size();

				//Check the recieved packets against what was missing





				//Figure out which are the missing packets
				int sizeOfNewPackets = (int) tempVector.size();

				int i;
				printf("******************************\n");	
				printf("Calculating lost packets\n");
				printf("******************************\n");

				cout << "size of new packets: " << sizeOfNewPackets << endl;
				int remainder;
				for (i = 0; i < sizeOfNewPackets-1; i++)
				{
					cout << "packets: " <<  tempVector[i+1] << " " << tempVector[i] << endl;
					remainder = tempVector[i+1] - tempVector[i];


					if(( remainder != 1)) //if they are not sequential
					{
						printf("******************************\n");	
						printf("There are missing packets DAY 1\n");
						printf("******************************\n");

						cout << "remainder: " << remainder << endl;
						int j;
						for(j = 1; j < remainder; j ++)
						{
							//add these values to the missing packet vector
							cout << "pushing packet " << tempVector[i] + j <<endl;

						 	packetsMissed.push_back(tempVector[i] + j);

						}

					}
					cout << i << endl;

				}

				tempVector.clear(); //empty this vector


			}


			


		}
	}



void* replaceLostPackets(void *thread_arg)
{
	
	printf("REPLACE LOST PACKETS\n");
	
	while(1)
	{
		sleep(20);
		pthread_mutex_lock(&mylock);
	
		if(!packetsMissed.empty() && !reSentMissedPackets.empty()) //there have been missed packets
		{
	//		printf("******************************\n");	
	//		printf("There have been missing packets\n");
	//		printf("******************************\n");
			//search for missed packets for anything new that may have come in
		    vector<int>::iterator searchMissingPackets;
			int sizeOfVector = (int) packetsMissed.size();
					
		   	int loop;
		
			for(loop = 0; loop < sizeOfVector; loop++)
			{
				int match[] = {packetsMissed[loop]};
				
	//			cout << "looking for match: " << match[0] << endl;
				

	//			printf("Searching for missing packet in newley recived packets\n");
	//			cout << "Size of reSent Packets before Search " << reSentMissedPackets.size() << endl;
				

				if(reSentMissedPackets.empty() == false)
				{
					
					searchMissingPackets = reSentMissedPackets.begin();
					while(searchMissingPackets != reSentMissedPackets.end())
					{
	//					cout << "Packet To Replace: " << *searchMissingPackets << " Replacing: " << *match << endl;
						if(*searchMissingPackets == *match)
						{
							printf("Found missing packet\n");
							 reSentMissedPackets.erase(find( reSentMissedPackets.begin(), reSentMissedPackets.end(), *match));
							cout << "SIZE" << reSentMissedPackets.size() << endl;
							packetsMissed.erase(find( packetsMissed.begin(), packetsMissed.end(), *match) ); 						
							--loop;
							break;
						}
						else
						{
							++searchMissingPackets;
						}
					}
					
				}		
			}
			reSentMissedPackets.clear();
		
		}
		pthread_mutex_unlock(&mylock);
		if(!packetsMissed.empty())
		{
			
	//		printf("NOTIFYING SERVER OF MISSED PACKETS\n");
				//even if we havent recieved any new packets, every 15 secounds go back and request the old ones

				/*
				int sizeOfLostPackets = (int)packetsMissed.size();	
				curl = curl_easy_init(); //initialize curl
				int lostPacket;
				int l;
				for(l = 0; l < sizeOfLostPackets; l++)
				{
					lostPacket = packetsMissed[l];
					if(curl) 
					{	
	    				url = "http://2sprout.com/lostPacket/?port="; //access this webpage to be added to the database
	    				url += lostPacket;
	   					curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	    				// Perform the request, res will get the return code/
	    				res = curl_easy_perform(curl);
	    				if(usleep(100000) == -1)
						{
							printf("Sleeping Error");
						}	
	    			}
	    			curl_easy_cleanup(curl);
				}
				*/
			
		}
	}
}















/*
insertToDb relays the information recieved from the listener
into the database
*/

void* insertToDb(void *thread_arg)
{
	if(database == "postgres")
	{

		connectionString = "host=" + host + " port=" + port + " dbname=" + dbname + " user=" + user + " password=" + pass;
		PGconn *Conn = PQconnectdb(connectionString.c_str());
		PGresult* result;
		
		if (PQstatus(Conn) == CONNECTION_BAD)
    	{
        	fprintf(stderr,"Failed to connect to database\n");
        	fprintf(stderr,"%s\n",PQerrorMessage(Conn));
        	PQfinish(Conn);
        	exit(1);
    	}

 		while(1)
 		{
			pthread_mutex_lock(&mylock);
 			if(sproutFeed.empty())
	 		{
				pthread_mutex_unlock(&mylock);
		
				if(usleep(100000) == -1)
				{
					printf("Sleeping Error");
				}	
			}
			else
			{
				pthread_mutex_unlock(&mylock);
				
			}
			
			
			pthread_mutex_lock(&mylock);
			if(!sproutFeed.empty()) //while the queue has items
			{
				//start of critical section
				string s = sproutFeed.front();
				//printf("read in: %s\n", s.c_str());
				//printf("Putting into Database\n");
				pthread_mutex_unlock(&mylock);
				//end of critical section
				string escapedString;
				int *error;
		 		unsigned long g = PQescapeStringConn(Conn, (char *)escapedString.c_str(), (char *)s.c_str(), strlen(s.c_str()),error);  
				cout << "Escaped String " << escapedString.c_str() << endl;
	   			try
	    		{
	  				// (Queries)
	  				string Query = "INSERT INTO ";
	  				Query = Query + "\""+ table + "\"" + " (" + col + ") " + "VALUES('";	
	  				Query = Query + escapedString.c_str();
	  				Query = Query +"');";
	  				cout << Query << endl;
	    			result = PQexec(Conn,Query.c_str());
					if (PQresultStatus(result) != PGRES_COMMAND_OK) 
					{
				             fprintf(stderr,"BEGIN command failed");
				             PQclear(result);
				    }
				  
					
					PQclear(result);
					//printf("CLEAR!!!!!!!!!!!!!!!!!");
					sproutFeed.pop();
					
	
				}
	    		catch (...)
	    		{
	    			//i failed...um fuck it
	    		}
	    

			}
			else
			{
				pthread_mutex_unlock(&mylock);
				
			}
		//PQfinish(Conn);
		}
	}


	if(database == "mysql")
	{
    	if (mysql_library_init(0,NULL,NULL))
		{
			cout << "Library init failed" << endl;
			exit(1);
		}
		
		conn = mysql_init (NULL);
	
    	if(conn == NULL)
    	{
    		cout << "Mysql initiation failed" << endl;
    		exit(1);
    	}
    
 
    	if(mysql_real_connect(conn, (char *)host.c_str(), (char *)user.c_str(), (char *)pass.c_str(), (char *)dbname.c_str(), atoi(port.c_str()), NULL,0) == NULL)
    	{
    		cout << "connection to server failed" << endl;
    		mysql_close(conn);
    		exit(1);
    	}
    
    	while(1)
 		{
			pthread_mutex_lock(&mylock);
		
 			if(sproutFeed.empty())
 			{
 				pthread_mutex_unlock(&mylock);
				
				if(usleep(100000) == -1)
				{
					printf("Sleeping Error");
				}
			}
			else
			{
				pthread_mutex_unlock(&mylock);
			}
		
			pthread_mutex_lock(&mylock);
		
			if(!sproutFeed.empty()) //while the queue has items
			{
				//start of critical section
				string s = sproutFeed.front();
				printf("read in: %s\n", s.c_str());
				printf("Putting into Database\n");
				pthread_mutex_unlock(&mylock);
				//end of critical section
		 		string escapedString;    
    			string mysqlQuery;
		   
		   
		  		unsigned long to_len = mysql_real_escape_string (conn, (char *)escapedString.c_str(), (char *)s.c_str(), strlen(s.c_str()));	//use the built in mysql function to put in escape characters...if ther are any	
				mysqlQuery = "INSERT INTO " + table + " ("+ col +") VALUES (\"" + escapedString.c_str() + "\");"; //actual creation of the sql statment
 				cout << mysqlQuery << endl;
 				if(mysql_query(conn, mysqlQuery.c_str()) != 0)
 		  		{
 		   			cout << "error query failed" << endl;
          		}
          
          		sproutFeed.pop();
			}
			else
			{
				pthread_mutex_unlock(&mylock);		
			}
    
		}    
		mysql_close(conn); //close the database connection
   	    mysql_library_end(); //stop using the library

	}

}   
   
   

   
/*
ReadConfig function, Goes through the 2sprout.conf file and reads in all the appropriate information 
used to create a connection with a database.
*/







int readConfig()
{
	string line;
	ifstream sproutConfig("2sprout.conf");
	size_t found;
	int foundPos;
	string firstSub, secoundSub;
	int numOfArgsFound = 0;
	if(sproutConfig.is_open())
	{
		while(!sproutConfig.eof())
		{
			getline(sproutConfig,line);
			if(!line.empty())
			{
				//find the first occurance of an '=' sign
				found = line.find("=");
				if(found!=string::npos)
				{
					foundPos = (int)found;
					firstSub = line.substr(0,found);
					secoundSub = line.substr(foundPos+1);				
					
						if(firstSub == "usedb" && secoundSub == "false")
						{
							useDatabase = false;	
							return 0;
						}
						else if(firstSub == "usedb" && secoundSub == "true")
						{
							numOfArgsFound++;
						}
						
						if(firstSub == "dbtype" && secoundSub != "")
						{
							database = secoundSub;
							numOfArgsFound++;
						}
						if(firstSub == "dbhost" && secoundSub != "") 
						{
							host = secoundSub;
							numOfArgsFound++;
						}
						if(firstSub == "dbport" && secoundSub != "") 
						{
							port = secoundSub;
							numOfArgsFound++;
						}
						if(firstSub == "dbname" && secoundSub != "") 
						{
							dbname = secoundSub;
							numOfArgsFound++;
						}
						if(firstSub == "dbuser" && secoundSub != "") 
						{
							user = secoundSub;
							numOfArgsFound++;
						}
						if(firstSub == "dbpassword" && secoundSub != "") 
						{
							pass = secoundSub;
							numOfArgsFound++;
						}
						if(firstSub == "dbtable" && secoundSub != "") 
						{
							table = secoundSub;
							numOfArgsFound++;
						
						}
						if(firstSub == "dbcol" && secoundSub != "") 
						{
							col = secoundSub;	
							numOfArgsFound++;
						}
				}	
			}	
		}
		
		cout << numOfArgsFound << endl;
		if(numOfArgsFound != 9)
		{
			printf("Configuration File is not Formatted Correctly...Exiting\n");
			exit(0);
			
		}
	}
}




void catch_sigpipe(int sig_num)
{
	printf("pipe is BROKEN\n");
	
}

 
 
/*
getFeed is an API function It allows users to not relay information directly into a database
but rather gives them access to the data via their own queue within the 2sprout library. 
Information is passed through the api via named pipes
*/
  
 void* getFeed(void *thread_arg)
{
	printf("Waiting to transmit Packets\n");
	int fd, ret_val, count, numread;

	printf("creating pipe for data transmission");

	
	ret_val = mkfifo(feedPipe, 0777); //create the pipe that will be used for transfering data back to user made app
	
	if (( ret_val == -1) && (errno != EEXIST)) 
	{
		perror("Error creating named pipe");
		exit(1);
	}

	int x = 0;
	while(1)
	{	
		pthread_mutex_lock(&mylock);
		if(!sproutFeed.empty())
		{
			printf("SIZE OF SPROUTFEED: %i\n", sproutFeed.size());
		}
		
		if(!sproutFeed.empty() && getFeedBool == true)
		{
			
			signal(SIGPIPE, catch_sigpipe);
			printf("NOT EMPTY \n");
			//start of critcal section
			string s;
			s.clear();
			s = sproutFeed.front();
			//check the packet here
			//end of critical section
			fd = open(feedPipe, O_WRONLY); //open the pipe for writing
			
			int sizeOfString = strlen(s.c_str());
			char sizeofStringBuffer[10];
			
			sprintf(sizeofStringBuffer, "%i", sizeOfString);
			string actualString = sizeofStringBuffer;
			int tempSize = strlen(sizeofStringBuffer);
			
			int remainder = 4 - tempSize;
			int x;
			for(x =0; x < remainder; x++)
			{
				actualString = actualString + "^";
			}
			
			
			//string SendString = sizeofStringBuffer;
			string SendString = actualString;
			
			//SendString = SendString + s;
			actualString = actualString + s;
				
			cout << "************************" << actualString << endl;
			
			int written = write(fd,actualString.c_str(),strlen(actualString.c_str())); 	//write the string to the pipe
			printf("WROTE: %i\n", written);
			close(fd); //close the connection to the pipe
			
			
			
			
			sproutFeed.pop();
			pthread_mutex_unlock(&mylock);
			
			
		}
		else
		{
			pthread_mutex_unlock(&mylock);
			if(usleep(1000) == -1)
			{
				printf("Sleeping Error");
			}
		}		
	}

}
 
 

 
 
 /*
 createAndReadPipe creates the pipe and then constantly reads
 waiting for calls from the API's
 */
 void* createAndReadPipe(void *thread_arg)
{

	int fd, ret_val, count, numread;
	char bufpipe[maxPipe];
	char copyPipe[maxPipe];
	string word;
	
	ret_val = mkfifo(sproutPipe, 0777); //make the sprout pipe
	
	if (( ret_val == -1) && (errno != EEXIST)) 
	{
		perror("Error creating named pipe");
		exit(1);
	}
	
	
	
	while(1)
	{
		fd = open(sproutPipe, O_RDONLY); //open the pipe for reading

		numread = read(fd,bufpipe, maxPipe);
		if(numread > 1)
		{
			bufpipe[numread] = '\0';
		//	printf("%s\n", bufpipe);
			word = bufpipe; 
			memset(bufpipe,'\0',maxPipe +1);
			close(fd);
			
			string token;
			string command[10];	//messages passed through can have a maximum of 10 arguments. (should never be more then that)
			istringstream iss(word);
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
		
		
			if(command[0] == "stopFeed")
			{
				closeAnnounce();
			}	
	
			if(command[0] == "startFeed")
			{
				//announce();
			}
	
			if(command[0] == "getFeed")
			{
				/*
				This will error out if the api starts a connection. Disconnects. Then restarts
				Possible fix, have the thread start in the Main. Then use a boolean to detect weither it's
				listening or not
				*/
				
				getFeedBool = true;
				
				//int rc, i , status;
				//pthread_t threads[1];
				//pthread_create(&threads[0], NULL, getFeed, NULL);
				printf("()()()()()()()()()()()()()()()()()()()()()()()()()Started getFeedThread\n");
			}

		}

	}

}
 

 

void catch_int(int sig_num)
{
    /* re-set the signal handler again to catch_int, for next time */
    signal(SIGINT, catch_int);
	unlink(feedPipe);
	unlink(sproutPipe);
	unlink(transferPipe); 
	unlink(passPipe);
	printf("Files deleted");
    fflush(stdout);
	exit(0);
}



  
/*

sproutClient takes 1 argument, which is the port number, This will be upated for username/password.
The defualt port used is 4950, otherwise set it to the user specified port. Once the port number
is relayed to the webserver the threads start for pening the UDP listener and Inserting data into the 
database
*/
int main(int argc, char *argv[])
{
	signal(SIGINT, catch_int); //redirect the signal so that when you press ctrl+c it deletes the named pipes
	
	apiReadyToRecieve = false;
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
	
	
	readConfig(); 	//read the configuration file for database access.

    if(useDatabase == true)
    {
	
		MYPORT = atoi(argv[1]); //set the port
	
		int rc, i , status;
		pthread_t threads[8];
		printf("Starting Threads...\n");
		
	
		
		pthread_create(&threads[0], NULL, announce, NULL);
	
		pthread_create(&threads[1], NULL, castListener, NULL);
		printf("Socket Thread Started\n");
		pthread_create(&threads[2], NULL, insertToDb, NULL);		
		printf("InsertDB Thread Started\n");
		pthread_create(&threads[3], NULL, checkPacketReliability, NULL);

		
		printf("lost packets starting\n");
		pthread_create(&threads[4], NULL, checkLostPackets, NULL);
		printf("checking for packets on day2\n");
		pthread_create(&threads[5], NULL, checkLostPacketsDay2, NULL);
		pthread_create(&threads[6], NULL, replaceLostPackets, NULL);
		
		pthread_create(&threads[7], NULL, replaceLostPacketsDay2, NULL);
			

		
		
		
		
		

		for(i =0; i < 8; i++)
		{
			rc = pthread_join(threads[i], (void **) &status); 
		}
		
		return 0;
	
	}

	/*
	If useDatabase is set to false Then the user is going to be using the sdk to interact with the feed
	Instead of announcing, and capturing data, just set up the pipe to allow api access
	*/
	else
	{
		printf("Not using Database\n");
		MYPORT = atoi(argv[1]); //set the port value
		
		// create the base folder	
	
	
	//	announce(); //announce to the server that we're ready to recieve
		int rc, i , status;
		pthread_t threads[9];		
		printf("Starting Threads...\n");
		pthread_create(&threads[0], NULL, announce, NULL);
		pthread_create(&threads[1], NULL, castListener, NULL);
		pthread_create(&threads[2], NULL, createAndReadPipe, NULL);
		pthread_create(&threads[3], NULL, checkPacketReliability, NULL);	

	
		printf("lost packets starting\n");
		pthread_create(&threads[4], NULL, checkLostPackets, NULL);
		printf("checking for packets on day2\n");
		pthread_create(&threads[5], NULL, checkLostPacketsDay2, NULL);
		pthread_create(&threads[6], NULL, replaceLostPackets, NULL);
		
		pthread_create(&threads[7], NULL, replaceLostPacketsDay2, NULL);
		pthread_create(&threads[8], NULL, getFeed, NULL);
		
		for(i =0; i < 9; i++)
		{
			rc = pthread_join(threads[i], (void **) &status); 
		}
	}
}


   
   


