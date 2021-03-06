
/*
THIS IS AN UNPUBLISHED WORK CONTAINING 2SPROUT INC CONFIDENTIAL AND PROPRIETARY INFORMATION. 
DISCLOSURE, USE, OR REPRODUCTION WITHOUT AUTHORIZATION OF 2SPROUT INC IS STRICTLY PROHIBITED.
*/
#include "SproutClient.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>


/*
Used for created a message for SYS V Message Queues, which is used for passing data from the client into the 
API for passing into a user made application
*/
typedef struct msgbuf1 {
         long    mtype;
         char    mtext[MSGSZ];
         int 	 fullMsg;
         } message_buf1;


string deObsfucate(string encoded)
{
	char what[9];
	int code = 45485867;
	sprintf(what,"%i", code);
	
	encoded = base64_decode(encoded);
	string unencrypted = XOR(encoded, what);
	return unencrypted;
}


/*
Announce uses CURL in order to contact the 2sprout server and requests an updated cipher,password,update triple
*/
void* announce(void *thread_arg)
{	
	
	char Portbuffer[5];
	sprintf(Portbuffer, "%i", MYPORT);
	string port = Portbuffer;
	//unencryped url: http://2sprout.com/client/keepalive/
	string unencryptedURL = deObsfucate("XEFASA8XGQVHRUZXQEwYVFtYG1tZUVNZQBpfXVBIV1tdQ1EX");
	
	string url = unencryptedURL + port + "/" + apiKey + "/";
	
	port.clear();
	bzero(Portbuffer, sizeof(Portbuffer));
	string html = "";
	
	while(1)
	{
		html.clear();
		sleep(sleeptime);	
		html = getHtml(url, "");
		//cout << html << endl;

		//Convert the buffer from base64
 		string decoded = base64_decode(html);
 		//XOR with the secret cypher
		string value(decoded);
		string key(cipher);
		value = XOR(decoded,key);
		//cout << value << endl;
		
		//parse the buffer Password^sleepTime
		//find the number of "^"
		int NumSpacesCount = 0;
		unsigned int loop;
		for(loop =0; loop < value.length(); loop++)
		{
			if(value[loop] == '^')
			{
				NumSpacesCount++;
			}
		}		
		if(NumSpacesCount != 2)
		{
			printf("Unable to retrieve update\n");
			sleeptime = 2;
		}
		else
		{		
			string token;
			string section[3];	
			istringstream iss(value);
			int count1 = 0;
			while(getline(iss,token,'^'))
			{
				section[count1] = token;
				count1++;
			}
			if(section[0] != "" && section[1] != "" && section[2] != "")
			{
				pthread_mutex_lock(&mylock);
				oldCipher.clear();
				oldCipher = cipher; //swap the new cipher with the old cipher
				cipher.clear();
				cipher = section[0];//set the new cipher
				
				oldPassword.clear();
				oldPassword = updatedPassword; //swap the passwords
				updatedPassword.clear();
				updatedPassword = section[1];
				pthread_mutex_unlock(&mylock);
				
				sleeptime = atoi(section[2].c_str());
				switch(sleeptime)
				{
					case 0:
						sleeptime = 30;
						break;
					case INT_MAX:
						sleeptime= 30;
						break;
					case INT_MIN:
						sleeptime = 30;
						break;
					default:
						break;
				}
			}
		}
	}
}

/*
This will allow users using the API to stop recieving packets. It relays a POST to the server containing its port number
it will then delete the proper value from the database based on the ip address it gathers and its port number
*/
int closeAnnounce()
{
	char Portbuffer[10];
	sprintf(Portbuffer, "%i", MYPORT);
	string port = Portbuffer;
	//unencryped url:  "http://2sprout.com/disconnect/"
	string UnencrptedUrl = deObsfucate("XEFASA8XGQVHRUZXQEwYVFtYG1xcS1VYWltRW0EX");
	
	string url = UnencrptedUrl + port + "/" + apiKey +"/";
	//string post = "ID=" + apiKey + "&port=" + port;
	port.clear();
	bzero(Portbuffer, sizeof(Portbuffer));
	string html = getHtml(url, "");
	if(html == "")
		return 1;
	else
		return 0;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*
castListner binds to the port specified, and waits for packets to come in from the server
*/

void* castListener(void *thread_arg)
{ 	
	
	
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;
	struct sockaddr_storage their_addr;
	char rawPacket[5000];
	socklen_t addr_len;
	char Portbuffer[5];
	sprintf(Portbuffer, "%i", MYPORT);
	
	
	
	memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
   hints.ai_socktype = SOCK_DGRAM;
   hints.ai_flags = AI_PASSIVE; // use my IP

   if ((rv = getaddrinfo(NULL, Portbuffer, &hints, &servinfo)) != 0) 
	{
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
   }

   // loop through all the results and bind to the first we can
   for(p = servinfo; p != NULL; p = p->ai_next) 
	{
   	if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1)
 		{
      	perror("listener: socket");
         continue;
      }

      if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
      	close(sockfd);
         perror("listener: bind");
         continue;
      }

        break;
   }

   if (p == NULL)
	{
   	fprintf(stderr, "listener: failed to bind socket\n");
      exit(1);
   }
	
	cout << "Waiting for updates.." << endl;

	int passedKeys = 0;
	int failedKeys =0;
 	while(1)
 	{
    	addr_len = sizeof their_addr;   		
    	if ((numbytes = recvfrom(sockfd, (void *) rawPacket, sizeof(rawPacket) , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1)
    	{
        	perror("recvfrom\n");
        	exit(1);
   	}
	         
    	rawPacket[numbytes] = '\0';
		
		if(numbytes < 5000 && unprocessedData.size() < 50000)
		{
   	   string input = rawPacket;
			//cout << input << endl;
			bzero(rawPacket, sizeof(rawPacket));
			numbytes = 0;
			string decoded = base64_decode(input);
			pthread_mutex_lock(&mylock);
			string value(decoded);
			string key(cipher);
			value = XOR(decoded,key);			
			pthread_mutex_unlock(&mylock);
					
			if(value.size() > 10 && value.substr(0,10) == updatedPassword)
			{	
				//cout << "Updated Key Passed" << endl;
				pthread_mutex_lock(&mylock);
				unprocessedData.push(value.substr(11,value.length()));
				pthread_mutex_unlock(&mylock);
				//cout << value.substr(11,value.length()) << endl;
				
				passedKeys++;				
			}
			else
			{
				value.clear();
				key.clear();
				pthread_mutex_lock(&mylock);
				key = oldCipher;
				value = XOR(decoded, key);
				pthread_mutex_unlock(&mylock);
				if(value.size() > 10 && value.substr(0,10) == oldPassword)
				{
					//cout << "Old Password Passed " << oldPassword << endl;
					pthread_mutex_lock(&mylock);
					unprocessedData.push(value.substr(11,value.length()));
					pthread_mutex_unlock(&mylock);
					
					//cout << value.substr(11,value.length()) << endl;
					passedKeys++;
				}
				else
				{
					//cout << "KEY FAILED" << endl;
					failedKeys++;	
					//cout << value.substr(0,10) << " " << oldPassword << endl;
				}	
			}			
			input.clear();
			value.clear();
			decoded.clear();	
			
		}	
   }
	close(sockfd);
}




/*
checkPacketReliablity() will check the validity of each packet and make sure that it is actually part of a 
2sprout broadcast.
*/
void* checkPacketReliability(void *thread_arg)
{
	string token;
	string s = "";
	string section[4];
	
	while(1)
	{
		pthread_mutex_lock(&mylock);
		int sizeOfUnprocessedData = unprocessedData.size();
		pthread_mutex_unlock(&mylock);
		
		if(sizeOfUnprocessedData != 0)
		{
            s.clear();
            //start of critical section
			pthread_mutex_lock(&mylock);
		    s = unprocessedData.front();
			unprocessedData.pop();
			pthread_mutex_unlock(&mylock);
			
			//end of critial section
			
			//Tokenize the string
			istringstream iss(s);
			int count1 = 0;
				
			while(getline(iss,token,'^'))
			{
				if(count1 < 3)
				{
					section[count1] = token;
				}
				
				count1++;
			}
		
			iss.clear();
			
			if(section[0]  != "" && section[1] != "" && section[2] != "") //make sure we have all the parts
			{	
				
				if(currentDate == "")
				{
					currentDate.clear();
					currentDate = section[0];
				}						
				if(section[0] != currentDate && nextDate == "")
				{
					nextDate.clear();
					nextDate = section[0];
				}	

				if(section[0] == currentDate)
				{		
					pthread_mutex_lock(&mylock);
					packetsRecieved.push_back(atoi(section[1].c_str()));
					pthread_mutex_unlock(&mylock);		
				}
			
				if(section[0] == nextDate)
				{
						
					pthread_mutex_lock(&mylock);
					packetsRecievedDay2.push_back(atoi(section[1].c_str()));
					pthread_mutex_unlock(&mylock);
					
				}	
				if(section[0] != currentDate && section[0] != nextDate && dateRecieved == false)
				{	
					pthread_mutex_lock(&mylock);
					if(!packetsMissed.empty())
					{
						packetsMissed.clear();
					}
					pthread_mutex_unlock(&mylock);
					pthread_mutex_lock(&mylock);
					if(!packetsRecieved.empty())
					{
						packetsRecieved.clear();
					}	
					pthread_mutex_unlock(&mylock);
					
					//printf("new date found, current date being overwritten\n");
					pthread_mutex_lock(&mylock);
					currentDate.clear();
					currentDate = section[0];
					packetsRecieved.push_back(atoi(section[1].c_str()));
					dateRecieved = true;	
					pthread_mutex_unlock(&mylock);
						
				}
				else if(section[0] != currentDate && section[0] != nextDate && dateRecieved == true)
				{
					pthread_mutex_lock(&mylock);
					
					if(!packetsMissedDay2.empty())
					{
						packetsMissedDay2.clear();
					}
					pthread_mutex_unlock(&mylock);
					pthread_mutex_lock(&mylock);
					if(!packetsRecievedDay2.empty())
					{
						packetsRecievedDay2.clear();
					}	
					pthread_mutex_unlock(&mylock);
					
					pthread_mutex_lock(&mylock);
					currentDate.clear();
					currentDate = section[0];
					packetsRecieved.push_back(atoi(section[1].c_str()));
					dateRecieved = false;
					pthread_mutex_unlock(&mylock);							
				}
										
				//if this passes add the packet number to the array of recieved packet numbers
				//Add only the message to the sproutQueue
				
				pthread_mutex_lock(&mylock);
		 		sproutFeed.push(section[2]);
				pthread_mutex_unlock(&mylock);
			
			}
		}
		else
		{
			if(usleep(1000) == -1)
			{
				printf("sleep failed\n");
			}
		}		
	}		
}



void *runLostPackets(void *thread_arg)
{
    checkLostPackets(1);
}

void *runLostPacketsDay2(void *thread_arg)
{
    checkLostPackets(2);
}

void *getLostPackets(void *thread_arg)
{
    replaceLostPackets(1);
}

void *getLostPacketsDay2(void *thread_arg)
{
    replaceLostPackets(2);
}


void checkLostPackets(int day)
{
	vector<int> tempVector;
	while(1)
	{
		sleep(2);
		pthread_mutex_lock(&mylock);
        int sizeOfRecieved = (day == 1) ? (int) packetsRecieved.size() : packetsRecievedDay2.size();
		pthread_mutex_unlock(&mylock);
		
		if(sizeOfRecieved > 1)
		{
			//start of critical section
			pthread_mutex_lock(&mylock);
			tempVector = packetsRecieved;
            if(day==1)
            {
            	packetsRecieved.clear();
            }
            else
            {
            	packetsRecievedDay2.clear();
            }
			//end critial section
			pthread_mutex_unlock(&mylock);
			
			//sort the tempVector
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
            if(day == 1)
            {
            	packetsRecieved.push_back(lastPacket);
            }
            else
            {
            	packetsRecievedDay2.push_back(lastPacket);
            }
            pthread_mutex_unlock(&mylock);

			//Check the recieved packets against what was missing
			//Figure out which are the missing packets
			int sizeOfNewPackets = (int) tempVector.size();
			int i;
			int remainder;
			for (i = 0; i < sizeOfNewPackets-1; i++)
			{
				remainder = tempVector[i+1] - tempVector[i];
				if(( remainder != 1)) //if they are not sequential
				{					
					int j;
					for(j = 1; j < remainder; j ++)
					{
						//add these values to the missing packet vector
						pthread_mutex_lock(&mylock);
                        if(day == 1)
                        {
                        	packetsMissed.push_back(tempVector[i] + j);
                        }
                        else
                        {
                        	packetsMissedDay2.push_back(tempVector[i] + j);
                        }
                        pthread_mutex_unlock(&mylock);
					}
				}
			}
			tempVector.clear(); //empty this vector
		}
	}
}



void replaceLostPackets(int day)
{		
	int maxReplace = 10;
	while(1)
	{
		sleep(10);
		pthread_mutex_lock(&mylock);
        int sizeOfMissed = (day == 1) ? (int) packetsMissed.size() : packetsMissedDay2.size();
        pthread_mutex_unlock(&mylock);

        if(sizeOfMissed > 0)
		{
			//Make a local copy of the vecotr
            pthread_mutex_lock(&mylock);
            vector<int> packets = (day == 1) ? packetsMissed : packetsMissedDay2;
            int numLostPackets = (int)packets.size();
            if(day == 1)
            {
            	packetsMissed.erase(packetsMissed.begin(), packetsMissed.begin()+numLostPackets); //clear out the vector
            }
            else
            {
            	packetsMissedDay2.erase(packetsMissedDay2.begin(), packetsMissedDay2.begin()+numLostPackets); //clear out the vector
            }
            pthread_mutex_unlock(&mylock);
			
			 string dateEncoded = base64_encode((const unsigned char*) currentDate.c_str(), strlen(currentDate.c_str()));

			//encrypted url: http://www.2sprout.com/missed/
			string url = deObsfucate("XEFASA8XGUBDQhoKRkhEWEFBGltaVRlaXUZHXVEX");

			string post = "date=" + dateEncoded + "&missed=";
			string packetsToReplace;

			int x;
			if (numLostPackets < maxReplace)
			{
				for(x = 0; x < numLostPackets; x++)
				{	
				  char convertBuf[128];
				  snprintf(convertBuf, sizeof(convertBuf), "%d", packets[x]);

				  packetsToReplace.append(convertBuf);
				  bzero(convertBuf, sizeof(convertBuf));
				  if(x != (numLostPackets - 1))
				  {
			       		packetsToReplace.append("^");	
				  }
				}					
					
				//Call the url to get the missed packets
            cout << packetsToReplace << endl;
				string packetsEncoded = base64_encode((const unsigned char*) packetsToReplace.c_str(), strlen(packetsToReplace.c_str()));
				
				post += packetsEncoded;
								
				string html = getHtml(url, post);
				int htmlLength = html.size();
				while(htmlLength < 15 || html.substr(0,15) != "Missed Packets:")
				{
					cout << "Failed getting packets" << endl;
					html.clear();
					html = getHtml(url, post);
					if(usleep(5000) == -1)
					{
						cout << "Sleeping error" << endl;
					}
				}
				
				//Check to make sure the server didn't error out				
				if(html.substr(0,15) == "Missed Packets:")
				{
				
					html = html.substr(16);
					//Tokenize the string based on newlines, since they can't
					//exist cause the json will bark
					string token;
					istringstream iss(html);
					while(getline(iss,token,'\n'))
					{
						//push the token
						cout << token << endl;
						sproutFeed.push(token);
					}
				}
			}
			else
			{
				int maxLost = 0;					
				//encrypted url: http://www.2sprout.com/missed/
				string url = deObsfucate("XEFASA8XGUBDQhoKRkhEWEFBGltaVRlaXUZHXVEX");    
				string post = "date=" + dateEncoded + "&missed=";
				string packetsToReplace;
				for(x = 0; x < numLostPackets; x++)
				{
					if(maxLost == maxReplace)
					{
						maxLost = 0;
						
						string packetsEncoded = base64_encode((const unsigned char*) packetsToReplace.c_str(), strlen(packetsToReplace.c_str()));
                  packetsToReplace.clear();
                  post += packetsEncoded;
                  string html = getHtml(url, post);
						string token;
						istringstream iss(html);
						while(getline(iss,token,'\n'))
						{
							//push the token
							sproutFeed.push(token);
						}
					//encrypted url: http://www.2sprout.com/missed/
					url = deObsfucate("XEFASA8XGUBDQhoKRkhEWEFBGltaVRlaXUZHXVEX");
					post ="date=" + dateEncoded + "&missed=";						
					}
					else
					{
					  char convertBuf[128];
					  snprintf(convertBuf, sizeof(convertBuf), "%d", packets[x]);
					  
					  packetsToReplace.append(convertBuf);
					  bzero(convertBuf, sizeof(convertBuf));
                 if(maxLost != maxReplace-1)
					  {
					      packetsToReplace.append("^");
					  }
					  maxLost++;                    
					}	
				}
				/*
				Call the server for the final missing packets < 10
				*/
					cout << "PACKETS: ***********" << packetsToReplace << endl;
					packetsToReplace = packetsToReplace.substr(0, packetsToReplace.size()-1);
					cout << "PACKETS: ***********" << packetsToReplace << endl;
					
					string packetsEncoded = base64_encode((const unsigned char*) packetsToReplace.c_str(), strlen(packetsToReplace.c_str()));
               packetsToReplace.clear();
               post += packetsEncoded;
               cout << "POST " << post << endl;
               string html = getHtml(url, post);
               cout << "recieved: " << html << endl;
					string token;
					istringstream iss(html);
					while(getline(iss,token,'\n'))
					{
						//push the token
						sproutFeed.push(token);
					}
					//encrypted url: http://www.2sprout.com/missed/
					url = deObsfucate("XEFASA8XGUBDQhoKRkhEWEFBGltaVRlaXUZHXVEX");
					
					post = "date=" + dateEncoded + "&missed=";				
			}
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
		const char *paramValues[3];
		connectionString = "host=" + host + " port=" + port + " dbname=" + dbname + " user=" + user + " password=" + pass;
		PGconn *Conn = PQconnectdb(connectionString.c_str());
		PGresult* result;
		string Query;
		string s = "";
		
		if (PQstatus(Conn) == CONNECTION_BAD)
    	{
			logFile("Unable to connect to database (Postgresql)", "ERROR");
			
        	fprintf(stderr,"Failed to connect to database\n");
        	fprintf(stderr,"%s\n",PQerrorMessage(Conn));
        	PQfinish(Conn);
        	exit(1);
    	}

 		while(1)
 		{
			pthread_mutex_lock(&mylock);
			int sizeOfSproutFeed = sproutFeed.size();
			pthread_mutex_unlock(&mylock);
			
			
 			if(sizeOfSproutFeed != 0)
	 		{
				s.clear();
				pthread_mutex_lock(&mylock);
			    s = sproutFeed.front();
				cout << "SproutFeed " << s << endl;
				sproutFeed.pop();
				pthread_mutex_unlock(&mylock);
				
				paramValues[0] = s.c_str();
				Query = "INSERT INTO ";
	  			Query = Query + "\""+ table + "\"" + " (" + col + ") " + "VALUES(";	
	  			Query = Query + "$1";
	  			Query = Query +");";
	  			cout << Query << endl;

				result = PQexecParams(Conn, Query.c_str(), 1, NULL, paramValues,NULL, NULL, 0);
				
				if (PQresultStatus(result) != PGRES_COMMAND_OK) 
				{	
					string error = PQerrorMessage(Conn);
				   fprintf(stderr, "INSERT failed: %s", PQerrorMessage(Conn));
					logFile("Insert Failed (Postgresql) " + error, "ERROR");
					
				}	
				Query.clear();
				PQclear(result);
			}
			
			else
			{
				if(usleep(1000) == -1)
				{
					printf("sleep failed\n");
					logFile("Unable to sleep", "ERROR");
					
				}
			}
		}
		PQfinish(Conn);
		
	}

	if(database == "mysql")
	{
	 	MYSQL *conn;
		
    	if (mysql_library_init(0,NULL,NULL))
		{
			logFile("Unable to initialize Mysql Library", "ERROR");
			
			cout << "Library init failed" << endl;
			exit(1);
		}
		
		conn = mysql_init (NULL);
	
    	if(conn == NULL)
    	{
			logFile("Mysql Initiation failed", "ERROR");
    		cout << "Mysql initiation failed" << endl;
    		exit(1);
    	}
 
    	if(mysql_real_connect(conn, (char *)host.c_str(), (char *)user.c_str(), (char *)pass.c_str(), (char *)dbname.c_str(), atoi(port.c_str()), NULL,0) == NULL)
    	{
    		cout << "connection to server failed" << endl;
			logFile("Unable to connect to server (Mysql)", "ERROR");

    		mysql_close(conn);
    		exit(1);
    	}
    
    	while(1)
 		{
			pthread_mutex_lock(&mylock);
			int sizeOfSproutFeed = sproutFeed.size();
			pthread_mutex_unlock(&mylock);
			
 			if(sizeOfSproutFeed != 0)
			{
				pthread_mutex_lock(&mylock);
				string s = sproutFeed.front();
				sproutFeed.pop();
          		
				pthread_mutex_unlock(&mylock);
				
				printf("read in: %s\n", s.c_str());
		 		string escapedString;    
    			string mysqlQuery;
		   
				
		  		mysql_real_escape_string (conn, (char *)escapedString.c_str(), (char *)s.c_str(), strlen(s.c_str()));	//use the built in mysql function to put in escape characters...if ther are any	
				s.clear();
				mysqlQuery = "INSERT INTO " + table + " ("+ col +") VALUES (\"" + escapedString.c_str() + "\");"; //actual creation of the sql statment
 				if(mysql_real_query(conn, mysqlQuery.c_str(), (unsigned int)strlen(mysqlQuery.c_str())) != 0)
 		  		{
					mysqlQuery.clear();
					logFile("INSERT Failed (Mysql)", "ERROR");
 		   	   cout << "error query failed" << endl;
          	}
          		
				mysqlQuery.clear();
			}
			else
			{
				if(usleep(1000) == -1)
				{
					printf("sleeping Error\n");
				}
			}
		}    
		mysql_close(conn); //close the database connection
   	    mysql_library_end(); //stop using the library
	}
	
	return NULL;
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
		cout << "Reading Configuration File" << endl;
		while(!sproutConfig.eof())
		{
			getline(sproutConfig,line);
			if(!line.empty())
			{
				trim(line);
				
				//find the first occurance of an '=' sign
				found = line.find("=");
				if(found!=string::npos)
				{
					foundPos = (int)found;
					firstSub = line.substr(0,found);
					secoundSub = line.substr(foundPos+1);
					trim(firstSub);
					trim(secoundSub);
					
					if(firstSub == "usedb" && secoundSub == "false")
					{
						useDatabase = false;	
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
					if(firstSub == "apiKey" && secoundSub != "")
					{
						apiKey = secoundSub;	
						unsigned int i;
						for (i = 0; i < apiKey.length(); i++)
						{
							if(isalnum(apiKey[i]) == 0 && apiKey[i] != '-')
							{
								cout << "API Key has invalid character, Check configuration file." << endl;
								exit(0);	
							}
								
						}	
						numOfArgsFound++;	
					}
				}	
			}	
		}
		
		if(useDatabase == false && apiKey != "")
		{
			return 0;
		}
		
		else if(numOfArgsFound != 11 )
		{
			cout << numOfArgsFound << endl;
			logFile("Configuration file not formatted correctly", "ERROR");		
			printf("Configuration File is not Formatted Correctly...Exiting\n");
			exit(0);	
		}
	}
	else
	{
		logFile("Unable to open configuration file", "ERROR");
		
		printf("Cannot Open Configuration File\n");
		exit(0);
	}
	
	return NULL;
}






 
/*
GetFeed uses Sys V Message Queues in order to transfer data from the Client into the User made application via
the API.
*/
  
void* getFeed(void *thread_arg)
{

    int msgflg = IPC_CREAT | 0666;
    key_t key;
    message_buf1 sbuf;
    size_t buf_length;

    /*
     * Get the message queue id for the
     * "name" 1234, which was created by
     * the server.
     */
    key = 5121;

	//(void) fprintf(stderr, "\nmsgget: Calling msgget(%#lx,%#o)\n",key, msgflg);

    if ((msqid = msgget(key, msgflg )) < 0) 
	{
        perror("msgget");
        exit(1);
    }


	while(1)
	{
		pthread_mutex_lock(&mylock);
		if(!sproutFeed.empty())
		{
			string s;
			s.clear();
			s = sproutFeed.front();
			pthread_mutex_unlock(&mylock);
			
			if(s.size() > 1024)
			{
				
				while(s.size() > 1024)
				{
					string toSend = s.substr(0,1024);
					s = s.substr(1024, s.size());
					//cout << "s: " << s << endl;
		  			sbuf.mtype = 1;
					sbuf.fullMsg = 0;
					bzero(sbuf.mtext, sizeof(sbuf.mtext));
		    		(void) strncpy(sbuf.mtext, toSend.c_str(), strlen(toSend.c_str()));
		    		buf_length = strlen(sbuf.mtext) + 1;
	
		    		if (msgsnd(msqid, &sbuf, sizeof(sbuf), false) < 0) 
					{
		       			perror("msgsnd");
		    		}
	
				}
				if(s.size() <= 1024)
				{
					
		  			sbuf.mtype = 1;
					sbuf.fullMsg = 1;
					bzero(sbuf.mtext, sizeof(sbuf.mtext));
					char * stringToBuffer = (char *)s.c_str();
		    		strncpy(sbuf.mtext, stringToBuffer, strlen(stringToBuffer));
		    		buf_length = strlen(sbuf.mtext) + 1;
	
		    		if (msgsnd(msqid, &sbuf, sizeof(sbuf), false) < 0) 
					{
		       			perror("msgsnd");
		    		}	
					else
					{
						pthread_mutex_lock(&mylock);
						sproutFeed.pop();
						pthread_mutex_unlock(&mylock);
						
					}
				}
			}
			else
			{
				sbuf.mtype = 1;
				sbuf.fullMsg = 1;
				bzero(sbuf.mtext, sizeof(sbuf.mtext));
	    		(void) strncpy(sbuf.mtext, s.c_str(), strlen(s.c_str()));
	    		buf_length = strlen(sbuf.mtext) + 1;

	    		if (msgsnd(msqid, &sbuf, sizeof(sbuf), false) < 0) 
				{
						logFile("Unable to recieve message", "ERROR");
	       			perror("msgsnd");
	    		}	
				else
				{
					pthread_mutex_lock(&mylock);
					sproutFeed.pop();
					pthread_mutex_unlock(&mylock);
					
				}
				
			}
			
		}
		else
		{
			pthread_mutex_unlock(&mylock);
			
			if(usleep(1000) == -1)
			{
				perror("Sleep failed\n");
			}
		}
	}
}
  

 

void cleanup(int sig_num)
{
    /* re-set the signal handler again to catch_int, for next time */
	void registerSignals();
	logFile("Cleaning up", "INFO");
	printf("Cleaning Files\n");
   fflush(stdout);
	closeAnnounce();
	if(useDatabase == false)
	{
		if(msgctl(msqid, IPC_RMID, NULL) == 1) //Delete the Message Queue
		{
			logFile("Error closing message queue msgctl", "ERROR");
			perror("Error closing message queue msgctl");
		}
	}
	
	exit(0);
}







/*
registerClient: Notifies 2sprout that the client wants to recieve messages. On success it will be returned the 
cipher, secretkey and wait time until it should refresh the keys
*/

bool registerClient()
{
  	char Portbuffer[10];
	sprintf(Portbuffer, "%i", MYPORT);
	string port = Portbuffer;
	
	//Unencrypted URL: http://2sprout.com/client/connect/
	string unEncryptedUrl = deObsfucate("XEFASA8XGQVHRUZXQEwYVFtYG1tZUVNZQBpXV1tWU1RAGg==");
	string url = unEncryptedUrl + port + "/" + apiKey + "/";
	cout << url << endl;
	//string post = "ID=" + apiKey + "&port=" + port;
	
	port.clear();
	bzero(Portbuffer, sizeof(Portbuffer));
	string html = "";

	cout << "Contacting 2Sprout" << endl;
	html = getHtml(url, "");
	if(html == "API Key does not exist")
	{
		cout << "API Key does not exist. Please register at http://2sprout.com/signup/ for an API key" << endl;
		logFile("API Key does not exist. Please register at http://2sprout.com/signup/ for an API key", "ERROR");
		exit(0);
	}
	cout << "Client Registerd" << endl;
	string decoded = base64_decode(html);
  	//XOR with the secret cypher
	string value(decoded);
	
	//Unencrypted code: 2#sPr0uT5!
	string key = deObsfucate("BhZHaEcIQ2MBFA==");
	cout << key << endl;
	value = XOR(decoded,key);
	cout << value << endl;
	//find the number of "^"
	int NumSpacesCount = 0;
	unsigned int loop;
	
	for(loop =0; loop < value.length(); loop++)
	{
		if(value[loop] == '^')
		{
			NumSpacesCount++;
		}
	}		
		
	if(NumSpacesCount != 2) //The number of ^'s that we recieved was not correct, It should try to re-register.
	{
		sleep(2);
		registerClient();
	}
	else
	{
		
		string token;
		string section[3];	
		istringstream iss(value);
		int count1 = 0;
		while(getline(iss,token,'^'))
		{
			section[count1] = token;
			count1++;
		}

		if(section[0] != "" && section[1] != "" && section[2] != "")
		{
			cipher = section[0];
			updatedPassword = section[1];
			sleeptime = atoi(section[2].c_str());
			switch(sleeptime)
			{
				case 0:
					sleeptime = 30;
					break;
				case INT_MAX:
					sleeptime= 30;
					break;
				case INT_MIN:
					sleeptime = 30;
					break;
				default:
					break;
			}

			logFile("Client registerd", "INFO");
			
			cout << "Client Sucessfully registered" << endl;
			//cout << "cipher: " << cipher << endl;
			//cout << "updated Password  << updatedPassword << endl;
			//cout << sleeptime << endl;	       
		}
		else //just put in as a failsafe. It should never reach this point.
		{
			sleep(2);
			registerClient();
		}
	}
	
	return NULL;
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

void registerSignals()
{
	signal(SIGINT, cleanup); //redirect the signal so that when you press ctrl+c it deletes the named pipes
	signal(SIGTERM, cleanup);
	signal(SIGKILL, cleanup);
	signal(SIGUSR1, cleanup);
	signal(SIGUSR2, cleanup);	
	signal(SIGHUP, cleanup);
}


/*
2sproutClient takes in two arguments in the following form [-p port_number] [-c configuration_path]
*/
int main(int argc, char *argv[])
{
	
/*
		
		char what[9];
		int code = 45485867;
		sprintf(what,"%i", code);
		
		
		
		cout << what << endl;
		//string decoded = base64_decode(encoded);
		string unencrypted = XOR("http://2sprout.com/client/keepalive/", what);
      unencrypted = base64_encode(reinterpret_cast<const unsigned char*>(unencrypted.c_str()), unencrypted.length());
		cout << unencrypted << endl;
		string g = deObsfucate(unencrypted);
		cout << g << endl;
		exit(0);
*/	

	
	bool guiMode = false;
	
	
	registerSignals();	
	string path = "/usr/local/etc/2sprout.conf";
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
		
			if(preFix != "-p" && preFix != "-c" && preFix != "-h" && preFix != "-v" && preFix != "-d" && preFix != "-g")
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
					//cout << postFix.c_str() << endl;
					
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
			if(preFix == "-g")
			{
				guiMode = true;
			}
			
			if(preFix == "-d") //this is used for testing the database connection
			{
				readConfig(path);
				bool ableToConnect = testConnection(database, host,port,dbname,user,pass, table, col);
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
	logFile("Configuration SET", "INFO");
	cout << "Configuration Set" << endl;
	cout << "Starting 2Sprout daemon" << endl;
	
	
	if(useUPNP == "true")
	{
		char portBuffer[20];
		sprintf(portBuffer, "%i", MYPORT);
		setUPNP(portBuffer);
		memset(portBuffer, '\0', sizeof(portBuffer));
    
	}
	
	
	/* Our process ID and Session ID */


	if(guiMode == false)
	{
		pid_t pid, sid;

		pid = fork();
		if (pid < 0) 
		{
			logFile("Unable to fork", "ERROR");
			exit(EXIT_FAILURE);
		}

		if (pid > 0) 
		{
			exit(EXIT_SUCCESS);
		}

		umask(0);

		sid = setsid();
		if (sid < 0) 
		{
			logFile("Unable to get session id", "ERROR");
	   	exit(EXIT_FAILURE);
		}

		if ((chdir("/")) < 0) 
		{		
			logFile("Unable to change directory", "ERROR");
	
	   	exit(EXIT_FAILURE);
		}
	

		/* Close out the standard file descriptors */
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
	}
	
	if(useDatabase == true)
    {	
		//Test To make sure we can access the database

		
		cout << "Testing Database Configuration..." << endl;
		bool ableToConnect = testConnection(database, host,port,dbname,user,pass, table,col);
		if(ableToConnect == true)
		{
			cout <<"Database Connection Successful." << endl;
		}
		else
		{
			logFile("Unable to connect to database. Check configuration", "ERROR");	
			cout << "Unable to connect to database. Please check configuration" << endl;
			exit(1);
		}
		
		registerClient();
		
	
		int rc, i , status;
		pthread_t threads[8];
		cout << "Starting 2Sprout Client" << endl;	
		pthread_create(&threads[0], NULL, announce, NULL);
		pthread_create(&threads[1], NULL, castListener, NULL);
		pthread_create(&threads[2], NULL, insertToDb, NULL);		
		pthread_create(&threads[3], NULL, checkPacketReliability, NULL);	
      pthread_create(&threads[4], NULL, runLostPackets, NULL);
      pthread_create(&threads[5], NULL, runLostPacketsDay2, NULL);
      pthread_create(&threads[6], NULL, getLostPackets, NULL);
      pthread_create(&threads[7], NULL, getLostPacketsDay2, NULL);
	
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
		registerClient();
		int rc, i , status;
		pthread_t threads[8];	
		cout << "Starting 2Sprout Client" << endl;	
		pthread_create(&threads[0], NULL, announce, NULL);
		pthread_create(&threads[1], NULL, castListener, NULL);
		pthread_create(&threads[2], NULL, checkPacketReliability, NULL);	
      pthread_create(&threads[3], NULL, runLostPackets, NULL);
      pthread_create(&threads[4], NULL, runLostPacketsDay2, NULL);
      pthread_create(&threads[5], NULL, getLostPackets, NULL);
      pthread_create(&threads[6], NULL, getLostPacketsDay2, NULL);
		pthread_create(&threads[7], NULL, getFeed, NULL);
		
		for(i =0; i < 8; i++)
		{
			rc = pthread_join(threads[i], (void **) &status); 
		}
	}
}

   


