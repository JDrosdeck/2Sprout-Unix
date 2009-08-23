/*
Includes For UPNP Library
*/
#include "upnp/miniwget.h"
#include "upnp/miniupnpc.h"
#include "upnp/upnpcommands.h"
#include "upnp/upnperrors.h"

void setUPNP(char* port);
static void forwardPort(struct UPNPUrls * urls,struct IGDdatas * data, const char * iaddr,const char * iport,const char * eport,const char * proto);
