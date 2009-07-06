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

char * getSproutItem();
char * jsonGetMember(char *json, char *key);
bool jsonHasKey(char * json, char * key);
