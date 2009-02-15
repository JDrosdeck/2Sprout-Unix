2sproutClient : md5.o SproutClient.o 
	g++ -I/opt/local/include/postgresql83 -L/usr/local/pgsql/lib -L/usr/local/lib/mysql -I/usr/local/include/mysql -lmysqlclient -lm -lcurl -lssl -lcrypto -lz -lpq -o 2sproutClient SproutClient.o md5.o

md5.o : md5.cpp
	g++ md5.cpp

SproutClient.o : SproutClient.cpp
	g++ -I/opt/local/include/postgresql83 -L/usr/local/pgsql/lib -L/usr/local/lib/mysql -I/usr/local/include/mysql -c SproutClient.cpp
	

	