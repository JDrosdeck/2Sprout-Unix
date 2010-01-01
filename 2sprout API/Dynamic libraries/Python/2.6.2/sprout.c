/*
THIS IS AN UNPUBLISHED WORK CONTAINING 2SPROUT INC CONFIDENTIAL AND PROPRIETARY INFORMATION. 
DISCLOSURE, USE, OR REPRODUCTION WITHOUT AUTHORIZATION OF 2SPROUT INC IS STRICTLY PROHIBITED.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
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



#define MSGSZ     1024


/*
firstCatch is used to determine if the message was to big if this is the very first message of the set. This
allows for the differention between a malloc and realloc.
*/
int firstCatch = 0;

typedef struct msgbuf1{
    long    mtype;
    char    mtext[MSGSZ];
	int 	fullMsg;
} message_buf1;


message_buf1  rbuf;

char message[];
/*
2Sprout API getSproutItem()
This function will return a char pointer to the next piece of data
*/

char * getSproutItem()
{
	
	bzero(message, strlen(message));
	int msqid;
    key_t key;
	int msgflg = 0666;

    /*
     * Get the message queue id for the
     * "name" 1234, which was created by
     * the server.
     */
    key = 5121;
	
	char* completedMessage;
	
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

	if(rbuf.fullMsg == 1)
	{
		completedMessage = (char *)malloc(sizeof(char) * strlen(rbuf.mtext)-1);
		strncpy(completedMessage, rbuf.mtext, strlen(rbuf.mtext));
	}

	else
	{
		while(rbuf.fullMsg == 0)
		{
			if(firstCatch == 0)
			{
				firstCatch = 1;
				completedMessage = (char *)malloc(sizeof(char) * strlen(rbuf.mtext)-1);
				strncpy(completedMessage, rbuf.mtext, strlen(rbuf.mtext)-1);
			}
			else
			{
				completedMessage = realloc(completedMessage, strlen(completedMessage) + strlen(rbuf.mtext));
				strncat(completedMessage, rbuf.mtext, strlen(rbuf.mtext));
			}
			bzero(rbuf.mtext, sizeof(rbuf.mtext));
			if (msgrcv(msqid, &rbuf, sizeof(rbuf), 1, 0) < 0) 
			{
				printf("Unable to recieve Message\n");	
	    	}
		}
		if(rbuf.fullMsg == 1) //this is to catch the very last bit of data that gets sent
		{
			completedMessage = realloc(completedMessage, strlen(completedMessage) + strlen(rbuf.mtext));
			strncat(completedMessage, rbuf.mtext, strlen(rbuf.mtext));
			//completedMessage += rbuf.mtext;
			bzero(rbuf.mtext, sizeof(rbuf.mtext));
		}
	}

	    message[strlen(completedMessage)+1]; //we need to make sure we grab the /0 on the end
		strncpy(message ,completedMessage, strlen(completedMessage)+1);
		free (completedMessage);

	return message;
}



