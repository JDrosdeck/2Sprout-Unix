
/*
THIS IS AN UNPUBLISHED WORK CONTAINING 2SPROUT INC CONFIDENTIAL AND PROPRIETARY INFORMATION. 
DISCLOSURE, USE, OR REPRODUCTION WITHOUT AUTHORIZATION OF 2SPROUT INC IS STRICTLY PROHIBITED.
*/
#include "SproutClient.h"




/*
Used for created a message for SYS V Message Queues, which is used for passing data from the client into the 
API for passing into a user made application
*/
typedef struct msgbuf1 {
         long    mtype;
         char    mtext[MSGSZ];
         } message_buf1;






/*
Announce uses CURL in order to contact the 2sprout server and requests an updated cipher,password,update triple
*/
void* announce(void *thread_arg)
{	
	
	char Portbuffer[10];
	sprintf(Portbuffer, "%i", MYPORT);
	string port = Portbuffer;
	
	string url = "http://2sprout.com/refresh/" + port + "/" + apiKey + "/";
	port.clear();
	bzero(Portbuffer, sizeof(Portbuffer));
	string html = "";
	
	while(1)
	{
		
		html.clear();
		sleep(sleeptime);	
		html = getHtml(url);

		//Convert the buffer from base64
 		string decoded = base64_decode(html);
 		//XOR with the secret cypher
		string value(decoded);
		string key(cipher);
		value = XOR(decoded,key);
		cout << value << endl;
		//parse the buffer Password^sleepTime
		//find the number of "^"
		int NumSpacesCount = 0;
		int loop;
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
				cipher = section[0];
				updatedPassword = section[1];
				#warning todo: Check The Return value of atoi
				sleeptime = atoi(section[2].c_str());	
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
	CURL *curl;
	CURLcode res;
	char Portbuffer[10];
	sprintf(Portbuffer, "%i", MYPORT);
	string port = Portbuffer;
	string url = "http://2sprout.com/disconnect/" + port + "/" + apiKey + "/";
	port.clear();
	bzero(Portbuffer, sizeof(Portbuffer));
	curl = curl_easy_init();
	if(curl)
	{
		curl_easy_setopt(curl,CURLOPT_URL, url.c_str());
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
	
	return 1;
}

/*
castListner binds to the port specified, and waits for packets to come in from the server
*/

void* castListener(void *thread_arg)
{ 	
	
	char *ipAdd;
	int sockfd;
	struct sockaddr_in my_addr;    // my address information
	struct sockaddr_in their_addr; // connector's address information
	socklen_t addr_len;
	int numbytes;
	char rawPacket[10000];
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
    {
        perror("Unable to set socket\n");
        exit(1);
    }

    my_addr.sin_family = AF_INET;         // host byte order
    my_addr.sin_port = htons(MYPORT);     // short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    memset(my_addr.sin_zero, '\0', sizeof my_addr.sin_zero);


    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof my_addr) == -1) 
    {
        perror("Unable to bind to socket\n");
        exit(1);
    }

	cout << "Waiting for updates.." << endl;

	int counter = 0;
 	while(1)
 	{
    	addr_len = sizeof their_addr;   		
    	if ((numbytes = recvfrom(sockfd, (void *) rawPacket, sizeof(rawPacket) , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1)
    	{
        	perror("recvfrom\n");
        	exit(1);
   		}
	
        ipAdd = inet_ntoa(their_addr.sin_addr);
         
    	buf[numbytes] = '\0';

		if(numbytes < 5000 && unprocessedData.size() < 50000)
		{
   	    	string input = rawPacket;
			bzero(rawPacket, sizeof(rawPacket));
			numbytes = 0;
			string decoded = base64_decode(input);
			string value(decoded);
			string key(cipher);
			
			value = XOR(decoded,key);
			if(value.substr(0,10) == updatedPassword)
			{
				unprocessedData.push(value.substr(10,value.length()));
				cout << "PUSHED" << endl;
				counter++;
			}
			else
			{
				counter++;
				value.clear();
				key.clear();
				key = oldCipher;
				value = XOR(decoded, key);
				if(value.substr(0,10) == oldPassword)
				{
					unprocessedData.push(value.substr(10,value.length()));
					cout << "Old Value Pushed" << endl;
				}
				
			}
			
			
			
			input.clear();
			value.clear();
			decoded.clear();	
			
		}
		cout << counter << endl;
		
   }
	close(sockfd);
}




/*
checkPacketReliablity() will check the validity of each packet and make sure that it is actually part of a 
2sprout broadcast.
*/
void* checkPacketReliability(void *thread_arg)
{

	while(1)
	{
		if(unprocessedData.empty())
 		{
		
 				if(usleep(1000) == -1)
				{
					printf("Sleeping Error");
				}
		}
		else
		{
		}
		
		if(!unprocessedData.empty()) //while the queue has items
		{

			//start of critical section
			string s = unprocessedData.front();
			unprocessedData.pop();
			
			//end of critial section
			
			//Tokenize the string
			
			string token;
			string section[50]; //a standard sproutcast should be made up of only 6 distince sections	
			istringstream iss(s);
			int count1 = 0;

			
			while(getline(iss,token,'^'))
			{
				section[count1] = token;
				count1++;
			}
		
			
			if((section[0]  != "") && section[1] != "" && section[2] != "" && section[3] != "" && section[4] != "" ) //make sure we have all the parts
			{	
				string CastMinusChecksum = "^" + section[1] + "^" + section[2] + "^" + section[3] + "^" + section[4]; //generate the origional string to grab the checksum sum from					
			
				
				
				//string checkMd5 = MD5String(CastMinusMD5); //get the value of the MD5 string
				//printf("MD5 SUM IS: %s", checkMd5.c_str());
			
				int CheckSum = calcCheckSum(CastMinusChecksum);
				
				cout << "Calculated CheckSum: " << CheckSum << " Recieved Checksum: " << section[0].c_str() << endl;
				
				if(CheckSum == atoi(section[0].c_str())) //The MD5 Sum is the same so data integrety is OK
				{
					CastMinusChecksum.clear();
					if(currentDate == "")
					{
						//printf("Current Day initially set\n");
						currentDate = section[1];
					}						
					if(section[1] != currentDate && nextDate == "")
					{
						//printf("next Date initially set\n");
						nextDate = section[1];
					}	

					if(section[1] == currentDate)
					{
						packetsRecieved.push_back(atoi(section[2].c_str()));
						//printf("NOT REPLACING PACKET pusing back packet number\n");				
					}
			
					if(section[1] == nextDate)
					{
						packetsRecievedDay2.push_back(atoi(section[2].c_str()));
						//printf("NOT REPLACING PACKET pusing back packet number\n");	
					}	
					if(section[1] != currentDate && section[1] != nextDate && dateRecieved == false)
					{
						if(!packetsMissed.empty())
						{
							packetsMissed.clear();
						}
						if(!packetsRecieved.empty())
						{
							packetsRecieved.clear();
						}	
						//printf("new date found, current date being overwritten\n");
						currentDate = section[1];
						packetsRecieved.push_back(atoi(section[2].c_str()));
						dateRecieved = true;	
					}
					else if(section[1] != currentDate && section[1] != nextDate && dateRecieved == true)
					{
						if(!packetsMissedDay2.empty())
						{
							packetsMissedDay2.clear();
						}
						if(!packetsRecievedDay2.empty())
						{
							packetsRecievedDay2.clear();
						}	
				//		printf("new date found next date being overwritten\n");	
						currentDate = section[1];
						packetsRecieved.push_back(atoi(section[2].c_str()));
						dateRecieved = false;							
					}
										
					//if this passes add the packet number to the array of recieved packet numbers
					//Add only the message to the sproutQueue
				
		 			sproutFeed.push(section[4]);
			
				}
				else
				{
					CastMinusChecksum.clear();
				}
			}
			else
			{
		
			}
		}
		else
		{
		}		
	}		
}




void* checkLostPacketsDay2(void *thread_arg)
{
	vector<int> tempVector;
	while(1)
	{
		sleep(2);
		int sizeOfRecieved = (int) packetsRecievedDay2.size();
		if(sizeOfRecieved > 1)
		{
			//start of critical section
			pthread_mutex_lock(&mylock);
			tempVector = packetsRecievedDay2;
			//end critial section
			packetsRecievedDay2.clear();		
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
			packetsRecievedDay2.push_back(lastPacket);
			pthread_mutex_unlock(&mylock);
			int sizeOfVector = (int) packetsMissedDay2.size();

			//Check the recieved packets against what was missing
			//Figure out which are the missing packets
			int sizeOfNewPackets = (int) tempVector.size();
			cout << sizeOfNewPackets << endl;
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
					 	packetsMissedDay2.push_back(tempVector[i] + j);
						pthread_mutex_unlock(&mylock);	
					}
				}
			}
			tempVector.clear(); //empty this vector
		}
	}
	
}


void* replaceLostPacketsDay2(void *thread_arg)
{
	while(1)
	{
		sleep(10);
		if(!packetsMissedDay2.empty())
		{
			
				printf("NOTIFYING SERVER OF MISSED PACKETS\n");
				int numLostPackets = (int)packetsMissedDay2.size();	
				string url = "http://www.2sprout.com/missing/";
			
				int x;
				stringstream out;
				if (numLostPackets < 10)
				{
					for(x = 0; x < numLostPackets; x++)
					{
					
						out.clear();
						out << packetsMissed[x];
						url.append(out.str());
						if(x != (numLostPackets - 1))
						{
							url.append("^");	
						}
					}
				
				}
				
				else
				{
					int maxLost = 0;
					
					for(x = 0; x < numLostPackets; x++)
					{
						if(maxLost == 10)
						{
							maxLost = 0;
							string html = getHtml(url);	
							#warning automatically tokenize and put data into proper queue 
						}
						else
						{
							out.clear();
							out << packetsMissedDay2[x];
							url.append(out.str());
							if(x != (numLostPackets -1))
							{
								url.append("^");
							}
							maxLost++;
						}	
					}				
				}
		}
		
		else
		{

		}
	}

}






/*
Number will come in looking like month/day/year/packetNumber
*/


void* checkLostPackets(void *thread_arg)
{
	vector<int> tempVector;
	while(1)
	{
		sleep(2);
		int sizeOfRecieved = (int) packetsRecieved.size();
		if(sizeOfRecieved > 1)
		{
			//start of critical section
			pthread_mutex_lock(&mylock);
			tempVector = packetsRecieved;
			//end critial section
			packetsRecieved.clear();		
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
			packetsRecieved.push_back(lastPacket);
			pthread_mutex_unlock(&mylock);
			int sizeOfVector = (int) packetsMissed.size();

			//Check the recieved packets against what was missing
			//Figure out which are the missing packets
			int sizeOfNewPackets = (int) tempVector.size();
			cout << sizeOfNewPackets << endl;
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
					 	packetsMissed.push_back(tempVector[i] + j);
						pthread_mutex_unlock(&mylock);	
					}
				}
			}
			tempVector.clear(); //empty this vector
		}
	}
}



void* replaceLostPackets(void *thread_arg)
{
		
	while(1)
	{
		sleep(10);
	
		if(!packetsMissed.empty())
		{
			
				printf("NOTIFYING SERVER OF MISSED PACKETS\n");
				int numLostPackets = (int)packetsMissed.size();	
				string url = "http://www.2sprout.com/missing/";
			
				int x;
				stringstream out;
				if (numLostPackets < 10)
				{
					for(x = 0; x < numLostPackets; x++)
					{
					
						out.clear();
						out << packetsMissed[x];
						url.append(out.str());
						if(x != (numLostPackets - 1))
						{
							url.append("^");	
						}
					}
				
				}
				
				else
				{
					int maxLost = 0;
					
					for(x = 0; x < numLostPackets; x++)
					{
						if(maxLost == 10)
						{
							maxLost = 0;
							string html = getHtml(url);	
							#warning automatically tokenize and put data into proper queue 
						}
						else
						{
							out.clear();
							out << packetsMissed[x];
							url.append(out.str());
							if(x != (numLostPackets -1))
							{
								url.append("^");
							}
							maxLost++;
						}	
					}				
				}
		}
		
		else
		{

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

			
			else //while the queue has items
			{
				string s = sproutFeed.front();
				sproutFeed.pop();
				
				string escapedString;
				int *error;
				
				char escapeBuffer[(s.length() * 2)+1];
				
		 		unsigned long g = PQescapeStringConn(Conn, escapeBuffer, (char *)s.c_str(), strlen(s.c_str()),error);  
				//cout << "Escaped String " << escapeBuffer << endl;
							
	  			// (Queries)
	  			string Query = "INSERT INTO ";
	  			Query = Query + "\""+ table + "\"" + " (" + col + ") " + "VALUES('";	
	  			Query = Query + escapeBuffer;
	  			Query = Query +"');";
	  			//cout << Query << endl;
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
		}
		PQfinish(Conn);
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
								
				
				//pthread_mutex_unlock(&mylock);
				//end of critical section
		 		string escapedString;    
    			string mysqlQuery;
		   
		  		unsigned long to_len = mysql_real_escape_string (conn, (char *)escapedString.c_str(), (char *)s.c_str(), strlen(s.c_str()));	//use the built in mysql function to put in escape characters...if ther are any	
				mysqlQuery = "INSERT INTO " + table + " ("+ col +") VALUES (\"" + escapedString.c_str() + "\");"; //actual creation of the sql statment
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
						int i;
						for (i = 0; i < apiKey.length(); i++)
						{
							if(isalnum(apiKey[i]) == 0)
							{
								cout << "API Key has invalid character, Check configuration file." << endl;
								exit(0);	
							}
								
						}	
						if(apiKey.length() != 10)
						{
							cout << "API Key is not a valid length, Check configuration file." << endl;
							exit(0);
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

    int msgflg = IPC_CREAT | 0666;
    key_t key;
    message_buf1 sbuf;
    size_t buf_length;

    /*
     * Get the message queue id for the
     * "name" 1234, which was created by
     * the server.
     */
    key = 1234;

	//(void) fprintf(stderr, "\nmsgget: Calling msgget(%#lx,%#o)\n",key, msgflg);

    if ((msqid = msgget(key, msgflg )) < 0) 
	{
        perror("msgget");
        exit(1);
    }


	while(1)
	{
		if(!sproutFeed.empty())
		{
			string s;
			s.clear();
			s = sproutFeed.front();
		
		  	sbuf.mtype = 1;


			bzero(sbuf.mtext, sizeof(sbuf.mtext));
		    (void) strncpy(sbuf.mtext, s.c_str(), strlen(s.c_str()));


		    buf_length = strlen(sbuf.mtext) + 1;


		    /*
		     * Send a message.
		     */
	
		    if (msgsnd(msqid, &sbuf, buf_length, false) < 0) 
			{
		       	perror("msgsnd");
		       	exit(1);
		    }
		    else
			{ 
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
 
 

 
 

 

void catch_int(int sig_num)
{
    /* re-set the signal handler again to catch_int, for next time */
	void registerSignals();
	unlink(sproutPipe);
	printf("Cleaning Files\n");
    fflush(stdout);
	closeAnnounce();
	if(useDatabase == false)
	{
		if(msgctl(msqid, IPC_RMID, NULL) == 1)
		{
			perror("Error closing message queue msgctl");
		}
	}
	
	exit(0);
}








bool registerClient()
{

  	char Portbuffer[10];
	sprintf(Portbuffer, "%i", MYPORT);
	string port = Portbuffer;
	string url = "http://2sprout.com/connect/" + port + "/" + apiKey + "/";
	port.clear();
	bzero(Portbuffer, sizeof(Portbuffer));
	string html = "";

	html = getHtml(url);
	
	
	string decoded = base64_decode(html);
  	//XOR with the secret cypher
	string value(decoded);
	string key("2#sPr0uT5!");
	value = XOR(decoded,key);
	//find the number of "^"
	int NumSpacesCount = 0;
	int loop;
	
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
		cout << "Client Sucessfully registered" << endl;	       
	}
	else //just put in as a failsafe. It should never reach this point.
	{
		sleep(2);
		registerClient();
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

void registerSignals()
{
	signal(SIGINT, catch_int); //redirect the signal so that when you press ctrl+c it deletes the named pipes
	signal(SIGTERM, catch_int);
	signal(SIGKILL, catch_int);
	signal(SIGUSR1, catch_int);
	signal(SIGUSR2, catch_int);	
	signal(SIGHUP, catch_int);
}

/*
2sproutClient takes in two arguments in the following form [-p port_number] [-c configuration_path]
*/
int main(int argc, char *argv[])
{
	registerSignals();
	
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
	
	
	if(useDatabase == true)
    {	
		//Test To make sure we can access the database

		cout << "Testing Database Configuration..." << endl;
		bool ableToConnect = testConnection(database, host,port,dbname,user,pass);
		if(ableToConnect == true)
		{
			cout <<"Database Connection Successful." << endl;
		}
		else
		{
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
		pthread_create(&threads[4], NULL, checkLostPackets, NULL);
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
		registerClient();
		int rc, i , status;
		pthread_t threads[8];	
		cout << "Starting 2Sprout Client" << endl;	
		pthread_create(&threads[0], NULL, announce, NULL);
		pthread_create(&threads[1], NULL, castListener, NULL);
		pthread_create(&threads[2], NULL, checkPacketReliability, NULL);	
		pthread_create(&threads[3], NULL, checkLostPackets, NULL);
		pthread_create(&threads[4], NULL, checkLostPacketsDay2, NULL);
		pthread_create(&threads[5], NULL, replaceLostPackets, NULL);
		pthread_create(&threads[6], NULL, replaceLostPacketsDay2, NULL);
		pthread_create(&threads[7], NULL, getFeed, NULL);
		
		for(i =0; i < 8; i++)
		{
			rc = pthread_join(threads[i], (void **) &status); 
		}
	}
}


   
   


