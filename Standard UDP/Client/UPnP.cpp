
#include <stdio.h>
#include <stdlib.h>

/*
Includes For UPNP Library
*/
#include "upnp/miniwget.h"
#include "upnp/miniupnpc.h"
#include "upnp/upnpcommands.h"
#include "upnp/upnperrors.h"


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
