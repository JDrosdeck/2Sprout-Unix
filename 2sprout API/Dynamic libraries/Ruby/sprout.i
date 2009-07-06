%module sprout

%{
extern char * jsonGetMember(char *json, char *key);
extern bool jsonHasKey(char * json, char * key);
extern char* getSproutItem();
%}
extern char * jsonGetMember(char *json, char *key);
extern bool jsonHasKey(char * json, char * key);
extern char* getSproutItem();

