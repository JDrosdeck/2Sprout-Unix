

2sproutClient : md5.o SproutClient.o 
	gcc -I/opt/local/include/postgresql83 -I/usr/local/include/mysql -lm -o 2sproutClient SproutClient.o base64.o md5.o -static-libgcc -lstdc++-static -lssl -lcrypto -lz /usr/local/lib/libcurl.a /usr/lib/libldap_r.dylib /usr/local/pgsql/lib/libpq.a /usr/local/lib/mysql/libmysqlclient.a
	
md5.o : md5.cpp
	g++ -c md5.cpp

SproutClient.o : SproutClient.cpp
	g++ -I/opt/local/include/postgresql83 -L/usr/local/pgsql/lib -L/usr/local/lib/mysql -I/usr/local/include/mysql -c SproutClient.cpp base64.cpp
	

cast : webCast.o base64.o
	g++ -o cast webCast.o base64.o 

webCast.o : webCast.cpp base64.cpp
	g++ -c webCast.cpp base64.cpp