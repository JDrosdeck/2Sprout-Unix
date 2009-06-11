
#include <stdio.h>
#include <string.h>
#include <cstdlib>

/*
Includes for postgres C library
*/
#include <libpq-fe.h>
#include <iomanip>


/*
Includes for mysql C library
*/
#include <my_global.h> 
#include <my_sys.h> 
#include <mysql.h>


using namespace std;


bool testConnection(string databaseType, string host, string port, string dbname, string user, string pass)
{
	if(databaseType == "postgres")
	{
		string connectionString = "host=" + host + " port=" + port + " dbname=" + dbname + " user=" + user + " password=" + pass;
		PGconn *Conn = PQconnectdb(connectionString.c_str());
		PGresult* result;

		if (PQstatus(Conn) == CONNECTION_BAD)
	    {
	        fprintf(stderr,"Failed to connect to database\n");
	        fprintf(stderr,"%s\n",PQerrorMessage(Conn));
	        PQfinish(Conn);
			return false;
	    }
		else
		{
			PQfinish(Conn);
			return true;
		}
	
	}
	else if(databaseType == "mysql")
	{
		MYSQL *conn;
		if (mysql_library_init(0,NULL,NULL))
		{
			return false;
		}
		
		conn = mysql_init (NULL);
	
    	if(conn == NULL)
    	{
			return false;
		}
 
    	if(mysql_real_connect(conn, (char *)host.c_str(), (char *)user.c_str(), (char *)pass.c_str(), (char *)dbname.c_str(), atoi(port.c_str()), NULL,0) == NULL)
    	{
    		mysql_close(conn);
			return false;
		}
		
		return true;	
	}
	
	
}