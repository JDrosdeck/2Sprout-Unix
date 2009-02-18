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



#include "md5.h"




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

#define feedPipe "feedPipe"		//pipe that feeds data back through the api to the user made application
#define sproutPipe "2sprout"	//pipe that takes in api calls from the user made application
#define transferPipe "transPipe" //pipe used to transfer data from the cast app into the client

#define maxPipe		255			

#define MAXDATASIZE 1000
#define MAXBUFLEN 10000
#define MAX_BUF_SIZE 5000

//Used for md5 sum
#define MD5_CTX MD5_CTX
#define MDInit MD5Init
#define MDUpdate MD5Update
#define MDFinal MD5Final


//startFeed Vars

CURL *curl;
CURLcode res;
string url;
int MYPORT;		//port which the client is bound to 

//getData vars

bool useDatabase = true;
bool apiReadyToRecieve;

char *ipAdd;
int sockfd;
struct sockaddr_in my_addr;    // my address information
struct sockaddr_in their_addr; // connector's address information
socklen_t addr_len;
int numbytes;
char buf[MAXBUFLEN];

void getData();
queue<string> sproutFeed; //this is the queue where the approved data is located
queue<string> unprocessedData; //this is the queue for data that has yet been tested

pthread_mutex_t mylock = PTHREAD_MUTEX_INITIALIZER;
string connectionString;


vector<int> packetsRecieved;
vector<int> packetsMissed;
int packetNumberCount = 0;










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


/*
startFeed takes in one argument which is the string, the argument is not used right now.
The libcurl library is used to contact 2sprout.com over http using tcp protocol, for reliability.
By being directed to the webpage the ipaddress is added to the database, and feeds are then sent
to that particular client
*/
int announce()
{

  curl = curl_easy_init(); //initialize curl
  if(curl) 
  {
  	url = "http://2sprout.com/onClient/?port="; //access this webpage to be added to the database
    url +=MYPORT;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
  }

	printf("\n"); //A messages from the server will be recieved if the client is already active
    return 1;
}




/*
This will allow users using the API to stop recieving packets. It relays a POST to the server containing its port number
it will then delete the proper value from the database based on the ip address it gathers and its port number
*/
int closeAnnounce()
{
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
	command += buffer;
	command += " &";
	
	printf("Command is %s\n", command.c_str());
	i = system(command.c_str());
	
	
	
	//Once the command has been started We can listen on a pipe to recieve data that has been recieved
	
	int fd1, numread;
	char bufpipe[MAXDATASIZE];
	
	while(1)
	{
		fd1 = open(transferPipe, O_RDONLY);
		numread = read(fd1,bufpipe, MAXDATASIZE);
		if(numread > 1)
		{
		bufpipe[numread] = '\0';
		printf("Recieved %s from Feed Pipe\n", bufpipe);
		
		//find the actual size 
		
		
		unprocessedData.push(bufpipe);
		memset(bufpipe,'\0',MAXDATASIZE +1);
		close(fd1);
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
			//start of critical section
			string s = unprocessedData.front();
			unprocessedData.pop();
			pthread_mutex_unlock(&mylock);	
			
			//end of critial section
			
			//Tokenize the string
			
			string token;
			string section[5]; //a standard sproutcast should be made up of only 5 distince sections	
			istringstream iss(s);
			int count1 = 0;

			
			while(getline(iss,token,'^'))
			{
				section[count1] = token;
				cout << token << endl;
				count1++;
			}
		
			
			if((section[0]  != "") && section[1] != "" && section[2] != "" && section[3] != "" && section[4] != "" ) //make sure we have all the parts
			{	
				string CastMinusMD5 = section[1] + "^" + section[2] + "^" + section[3] + "^" + section[4]; //generate the origional string to grab the MD5 sum from	
				
			
				//check the md5 sum
				
				CastMinusMD5.erase(CastMinusMD5.find('\n'));
				
				string checkMd5 = MD5String(CastMinusMD5); //get the value of the MD5 string
				printf("MD5 SUM IS: %s", checkMd5.c_str());
				if(checkMd5 == section[0]) //The MD5 Sum is the same so data integrety is OK
				{
				
					printf("MD5 Just Fine! PASSED\n");
		  		
					//check the secret key
					if(section[1] == secretKey)
					{
						printf("Secret Key just fine! PASSED\n");
						
						if(currentDate == "") //this must be our first packet
							currentDate == section[2];
						else
							if(currentDate != section[2])
							{
								//the dates have changed so we should be reading to start recieving packet numbers starting at 0
								
							}
							
				
						
							//push the packet to the vector of recieved packet numbers
							packetsRecieved.push_back(atoi(section[3].c_str()));
						    	
							
							
					
					//if this passes add the packet number to the array of recieved packet numbers
			
					//Add only the message to the sproutQueue
					printf("Packet OK!!!!!!!!\n");
					pthread_mutex_lock(&mylock);
		 			sproutFeed.push(section[4]);
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








/*
TODO: ADD IN DATE DETECTION AND SHIT
Number will come in looking like month/day/year/packetNumber

*/

void* checkLostPackets(void *thread_arg)
{
	
	vector<int> tempVector;
	vector<int> brandNewPacket; //this stores packets #'s that we have not been searching for and need to check missing packets based on
	
	
	while(1)
	{
			printf("******************************\n");	
				
			sleep(15);
			if(!packetsRecieved.empty())
			{
			printf("******************************\n");	
			printf("Getting Ready to check packets\n");
			printf("******************************\n");	
			
			//start of critical section
			pthread_mutex_lock(&mylock);
			tempVector = packetsRecieved;
			pthread_mutex_unlock(&mylock);
			//end critial section
			printf("******************************\n");	
			printf("Clearing old Vector\n");
			printf("******************************\n");	
			packetsRecieved.clear();		
					
			//sort the tempVector
			printf("******************************\n");	
			printf("Sorting the Vector\n");
			printf("******************************\n");
			sort(tempVector.begin(),tempVector.end());
			
			
			
			int sizeOfVector = (int) packetsMissed.size();
					
			//Check the recieved packets against what was missing
					
			if(!packetsMissed.empty()) //there have been missed packets
			{
				printf("******************************\n");	
				printf("There have been missing packets\n");
				printf("******************************\n");
				//search for missed packets for anything new that may have come in
			    vector<int>::iterator searchMissingPackets;
						
			   	int loop;
				for(loop = 0; loop < sizeOfVector; loop++)
				{
					int match[] = {packetsMissed[loop]};
					
					cout << "looking for match: " << match[0] << endl;
					
					searchMissingPackets = search(tempVector.begin(),tempVector.end(),match, match+1);
					if(searchMissingPackets != tempVector.end())
					{
						printf("A missing packet has been found\n");
						//Missing packet has been found so delete it from the missing packet queue and the temp queue
						packetsMissed.erase(packetsMissed.begin()+searchMissingPackets[0]);
						//erase that value from the tempVector
						tempVector.erase(tempVector.begin()+loop);
						//decrease the loop amount so that it dosent skip a value
						loop--;
					}
					else //we were not waiting on this old packet so put in the actually recieved vector
					{
							printf("******************************\n");	
							printf("Not replacing lost packet...adding to new vector\n");
							printf("******************************\n");
							brandNewPacket.push_back(tempVector[loop]); //add this to the new vector
							printf("******************************\n");	
							printf("Erasing value from old vector\n");
							printf("******************************\n");
							tempVector.erase(tempVector.begin()); //delete from the old vector
							//loop--;//decrease the loop so as not to skip a value
					}
							
							//No matter what by this point tempVector should be totally empty
				}
												
			}
			else
			{
				
					printf("******************************\n");	
					printf("Not replacing lost packet...adding to new vector\n");
					printf("******************************\n");
					brandNewPacket = tempVector; //add this to the new vector
					printf("******************************\n");	
					printf("Erasing value from old vector\n");
					printf("******************************\n");
					tempVector.clear(); //delete from the old vector
			}
					
					
	
			//Figure out which are the missing packets
			int sizeOfNewPackets = (int) brandNewPacket.size();
					
			int i;
			printf("******************************\n");	
			printf("Calculating lost packets\n");
			printf("******************************\n");
			
			cout << "size of new packets: " << sizeOfNewPackets << endl;
			int remainder;
			for (i = 0; i < sizeOfNewPackets-1; i++)
			{
				cout << "packets: " <<  brandNewPacket[i+1] << " " << brandNewPacket[i] << endl;
				remainder = brandNewPacket[i+1] - brandNewPacket[i];
		
				
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
						cout << "pushing packet " << brandNewPacket[i] + j <<endl;
						
					 	packetsMissed.push_back(brandNewPacket[i] + j);
					
					}
							
				}
				cout << i << endl;
						
			}
			brandNewPacket.clear(); //empty this vector
						
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
				printf("read in: %s\n", s.c_str());
				printf("Putting into Database\n");
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
					printf("CLEAR!!!!!!!!!!!!!!!!!");
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
    int i;
    int command = 0; //used to keep track of what command has been read it
 	string line;
 	int found =0;
 	ifstream sproutConfig("2sprout.conf");
 	if(sproutConfig.is_open())
 	{
 		while(!sproutConfig.eof())
 		{
 			getline(sproutConfig,line);
 			if(!line.empty())
 			{
 		 	found = 0;

 			for(i = 0; i< line.length(); i++)
 			{
 				if(found == 0)
 				{
 					if(line.at(i) != ' ')
 					{
 						if(line.at(i) == '#')
 						{
 							break;
 						}
 						else
 						{
 						if(command == 0)
 						{
 							database = line;
 							found = 1;
 						}
 						if(database == "postgres")
 						{
 							if(command == 1)
 							{
 								host = line;
 								found = 1;
 							}
 							if(command == 2)
 							{
 								port = line;
 								found = 1;
 							}
 							if(command == 3)
 							{
 								dbname = line;
 								found = 1;
 							}
 							if(command == 4)
 							{
 								user = line;
 								found = 1;
 							}
 							if(command == 5)
 							{
 								pass = line;
 								found = 1;
 							}
 							if(command == 6)
 							{
 								table = line;
 								found = 1;
 							}
 							if(command == 7)
 							{
 								col = line;
 								found = 1;
 							}
 						}
 					else if(database == "mysql")
 					{
 						if(command == 1)
 							{
 								host = line;
 								found = 1;
 							}
 							if(command == 2)
 							{
 								port = line;
 								found = 1;
 							}
 							if(command == 3)
 							{
 								dbname = line;
 								found = 1;
 							}
 							if(command == 4)
 							{
 								user = line;
 								found = 1;
 							}
 							if(command == 5)
 							{
 								pass = line;
 								found = 1;
 							}
 							if(command == 6)
 							{
 								table = line;
 								found = 1;
 							}
 							if(command == 7)
 							{
 								col = line;
 								found = 1;
 							}
 					}
 					
 					else if(database == "none")
 					{
 						useDatabase = false;
 					
 					}
 					
 					command ++;
  					break;
  					}		
 				}
 				}
 			}
 			}
 			
 		}
 		sproutConfig.close();
 	}
 
 return 1;
 
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
		
		
		if(apiReadyToRecieve == true)
		{
			pthread_mutex_lock(&mylock);
			if(!sproutFeed.empty())
			{
				
				printf("NOT EMPTY \n");
				//start of critcal section
				string s = sproutFeed.front() + "\n";
				//check the packet here
				sproutFeed.pop();
				pthread_mutex_unlock(&mylock);
				//end of critical section
				fd = open(feedPipe, O_WRONLY); //open the pipe for writing
				write(fd,s.c_str(),strlen(s.c_str())); 	//write the string to the pipe
				close(fd); //close the connection to the pipe
			}
			else
			{
				pthread_mutex_unlock(&mylock);
				
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
	
//	fd = open(sproutPipe, O_RDONLY); //open the pipe for reading
	
	while(1)
	{
		fd = open(sproutPipe, O_RDONLY); //open the pipe for reading
		
		numread = read(fd,bufpipe, maxPipe);
		if(numread > 1)
		{
			bufpipe[numread] = '\0';
			printf("%s\n", bufpipe);
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
				announce();
			}
	
			if(command[0] == "getFeed")
			{
				apiReadyToRecieve = true;
				printf("done\n");
			}

		}

	}
}
 
 
  
/*

sproutClient takes 1 argument, which is the port number, This will be upated for username/password.
The defualt port used is 4950, otherwise set it to the user specified port. Once the port number
is relayed to the webserver the threads start for pening the UDP listener and Inserting data into the 
database
*/
int main(int argc, char *argv[])
{
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
		announce(); //announce to the server that we're ready to recieve
    	int rc, i , status;
		pthread_t threads[9];
		printf("Starting Threads...\n");
		pthread_create(&threads[0], NULL, castListener, NULL);
		printf("Socket Thread Started\n");
		pthread_create(&threads[1], NULL, insertToDb, NULL);		
		printf("InsertDB Thread Started\n");
		pthread_create(&threads[2], NULL, createAndReadPipe, NULL);
		printf("pipethread started \n");		
		pthread_create(&threads[3], NULL, checkPacketReliability, NULL);
		pthread_create(&threads[4], NULL, checkPacketReliability, NULL);
		pthread_create(&threads[5], NULL, checkPacketReliability, NULL);	
		pthread_create(&threads[6], NULL, checkPacketReliability, NULL);	
		pthread_create(&threads[7], NULL, checkPacketReliability, NULL);
		
		printf("lost packets starting\n");
		pthread_create(&threads[8], NULL, checkLostPackets, NULL);
		
		
		printf("Checking Packets \n");
		
		

		for(i =0; i < 9; i++)
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
		announce(); //announce to the server that we're ready to recieve
		int rc, i , status;
		pthread_t threads[8];
		printf("Starting Threads...\n");
		pthread_create(&threads[0], NULL, castListener, NULL);
		pthread_create(&threads[1], NULL, createAndReadPipe, NULL);
		pthread_create(&threads[2], NULL, getFeed, NULL);
		pthread_create(&threads[3], NULL, checkPacketReliability, NULL);	
		pthread_create(&threads[4], NULL, checkPacketReliability, NULL);		
		pthread_create(&threads[5], NULL, checkPacketReliability, NULL);
		pthread_create(&threads[6], NULL, checkPacketReliability, NULL);
		pthread_create(&threads[7], NULL, checkPacketReliability, NULL);
		
		printf("Checking Packets \n");
		
		pthread_create(&threads[8], NULL, checkPacketReliability, NULL);
		for(i =0; i < 8; i++)
		{
			rc = pthread_join(threads[i], (void **) &status); 
		}
	}
}


   
   


