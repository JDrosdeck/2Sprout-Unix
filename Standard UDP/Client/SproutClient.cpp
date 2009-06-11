
/*
THIS IS AN UNPUBLISHED WORK CONTAINING 2SPROUT INC CONFIDENTIAL AND PROPRIETARY INFORMATION. 
DISCLOSURE, USE, OR REPRODUCTION WITHOUT AUTHORIZATION OF 2SPROUT INC IS STRICTLY PROHIBITED.
*/
#include "sprout.h"


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
vector<int> reSentMissedPackets;	//Stores the packet numbers of packets that have been sent to replace missed packets
vector<int> reSentMissedPacketsDay2;

bool dateRecieved = false;


/*
variables for holding data read in from the configuration files
*/
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
int sleeptime = 0;



static char errorBuffer[CURL_ERROR_SIZE];
static std::string buffer;
pthread_mutex_t mylock = PTHREAD_MUTEX_INITIALIZER;



/*
Used for created a message for SYS V Message Queues, which is used for passing data from the client into the 
API for passing into a user made function
*/
typedef struct msgbuf {
         long    mtype;
         char    mtext[MSGSZ];
         } message_buf;


/*
Libcurl callback which will write the html recieved into memory instead of passing it to stdout
*/
static int writer(char *data, size_t size, size_t nmemb,
                  std::string *writerData)
{
  if (writerData == NULL)
    return 0;

  writerData->append(data, size*nmemb);
  return size * nmemb;
}



/*
Performs XOR operation on a string given a key
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
	
		string url = "http://2sprout.com/u/8573/Frer8n4drE/";
		//url += Portbuffer;
		cout << url << endl;
		while(1)
		{
			buffer.clear();
			memset(errorBuffer, '\0', sizeof(errorBuffer));
			sleep(sleeptime);	
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
		string key(cipher);
		value = XOR(decoded,key);
		cout << "Decrypted: " << value << endl;

		//parse the buffer Password^sleepTime

		//find the number of "^"
		int NumSpacesCount = 0;
		int loop;
		for(loop =0; loop < value.length(); loop++)
		{
			string::size_type loc = value.find('^', loop);
			if(loc != string::npos)
			{
				NumSpacesCount +=1;
			}	
		}		
		if(NumSpacesCount == 0)
		{
			NumSpacesCount = 1;
		}

		
		string token;
		string section[NumSpacesCount];	
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
			cipher = section[0];
			updatedPassword = section[1];
		

			cout << "CIPHER: " << cipher << endl;
			cout <<"PASS: " << updatedPassword << endl;
			sleeptime = atoi(section[2].c_str());
		
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
		string url = "http://2sprout.com/client/close/?port=";
		url += MYPORT;
		curl_easy_setopt(curl,CURLOPT_URL, url.c_str());
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
	
	printf("\n"); //Will hold message from server when client is closed 
	return 1;
}


void* castListener(void *thread_arg)
{ 	
	
	char *ipAdd;
	int sockfd;
	struct sockaddr_in my_addr;    // my address information
	struct sockaddr_in their_addr; // connector's address information
	socklen_t addr_len;
	int numbytes;
	string rawPacket;
	
	
	
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
    	if ((numbytes = recvfrom(sockfd, (void *) rawPacket.c_str(), 50000 , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1)
    	{
        	perror("recvfrom\n");
        	exit(1);
   		}
	
        ipAdd = inet_ntoa(their_addr.sin_addr);
         
    	buf[numbytes] = '\0';
    
		if(numbytes < 5000 && unprocessedData.size() < 50000)
		{
   	    	string input = rawPacket.c_str();
			rawPacket.clear();
				numbytes = 0;
			string decoded = base64_decode(input);
				
			printf("Starting Encrpytion...\n");
			string value(decoded);
			string key(cipher);
			value = XOR(decoded,key);
			decoded.clear();
			input.clear();
			if(value.substr(0,10) == updatedPassword)
			{
				unprocessedData.push(value.substr(10,value.length()));
				
				printf("PUSHED\n");
			} 		
			value.clear();

		}
   }
close(sockfd);
}







/*
checkPacketReliablity() will check the validity of each packet and make sure that it is actually part of a 
2sprout broadcast.

format is as follows:

md5^date^packetNumber^send\resend^2sproutString

*/
void* checkPacketReliability(void *thread_arg)
{
	int x = 0;
	int y=0;
	while(1)
	{
		//pthread_mutex_lock(&mylock);
		if(unprocessedData.empty())
 		{
			//pthread_mutex_unlock(&mylock);
		
 				if(usleep(1000) == -1)
				{
					printf("Sleeping Error");
				}
		}
		else
		{
			//pthread_mutex_unlock(&mylock);
		}
		//pthread_mutex_lock(&mylock);
		if(!unprocessedData.empty()) //while the queue has items
		{
			x++;
			printf("X: %i\n", x);
			//start of critical section
			string s = unprocessedData.front();
			unprocessedData.pop();
			//pthread_mutex_unlock(&mylock);	
			
			//end of critial section
			
			//Tokenize the string
			
			string token;
			string section[50]; //a standard sproutcast should be made up of only 6 distince sections	
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
				
				
				//string checkMd5 = MD5String(CastMinusMD5); //get the value of the MD5 string
				//printf("MD5 SUM IS: %s", checkMd5.c_str());
			
				md5wrapper md5;
				string checkMd5 = md5.getHashFromString(CastMinusMD5); 
				
				if(checkMd5 == section[0]) //The MD5 Sum is the same so data integrety is OK
				{
					CastMinusMD5.clear();
					checkMd5.clear();
					//printf("MD5 Just Fine! PASSED\n");
		  		
				
						
						
						if(currentDate == "")
						{
							printf("Current Day initially set\n");
							currentDate = section[1];
						}
						
						if(section[1] != currentDate && nextDate == "")
						{
							printf("next Date initially set\n");
							nextDate = section[1];
						}
						
						if(section[1] == currentDate && section[3] != "0")
						{
							//printf("CurrentDate has been matched Pushing...\n");
							reSentMissedPackets.push_back(atoi(section[2].c_str()));
						}
						if(section[1] == currentDate && section[3] == "0")
						{
							packetsRecieved.push_back(atoi(section[2].c_str()));
							printf("NOT REPLACING PACKET pusing back packet number\n");	
								
						}
						if(section[1] == nextDate && section[3] != "0")
						{
							reSentMissedPacketsDay2.push_back(atoi(section[2].c_str()));
						}
						
						if(section[1] == nextDate && section[2] == "0")
						{
							printf("NextDate has been matched Pushing...\n");
							packetsRecievedDay2.push_back(atoi(section[2].c_str()));
							printf("NOT REPLACING PACKET pusing back packet number\n");	
						}
						
						if(section[1] != currentDate && section[1] != nextDate && dateRecieved == false)
						{
							if(!packetsMissed.empty())
								packetsMissed.clear();
							if(!packetsRecieved.empty())
								packetsRecieved.clear();
							
							printf("new date found, current date being overwritten\n");
							currentDate = section[1];
							packetsRecieved.push_back(atoi(section[2].c_str()));
							dateRecieved = true;	
						}
						else if(section[1] != currentDate && section[1] != nextDate && dateRecieved == true)
						{
							if(!packetsMissedDay2.empty())
								packetsMissedDay2.clear();
							if(!packetsRecievedDay2.empty())
								packetsRecievedDay2.clear();
							
							printf("new date found next date being overwritten\n");	
							currentDate = section[1];
							packetsRecieved.push_back(atoi(section[2].c_str()));
							dateRecieved = false;							
						}
										
					//if this passes add the packet number to the array of recieved packet numbers
			
					//Add only the message to the sproutQueue
					printf("Packet OK!!!!!!!!\n");
					//pthread_mutex_lock(&mylock);
							y++;
							printf("Y: %i\n", y);
		 			sproutFeed.push(section[4]);
					//pthread_mutex_unlock(&mylock);	
			
				}
				else{
					
					ofstream myfile;
					myfile.open("md5Errors.txt",ios::out | ios::app);
					myfile << "String: " << CastMinusMD5 << "\n";
					myfile << "Client Created MD5: "<< checkMd5 << "\n";
					myfile << "Python Created MD5: " << section[0] << "\n\n";
					myfile.close();
					CastMinusMD5.clear();
					checkMd5.clear();
				printf("MD5 sum FAILED\n");}
			}
			else{
			printf("Not all data recieved FAIL!\n");}
		
		}
		else
		{
			//pthread_mutex_unlock(&mylock);
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
	//pthread_mutex_lock(&mylock);
	
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
		
		//pthread_mutex_unlock(&mylock);
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
					between 7 and 9.
				*/

				int lastPacket = tempVector[sizeOfTempVector-1];
				pthread_mutex_lock(&mylock);
				packetsRecieved.push_back(lastPacket);
				pthread_mutex_unlock(&mylock);
				

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
							pthread_mutex_lock(&mylock);
						 	packetsMissed.push_back(tempVector[i] + j);
							pthread_mutex_unlock(&mylock);
							
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
		//pthread_mutex_lock(&mylock);
	
		if(!packetsMissed.empty() && !reSentMissedPackets.empty()) //there have been missed packets
		{
			//search for missed packets for anything new that may have come in
		    vector<int>::iterator searchMissingPackets;
			int sizeOfVector = (int) packetsMissed.size();
		   	int loop;
			for(loop = 0; loop < sizeOfVector; loop++)
			{
				int match[] = {packetsMissed[loop]};
				//cout << "looking for match: " << match[0] << endl;
				//printf("Searching for missing packet in newley recived packets\n");
				//cout << "Size of reSent Packets before Search " << reSentMissedPackets.size() << endl;
				

				if(reSentMissedPackets.empty() == false)
				{
					
					searchMissingPackets = reSentMissedPackets.begin();
					while(searchMissingPackets != reSentMissedPackets.end())
					{
						//cout << "Packet To Replace: " << *searchMissingPackets << " Replacing: " << *match << endl;
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
		//pthread_mutex_unlock(&mylock);
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
 			if(sproutFeed.empty())
	 		{
				if(usleep(1000) == -1)
				{
					printf("Sleeping Error");
				}	
			}

			
			
			//pthread_mutex_lock(&mylock);
			else //while the queue has items
			{
				//start of critical section
				string s = sproutFeed.front();
				sproutFeed.pop();
				
				//printf("read in: %s\n", s.c_str());
				//printf("Putting into Database\n");
				//pthread_mutex_unlock(&mylock);
				//end of critical section
				string escapedString;
				int *error;
				
				char escapeBuffer[(s.length() * 2)+1];
				
		 		unsigned long g = PQescapeStringConn(Conn, escapeBuffer, (char *)s.c_str(), strlen(s.c_str()),error);  
				cout << "Escaped String " << escapeBuffer << endl;
								
	   				try
	    			{
	  					// (Queries)
	  					string Query = "INSERT INTO ";
	  					Query = Query + "\""+ table + "\"" + " (" + col + ") " + "VALUES('";	
	  					Query = Query + escapeBuffer;
	  					Query = Query +"');";
	  					cout << Query << endl;
	    				result = PQexec(Conn,Query.c_str());
						if (PQresultStatus(result) != PGRES_COMMAND_OK) 
						{
							
				    		fprintf(stderr,"BEGIN command failed");
							
				        	PQclear(result);
				    	}
						else
						{
							PQclear(result);
						}
						memset(escapeBuffer, '\0', sizeof(escapeBuffer) );
					}
	    			catch (...)
	    			{
	    			}
			}

		}
		//PQfinish(Conn);
	}

	if(database == "mysql")
	{
	 	MYSQL *conn;
		
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
		
 			if(sproutFeed.empty())
 			{
				if(usleep(1000) == -1)
				{
					printf("Sleeping Error");
				}
			}
	
		
			else//while the queue has items
			{
				//start of critical section
				string s = sproutFeed.front();
				printf("read in: %s\n", s.c_str());
				printf("Putting into Database\n");
				//pthread_mutex_unlock(&mylock);
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
 
    
		}    
		mysql_close(conn); //close the database connection
   	    mysql_library_end(); //stop using the library
	}
}   
   
   

   
/*
ReadConfig function, Goes through the 2sprout.conf file and reads in all the appropriate information 
used to create a connection with a database.
*/



int readConfig(string path)
{
	string line;
	ifstream sproutConfig(path.c_str());
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
						if(firstSub == "upnp" && secoundSub != "")
						{
							useUPNP = secoundSub;
							numOfArgsFound++;
						}
				}	
			}	
		}
		
		if(numOfArgsFound != 10)
		{
			printf("Configuration File is not Formatted Correctly...Exiting\n");
			exit(0);
			
		}
	
	}
	else
	{
		printf("Cannot Open Configuration File\n");
		exit(0);
	}
}






 
/*
GetFeed uses Sys V Message Queues in order to transfer data from the Client into the User made application via
the API.
*/
  
 void* getFeed(void *thread_arg)
{

	int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    message_buf sbuf;
    size_t buf_length;

    /*
     * Get the message queue id for the
     * "name" 1234, which was created by
     * the server.
     */
    key = 1234;

	(void) fprintf(stderr, "\nmsgget: Calling msgget(%#lx,%#o)\n",key, msgflg);

    if ((msqid = msgget(key, msgflg )) < 0) 
	{
        perror("msgget");
        exit(1);
    }
    else 
     (void) fprintf(stderr,"msgget: msgget succeeded: msqid = %d\n", msqid);



	while(1)
	{
		if(!sproutFeed.empty())
		{
			string s;
			s.clear();
			s = sproutFeed.front();
		
		  	sbuf.mtype = 1;

		    (void) fprintf(stderr,"msgget: msgget succeeded: msqid = %d\n", msqid);

		    (void) strcpy(sbuf.mtext, s.c_str());

		    (void) fprintf(stderr,"msgget: msgget succeeded: msqid = %d\n", msqid);

		    buf_length = strlen(sbuf.mtext) + 1;


		    /*
		     * Send a message.
		     */
	
		    	if (msgsnd(msqid, &sbuf, buf_length, false) < 0) 
				{
		       		printf ("%d, %d, %s, %d\n", msqid, sbuf.mtype, sbuf.mtext, buf_length);
		        	perror("msgsnd");
		        	exit(1);
		    	}

		    	else
				{ 
		      		printf("Message: \"%s\" Sent\n", sbuf.mtext);
			  		sproutFeed.pop();		
				}
			
		}
		else
		{
			if(usleep(1000) == -1)
			{
				perror("Sleep failed\n");
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
	string word;
	char bufpipe[4];
	
	
	ret_val = mkfifo(sproutPipe, 0777); //make the sprout pipe
	
	if (( ret_val == -1) && (errno != EEXIST)) 
	{
		perror("Error creating named pipe");
		exit(1);
	}
	
	
	
	while(1)
	{
		fd = open(sproutPipe, O_RDONLY); //open the pipe for reading
		
		numread = read(fd,bufpipe, 4);
		
		if(numread > 1)
		{
			bufpipe[numread] = '\0';

			string temp = bufpipe;
			
			memset(bufpipe,'\0',4);
			int pos = temp.find("^");
			if(pos != string::npos)
			{
				temp = temp.substr(0, pos);
			}
					
			int sizeOfString = atoi(temp.c_str());
			
			char feedWord[sizeOfString];
			int numRead1 = read(fd, feedWord, sizeOfString);
			
			if(numRead1 > 1)
			{					
				feedWord[sizeOfString] = '\0';
				word = feedWord;
			}

			close(fd);
			
			
			/*
			Find the number of spaces in the command to figure out how large to set the command[] buffer
			*/
			int NumSpacesCount = 0;
			int loop;
			for(loop =0; loop < word.length(); loop++)
			{
				string::size_type loc = word.find(' ', loop);
				if(loc != string::npos)
				{
					NumSpacesCount +=1;
				}	
			}		
			string token;
			if(NumSpacesCount == 0)
			{
				NumSpacesCount = 1;
			}
			string parsWord = word;
			string command[NumSpacesCount];	//messages passed through can have a maximum of 10 arguments. (should never be more then that)	
			istringstream iss(parsWord);
			int count1 = 0;
			
			/*
			This will tokenize the string and figure out what commands and arguments have been passed through from the API
			It will then call the specified functions with their arguments
			*/
			
			while(getline(iss,token,' '))
			{
				command[count1] = token;
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
			}
		}
	}
}
 

 

void catch_int(int sig_num)
{
    /* re-set the signal handler again to catch_int, for next time */
    signal(SIGINT, catch_int);
	unlink(sproutPipe);
	printf("Files deleted");
    fflush(stdout);
	exit(0);
}



static void forwardPort(struct UPNPUrls * urls,struct IGDdatas * data, const char * iaddr,const char * iport,const char * eport,const char * proto)
{
 
int r = UPNP_AddPortMapping(urls->controlURL, data->servicetype,
	                        eport, iport, iaddr, 0, proto, 0);
if(r!=UPNPCOMMAND_SUCCESS)
{
	//printf("AddPortMapping(%s, %s, %s) failed with code %d\n",eport, iport, iaddr, r);
	switch(r)
	{
		case 402:
			printf("Invalid arguments\n");
			break;
		case 501:
			printf("Action Failed\n");
			break;
		case 715:
			printf("Wildcard Not Permitted In Source IP\n");
			break;
		case 716:
			printf("Wildcard Not Permitted In External Port\n");
			break;
		case 718:
			printf("Mapping Assigned to Another User\n");
			break;
		case 724:
			printf("Internal And External Port Values Must Be The Same\n");
			break;
		case 725:
			printf("NAT Implementation Only Supports Permanent least Times On Port Mappings\n");
			break;
		case 726:
			printf("Remote Host Must Be A Wildcard And Cannot Be A Specific IP Address Or DNS Name\n");
			break;
		case 727:
			printf("External Port Must Be A Wildcard And Connot Be A Specific Port Value\n");
			break;
			
		default:
			printf("Unknown UPNP Error Ocurred\n");
			break;
		
		
	}
	printf("Unable To Set Port Forwarding. Please Check Configuration File, Or Turn Off Support For UPNP\n");
	exit(1);
}
	
else
	printf("Port added successfully\n");

}


void setUPNP(char* port)
{
	struct UPNPDev *devlist;
	char lanaddr[16];
	int i;
	const char * rootdescurl = 0;
	const char * multicastif = 0;
	const char * minissdpdpath = 0; 

	if( rootdescurl
	  || (devlist = upnpDiscover(2000, multicastif, minissdpdpath, 0)))
	{
		struct UPNPDev * device;
		struct UPNPUrls urls;
		struct IGDdatas data;
		i = 1;
		if( (rootdescurl && UPNP_GetIGDFromUrl(rootdescurl, &urls, &data, lanaddr, sizeof(lanaddr)))
		  || (i = UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr))))
		{
			switch(i) 
			{
			case 1:
				//printf("Found valid IGD : %s\n", urls.controlURL);
				break;
			case 2:
				printf("Internet Gateway Not Connected : %s\n", urls.controlURL);
				printf("continuing...\n");
				break;
			case 3:
				printf("UPnP device found. Checking for Internet Gateway : %s\n", urls.controlURL);
				printf("continuing...\n");
				break;
			default:
				printf("Found A Device : %s\n", urls.controlURL);
				printf("continuing...\n");
			}
			//printf("Local LAN ip address : %s\n", lanaddr);
			
			forwardPort(&urls, &data, lanaddr,port,port,"UDP");

			FreeUPNPUrls(&urls);
		}
		else
		{
			fprintf(stderr, "No valid UPNP gateway found\n");
		}
		freeUPNPDevlist(devlist); devlist = 0;
	}
	else
	{
		fprintf(stderr, "Unable To Set Port Forwarding. Please Check Configuration File, Or Turn Off Support For UPNP\n");
		exit(1);
	}


}





bool registerClient()
{
		CURL *curl;
		CURLcode res;
		char Portbuffer[10];
		sprintf(Portbuffer, "%i", MYPORT);
	
		string url = "http://2sprout.com/r/8573/Frer8n4drE/";
		//url += Portbuffer;
		cout << url << endl;
		
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
		if (buffer == "0000")
		{
			cout << "Client Already in the database" << endl;
		}
		else
		{
			cout << "Client Successfully Registerd" << endl;
		}
	
		string decoded = base64_decode(buffer);
	  	cout << "Decoded: " <<  decoded << endl;
	  	//XOR with the secret cypher
		string value(decoded);
		string key("2#sPr0uT5!");
		value = XOR(decoded,key);
		cout << "Decrypted: " << value << endl;


		//find the number of "^"
		int NumSpacesCount = 0;
		int loop;
		for(loop =0; loop < value.length(); loop++)
		{
			string::size_type loc = value.find('^', loop);
			if(loc != string::npos)
			{
				NumSpacesCount +=1;
			}	
		}		
		if(NumSpacesCount == 0)
		{
			NumSpacesCount = 1;
		}


		string token;
		string section[NumSpacesCount];	
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
			cipher = section[0];
			updatedPassword = section[1];
		
			cout << "CIPHER: " << cipher << endl;
			cout <<"PASS: " << updatedPassword << endl;
			sleeptime = atoi(section[2].c_str());
		}
}

/*
showHelp() Prints out the program usage.
*/
void showHelp()
{
	printf("Usage: 2sproutClient [-p port_number] [-c configuration_path]\nOptional: [-h help][-v version]\n");
}

void showVersion()
{
	printf("Client Version: %s\n", version);
}


/*
2sproutClient takes in two arguments in the following form [-p port_number] [-c configuration_path]
*/
int main(int argc, char *argv[])
{
	signal(SIGINT, catch_int); //redirect the signal so that when you press ctrl+c it deletes the named pipes
	
	string path = "2sprout.conf";


	/*
	This will loop through the array of command line arguments searching for the preFix which is the -* and the 
	postFix which is anything after that preFix. It sets the arguments appropriatly.
	*/

	int x;
	for(x = 1; x < argc; x++)
	{
		string input = argv[x];
		
		if(strlen(input.c_str()) >= 2)
		{
			string preFix = input.substr(0,2);
			string postFix = input.substr(2, strlen(input.c_str()));
		
			if(preFix != "-p" && preFix != "-c" && preFix != "-h" && preFix != "-v" && preFix != "-d")
			{
				printf("Unknown Option: %s\n", input.c_str());
				showHelp();
				exit(1);
			}
		
			if(preFix == "-p") //port number
			{
				if(postFix.length() < 4 || atoi(postFix.c_str()) <= 1024)
				{
					printf("Port Number is system reserved: Must be greater then 1024\n");
					showHelp();
					exit(1);	
				}
				else
				{
					MYPORT = atoi(postFix.c_str());
					cout << postFix.c_str() << endl;
					
					preFix.clear();
					postFix.clear();
				}
			}		
			if(preFix == "-c") //configuration path
			{
				if(postFix.length() >=1)
				{
					path = postFix;
					postFix.clear();
					preFix.clear();
				}
				else
				{
					printf("Path to configuration file is not set Properly");
					showHelp();
					exit(1);
				}
			}
			if(preFix == "-h") //show the help file
			{
				showHelp();
				exit(1);
			}
			if(preFix == "-v")
			{
				showVersion();
				exit(1);
			}
			if(preFix == "-d") //this is used for testing the database connection
			{
				readConfig(path);
				bool ableToConnect = testConnection(database, host,port,dbname,user,pass);
				if(ableToConnect == true)
				{
					cout <<"Database Connection Successful." << endl;
					exit(1);
				}
				else
				{
					cout << "Unable to connect to database. Please check configuration" << endl;
					exit(1);
				}
				
				
			}
			
		}
		else
		{	
			showHelp();
			exit(1);
		}
	}
	
	
	/*
	Sets the default port number to 4950 if no port number has been given
	*/
	if(MYPORT == 0)
	{
		MYPORT = 4950;
	}
	

	
	readConfig(path); 	//read the configuration file for database access.
	if(useUPNP == "true")
	{
		char portBuffer[20];
		sprintf(portBuffer, "%i", MYPORT);
		setUPNP(portBuffer);
		memset(portBuffer, '\0', sizeof(portBuffer));
    
	}
	
	
	registerClient();
	if(useDatabase == true)
    {	
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


   
   


