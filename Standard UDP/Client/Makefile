2sproutClient : md5.o SproutClient.o 
	gcc -I/opt/local/include/postgresql83 -L/usr/local/pgsql/lib -L/usr/local/lib/mysql -I/usr/local/include/mysql -lm -o 2sproutClient SproutClient.o md5.o -static-libgcc -lstdc++-static -lpq -lssl -lcrypto -lz -lcurl -lmysqlclient

md5.o : md5.cpp
	g++ md5.cpp

SproutClient.o : SproutClient.cpp
	g++ -I/opt/local/include/postgresql83 -L/usr/local/pgsql/lib -L/usr/local/lib/mysql -I/usr/local/include/mysql -c SproutClient.cpp
	

	