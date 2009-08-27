
2sproutClient : SproutClient.o 
	gcc -I/opt/local/include/postgresql83 -I/usr/local/include/mysql -lm -o 2sproutClient SproutClient.o base64.o testDBConnection.o htmlGrabber.o UPnP.o strOperations.o -static-libgcc -lstdc++-static -lssl -lcrypto -lz /usr/local/lib/libcurl.a /usr/local/pgsql/lib/libpq.a /usr/local/lib/mysql/libmysqlclient.a /usr/lib/libminiupnpc.a
	

SproutClient.o : SproutClient.cpp
	g++ -I/usr/local/pgsql/include -I/usr/local/include/mysql -c SproutClient.cpp
	
base64.o : base64.cpp
	g++ -c base64.cpp

testDBConnection.o : testDBConnection.cpp
	g++ -I/usr/local/pgsql/include -I/usr/local/include/mysql -c testDBConnection.cpp
	
htmlGrabber.o : htmlGrabber.cpp
	g++ -c htmlGrabber.cpp
	
UPnP.o : UPnP.cpp
	g++ -c UPnP.cpp
	
strOperations.o : strOperations.cpp
	g++ -c strOperations.cpp