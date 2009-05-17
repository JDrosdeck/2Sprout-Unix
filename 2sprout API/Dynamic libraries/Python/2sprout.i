%module sprout



%{
extern int startFeed();
extern int stopFeed();
extern int getFeed();
extern char* getNextItem();
%}

extern int startFeed();
extern int stopFeed();
extern int getFeed();
extern char* getNextItem();

