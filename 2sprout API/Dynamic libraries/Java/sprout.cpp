/*
THIS IS AN UNPUBLISHED WORK CONTAINING 2SPROUT INC CONFIDENTIAL AND PROPRIETARY INFORMATION. 
DISCLOSURE, USE, OR REPRODUCTION WITHOUT AUTHORIZATION OF 2SPROUT INC IS STRICTLY PROHIBITED.
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <cstdlib>



using namespace std;

#define MSGSZ     10

typedef struct msgbuf1{
    long    mtype;
    char    mtext[MSGSZ];
	bool 	fullMsg;
} message_buf1;


message_buf1  rbuf;




/*
2Sprout API getSproutItem()
This function will return a char pointer to the next piece of data
*/

char * getSproutItem()
{
	
	int msqid;
    key_t key;
	int msgflg = 0666;

    /*
     * Get the message queue id for the
     * "name" 1234, which was created by
     * the server.
     */
    key = 5121;
	
	string completedMessage;
	
    if ((msqid = msgget(key, msgflg)) < 0) {
		printf("Unable to interact with Client. Is the client running?\n");
        exit(1);
    }

    
    /*
     * Receive an answer of message type 1.
     */
    if (msgrcv(msqid, &rbuf, sizeof(rbuf), 1, 0) < 0) {
		printf("Unable to recieve Message\n");	
		exit(1);
    }

	if(rbuf.fullMsg == true)
	{
		completedMessage = rbuf.mtext;
	}

	else
	{
		while(rbuf.fullMsg == false)
		{
			completedMessage += rbuf.mtext;
			bzero(rbuf.mtext, sizeof(rbuf.mtext));
			if (msgrcv(msqid, &rbuf, sizeof(rbuf), 1, 0) < 0) 
			{
				printf("Unable to recieve Message\n");	
	    	}
		}
		if(rbuf.fullMsg == true)
		{
			completedMessage += rbuf.mtext;
			bzero(rbuf.mtext, sizeof(rbuf.mtext));
		}
	}

	return (char *)completedMessage.c_str();	
}



