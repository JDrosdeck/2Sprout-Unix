
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
         bool 	 fullMsg;
         } message_buf1;


/*
Announce uses CURL in order to contact the 2sprout server and requests an updated cipher,password,update triple
*/
void* announce(void *thread_arg)
{	
	
	char Portbuffer[5];
	sprintf(Portbuffer, "%i", MYPORT);
	string port = Portbuffer;
	
	string url = "http://2sprout.com/refresh/";
	string post = "ID=" + apiKey + "&port=" + port;
	
	port.clear();
	bzero(Portbuffer, sizeof(Portbuffer));
	string html = "";
	
	while(1)
	{
		logFile("Announcing Client To Server");
		html.clear();
		sleep(sleeptime);	
		html = getHtml(url, post);
		cout << html << endl;

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
			logFile("Announce: Couldn't Recieve Update");
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
				
				#warning todo: Check The Return value of atoi
				sleeptime = atoi(section[2].c_str());	
			}
		}
	}
	
}

//was


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
	string url = "http://2sprout.com/disconnect/";
	string post = "ID=" + apiKey + "&port=" + port;
	port.clear();
	bzero(Portbuffer, sizeof(Portbuffer));
	string html = getHtml(url, post);
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

    if ((rv = getaddrinfo(NULL, Portbuffer, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        exit(1);
    }
	
	
	cout << "Waiting for updates.." << endl;

	int counter = 0;
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

		if(numbytes < 5000 && unprocessedData.size() < 50000 && counter != 5)
		{
   	    	string input = rawPacket;
			cout << "recieved: " << input << endl;
			bzero(rawPacket, sizeof(rawPacket));
			numbytes = 0;
			string decoded = base64_decode(input);
			pthread_mutex_lock(&mylock);
			
			
			string value(decoded);
			string key(cipher);
			
			value = XOR(decoded,key);
			
			pthread_mutex_unlock(&mylock);
						
			if(value.substr(0,10) == updatedPassword)
			{
				pthread_mutex_lock(&mylock);
				unprocessedData.push(value.substr(11,value.length()));
				pthread_mutex_unlock(&mylock);
				
				passedKeys++;
				logFile("CastListener: Secret Key Passed");
				cout << "Secret Key Passed " << passedKeys << endl;
				
				counter++;
			}
			else
			{
				
				counter++;
				value.clear();
				key.clear();
				pthread_mutex_lock(&mylock);
				key = oldCipher;
				value = XOR(decoded, key);
				pthread_mutex_unlock(&mylock);
				
				if(value.substr(0,10) == oldPassword)
				{
					unprocessedData.push(value.substr(11,value.length()));
					cout << "Old Value Pushed" << endl;
					passedKeys++;
					cout << "Secret Key Passed " << passedKeys << endl;
				}
				else
				{
					failedKeys++;
					cout << "Secret Key Failed " << failedKeys << endl;
					logFile("castListener: Secret Key Failed: " + failedKeys);
					
					cout << value.substr(0,10) << " " << oldPassword << endl;
				}	
			}
						
			input.clear();
			value.clear();
			decoded.clear();	
			cout << "Passes: " << passedKeys << endl;
			cout << "Fails: " << failedKeys << endl;
			
			
			
		}
		if (counter == 5)
			counter++;
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
	string token;
	string s = "";
	string section[4];
	
	while(1)
	{
		pthread_mutex_lock(&mylock);
		if(!unprocessedData.empty())
		{
                    s.clear();
                    //start of critical section
		    s = unprocessedData.front();
			cout << "PACKER RELIABILITY: " << s << endl;
			unprocessedData.pop();
			pthread_mutex_unlock(&mylock);
			
			//end of critial section
			
			//Tokenize the string
			istringstream iss(s);
			int count1 = 0;
				
			while(getline(iss,token,'^'))
			{
				if(count1 < 4)
				{
					section[count1] = token;
				}
				
				count1++;
			}
		
			iss.clear();
			
			if(section[0]  != "" && section[1] != "" && section[2] != "" && section[3] != "") //make sure we have all the parts
			{	
				
				if(BroadcastingServer == "") //this is the first update
				{
					logFile("Packet Reliability: Updating Broadcasting Server");
					cout << "Updating Broadcasting Server" << endl;
					BroadcastingServer = section[2];
				}
				if(BroadcastingServer != section[2]) //we switched broadcasters, So contract 2sprout and retrieve all the missed packets for the previous broadcaster
				{
					/*
					Contact the server and retrieve all lost updates
					*/
					
					BroadcastingServer = section[2];
				}
				
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
					logFile("packet reliability: New Date Found");
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
				logFile("packet Reliability: Adding Packet To Vector of recieved packet Numbers");
		 		sproutFeed.push(section[3]);
				pthread_mutex_unlock(&mylock);
			
			}
		}
		else
		{
			pthread_mutex_unlock(&mylock);
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
			logFile("Lost Packets: Calculating Lost Packets");
			//start of critical section
			pthread_mutex_lock(&mylock);
			tempVector = packetsRecieved;
			//end critial section
                        if(day==1)
                        {
                            packetsRecieved.clear();
                        }
                        else
                        {
                            packetsRecievedDay2.clear();
                        }
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
			cout << sizeOfNewPackets << endl;
			int i;
			int remainder;
			for (i = 0; i < sizeOfNewPackets-1; i++)
			{
				remainder = tempVector[i+1] - tempVector[i];
				if(( remainder != 1)) //if they are not sequential
				{
					logFile("Missing Packets: Found Missing Packet");
					
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
			logFile("Notifying Server Of Missed Packets");
			printf("**********NOTIFYING SERVER OF MISSED PACKETS***********\n");
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
			
			string broadcastServerEncoded = base64_encode((const unsigned char*)BroadcastingServer.c_str(), strlen(BroadcastingServer.c_str()));
			string dateEncoded = base64_encode((const unsigned char*) currentDate.c_str(), strlen(currentDate.c_str()));

			string url = "http://www.2sprout.com/missed/";
			string post = "ID=" + broadcastServerEncoded + "&date=" + dateEncoded + "&missed=";
			string packetsToReplace;

			int x;
			if (numLostPackets < 10)
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
				
				
				//string packetsEncoded = base64_encode((const unsigned char*) post.c_str(), strlen(post.c_str()));
				
				
				cout << "Recieving missed packets via plaintext url: http://www.2sprout.com/missed/?ID=" << BroadcastingServer << "&date=" << currentDate << "&missed=" << post << endl;
				cout << "Recieving missed packets via encoded url: " << url << endl;
					
					
				//Call the url to get the missed packets
				cout << "calling url: " << url << endl;
                                cout << packetsToReplace << endl;
				string packetsEncoded = base64_encode((const unsigned char*) packetsToReplace.c_str(), strlen(packetsToReplace.c_str()));
				
				post += packetsEncoded;
				string html = getHtml(url, post);
                cout << "recieved: " << html << endl;
				logFile("Recieved Missing Packets");
				//Tokenize the string based on newlines, since they can't
				//exist cause the json will bark
				string token;
				istringstream iss(html);
				while(getline(iss,token,'\n'))
				{
					//push the token
					sproutFeed.push(token);
				}
			}
			else
			{
                cout << "GREATER tHAN TEN" << endl;
				int maxLost = 0;	
                string url = "http://www.2sprout.com/missed/";

                string post = "ID=" + broadcastServerEncoded + "&date=" + dateEncoded + "&missed=";
				string packetsToReplace;
				for(x = 0; x < numLostPackets; x++)
				{
					if(maxLost == 10)
					{
						maxLost = 0;
						
                        cout << packetsToReplace << endl;
						string packetsEncoded = base64_encode((const unsigned char*) packetsToReplace.c_str(), strlen(packetsToReplace.c_str()));
                        packetsToReplace.clear();
                        post += packetsEncoded;
                        cout << "POST " << post << endl;

                        string html = getHtml(url, post);
                        cout << "recieved: " << html << endl;
						#warning automatically tokenize and put data into proper queue 
						string token;
						istringstream iss(html);
						while(getline(iss,token,'\n'))
						{
							//push the token
							sproutFeed.push(token);
						}
					    url = "http://www.2sprout.com/missed/";
						post = "ID=" + broadcastServerEncoded + "&date=" + dateEncoded + "&missed=";						
					}
					else
					{
					  char convertBuf[128];
					  snprintf(convertBuf, sizeof(convertBuf), "%d", packets[x]);
					  
					  packetsToReplace.append(convertBuf);
					  bzero(convertBuf, sizeof(convertBuf));
                      if(maxLost != 9)
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
					#warning automatically tokenize and put data into proper queue 
					string token;
					istringstream iss(html);
					while(getline(iss,token,'\n'))
					{
						//push the token
						sproutFeed.push(token);
					}
					url = "http://www.2sprout.com/missed/";
					post = "ID=" + broadcastServerEncoded + "&date=" + dateEncoded + "&missed=";				
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
        	fprintf(stderr,"Failed to connect to database\n");
        	fprintf(stderr,"%s\n",PQerrorMessage(Conn));
        	PQfinish(Conn);
        	exit(1);
    	}

 		while(1)
 		{
			pthread_mutex_lock(&mylock);
 			if(!sproutFeed.empty())
	 		{
				s.clear();
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
					logFile("Insert To DB: INSERT FAILED" + error);
				    fprintf(stderr, "INSERT failed: %s", PQerrorMessage(Conn));			   
				}
				else
				{
					logFile("Insert To DB: Data Inserted");
					cout << "Data Inserted" << endl;
				}	
				Query.clear();
				PQclear(result);
				logFile("Insert To DB: Clearing Query");
			}
			
			else
			{
				pthread_mutex_unlock(&mylock);
				if(usleep(1000) == -1)
				{
					printf("sleep failed\n");
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
				string s = sproutFeed.front();
				printf("read in: %s\n", s.c_str());
		 		string escapedString;    
    			string mysqlQuery;
		   
				
		  		unsigned long to_len = mysql_real_escape_string (conn, (char *)escapedString.c_str(), (char *)s.c_str(), strlen(s.c_str()));	//use the built in mysql function to put in escape characters...if ther are any	
				s.clear();
				mysqlQuery = "INSERT INTO " + table + " ("+ col +") VALUES (\"" + escapedString.c_str() + "\");"; //actual creation of the sql statment
 				if(mysql_query(conn, mysqlQuery.c_str()) != 0)
 		  		{
					mysqlQuery.clear();
 		   			cout << "error query failed" << endl;
          		}
          		
				mysqlQuery.clear();
          		sproutFeed.pop();
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
	
	return NULL;
}






 
/*
GetFeed uses Sys V Message Queues in order to transfer data from the Client into the User made application via
the API.
*/
  
void* getFeed(void *thread_arg)
{

	logFile("API: Initializing API");
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
					logFile("API: Value to big to send in one take");
					string toSend = s.substr(0,1024);
					cout << "toSend: " << toSend << endl;
					s = s.substr(1024,s.size());
					cout << "s: " << s << endl;
		  			sbuf.mtype = 1;
					sbuf.fullMsg = false;
					bzero(sbuf.mtext, sizeof(sbuf.mtext));
		    		(void) strncpy(sbuf.mtext, toSend.c_str(), strlen(toSend.c_str()));
		    		buf_length = strlen(sbuf.mtext) + 1;
	
		    		if (msgsnd(msqid, &sbuf, sizeof(sbuf), false) < 0) 
					{
		       			perror("msgsnd");
		    		}
	
				}
				if(s.size() < 1024)
				{
					
		  			sbuf.mtype = 1;
					sbuf.fullMsg = true;
					logFile("API: Sending: " + s + " Through API");
					cout << "TRYING TO SEND= " << s << endl;
					bzero(sbuf.mtext, sizeof(sbuf.mtext));
		    		(void) strncpy(sbuf.mtext, s.c_str(), strlen(s.c_str()));
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
				logFile("API: Item Small enough to send in one take");
				sbuf.mtype = 1;
				sbuf.fullMsg = true;
				logFile("API: Sending: " + s + " Through API");
				cout << "TRYING TO SEND= " << s << endl;
				bzero(sbuf.mtext, sizeof(sbuf.mtext));
	    		(void) strncpy(sbuf.mtext, s.c_str(), strlen(s.c_str()));
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
	unlink(sproutPipe);
	printf("Cleaning Files\n");
    fflush(stdout);
	closeAnnounce();
	if(useDatabase == false)
	{
		if(msgctl(msqid, IPC_RMID, NULL) == 1) //Delete the Message Queue
		{
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

	logFile("Registering Client");
  	char Portbuffer[10];
	sprintf(Portbuffer, "%i", MYPORT);
	string port = Portbuffer;
	string url = "http://2sprout.com/connect/";
	string post = "ID=" + apiKey + "&port=" + port;
	
	port.clear();
	bzero(Portbuffer, sizeof(Portbuffer));
	string html = "";

	html = getHtml(url, post);
	
	string decoded = base64_decode(html);
  	//XOR with the secret cypher
	string value(decoded);
	
	#warning "This Cipher should not be hardcoded, take out when ssl is implemented"
	string key("2#sPr0uT5!");
	value = XOR(decoded,key);
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
		cout << "cipher: " << cipher << endl;
		cout << "updated Password" << updatedPassword << endl;
		cout << sleeptime << endl;
			       
	}
	else //just put in as a failsafe. It should never reach this point.
	{
		logFile("Register Client: Wrong input given; re-registering in 2 secounds");
		sleep(2);
		registerClient();
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
	logFile("registerSignals: Registering Signals");
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
	logFile("Starting Client");
	
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

   


