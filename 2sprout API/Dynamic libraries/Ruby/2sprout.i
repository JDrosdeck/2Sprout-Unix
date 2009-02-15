%module sprout


%{
extern int startFeed(int portNumber);
extern int stopFeed(int portNumber);
extern int getFeed();
extern char* getNextItem();
%}

extern int startFeed(int portNumber);
extern int stopFeed(int portNumber);
extern char* getNextItem();
extern int getFeed();

