2sproutClient : md5.o SproutClient.o 
	gcc -I/opt/local/include/postgresql83 -I/usr/local/include/mysql -lm -o 2sproutClient SproutClient.o md5.o -static-libgcc -lstdc++-static -lssl -lcrypto -lz /usr/local/lib/libcurl.a /usr/lib/libldap_r.dylib /usr/local/pgsql/lib/libpq.a /usr/local/lib/mysql/libmysqlclient.a

md5.o : md5.cpp
	g++ md5.cpp

SproutClient.o : SproutClient.cpp
	g++ -I/opt/local/include/postgresql83 -L/usr/local/pgsql/lib -L/usr/local/lib/mysql -I/usr/local/include/mysql -c SproutClient.cpp
	

	