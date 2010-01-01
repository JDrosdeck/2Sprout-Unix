CC = g++
CFLAGS = -g
INCLUDE = -I/usr/local/pgsql/include -I/usr/local/include/mysql 
DLIBS = -static-libgcc -lstdc++-static -lssl -lcrypto -lz
SLIBS = /usr/local/lib/libcurl.a /usr/local/pgsql/lib/libpq.a /usr/local/lib/mysql/libmysqlclient.a /usr/lib/libminiupnpc.a
PROG = 2Sprout

SRCS = log.cpp strOperations.cpp UPnP.cpp htmlGrabber.cpp testDBConnection.cpp base64.cpp SproutClient.cpp

all: $(PROG)

$(PROG):	$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(INCLUDE) $(SRCS) $(DLIBS) $(SLIBS)

clean:
	rm -f $(PROG) *.o