
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


bool testConnection(string databaseType, string host, string port, string dbname, string user, string pass, string table, string col)
{
	if(databaseType == "postgres")
	{
		string connectionString = "host=" + host + " port=" + port + " dbname=" + dbname + " user=" + user + " password=" + pass;
		PGconn *Conn = PQconnectdb(connectionString.c_str());

		if (PQstatus(Conn) == CONNECTION_BAD)
	    {
	        fprintf(stderr,"Failed to connect to database\n");
	        fprintf(stderr,"%s\n",PQerrorMessage(Conn));
	        PQfinish(Conn);
			return false;
	    }
		else
		{
			//We can connect to the database, Make sure the table exists. If not create it
			string QueryString = "select * from pg_tables where tablename=\'" + table + "\';";
			PGresult* result = PQexec(Conn, QueryString.c_str());
			printf("Detecting Tables\n");
			if(PQresultStatus(result) != PGRES_TUPLES_OK)
			{
				PQclear(result);
			}
			else
			{
				int numTables = PQntuples(result);
				PQclear(result);
				printf("%i\n",numTables);
				if(numTables == 0) //the table dosen't exist. Create it
				{
					string tableCreation = "CREATE TABLE " + table +"(id serial NOT NULL, " + col + " text NOT NULL, CONSTRAINT " + table + "_data_pkey PRIMARY KEY (id)) WITH (OIDS=FALSE); ALTER TABLE " + table +  " OWNER TO " + user +";";
					printf ("%s\n", tableCreation.c_str());
					PQexec(Conn, tableCreation.c_str());
					
				}
				else //the table exists, check to see if the columns exists
				{
					string colExists = "SELECT attname FROM pg_attribute WHERE attrelid = (SELECT oid FROM pg_class WHERE relname = \'" + table +"\') AND attname = \'" + col +"\'";
					PGresult* colResult = PQexec(Conn, colExists.c_str());
					if(PQresultStatus(colResult) != PGRES_TUPLES_OK)
					{
						PQclear(colResult);
					}
					else
					{
						int numCol = PQntuples(colResult);
						if(numCol == 0)//the column dosen't exist. Create it.
						{
							//create the table sequence
							string createSeq = "CREATE SEQUENCE " +  table + "_id_seq INCREMENT 1 MINVALUE 1 MAXVALUE 9223372036854775807 START 1 CACHE 1; ALTER TABLE " + table + "_id_seq OWNER TO postgres;";
							PQexec(Conn, createSeq.c_str());
							string createIdCol = "ALTER TABLE " + table + " ADD COLUMN id integer;ALTER TABLE " + table + " ALTER COLUMN id SET STORAGE PLAIN;ALTER TABLE " + table + " ALTER COLUMN id SET NOT NULL;ALTER TABLE " + table + " ALTER COLUMN id SET DEFAULT nextval(\'" + table + "_id_seq\'::regclass);";
							PQexec(Conn, createIdCol.c_str());
							string createJsonCol = "ALTER TABLE " + table + " ADD COLUMN " + col + " text; ALTER TABLE " + table + " ALTER COLUMN " + col + " SET STORAGE EXTENDED; ALTER TABLE " + table + " ALTER COLUMN " + col + " SET NOT NULL;";
							PQexec(Conn, createJsonCol.c_str());	
						}
						PQclear(colResult);
					}
				}
			}
			
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
  	      mysql_library_end(); //stop using the library

			return false;
		}
		else //we could connect to the db 
		{
			printf("Connected To Database\n");
			//make sure the table exists
			MYSQL_RES *result;
			
			string tableQuery = "show tables like \"" + table + "\";";
			printf("%s\n", tableQuery.c_str());
			mysql_real_query(conn, tableQuery.c_str(), (unsigned int)strlen(tableQuery.c_str()));
			result = mysql_store_result(conn);
			if(result != NULL)
			{
				int resultCount = mysql_num_rows(result);
				printf("%i\n", resultCount);
				mysql_free_result(result);
				if(resultCount == 0) //The table does not exist
				{
					//create the table
					string createTableQuery = "CREATE TABLE \`" + dbname + "\`.\`" + table + "\`(\`id\` INT NOT NULL AUTO_INCREMENT, \`" + col + "\` text NOT NULL, PRIMARY KEY (\`id\`)) CHARACTER SET utf8;";
					printf("%s\n", createTableQuery.c_str());
					if(mysql_real_query(conn, createTableQuery.c_str(), (unsigned int)strlen(createTableQuery.c_str())) == 0)
					{
						printf("Table created successfully\n");
					}
					else
					{
						printf("failed to create table\n Error Code: %u\n Description: %s\n", mysql_errno(conn), mysql_error(conn));
					}
					
				}

				//make sure the columns exist
				string columnExistsQuery = "Show columns from " + table + " like \"" + col + "\"";
				printf("%s\n", columnExistsQuery.c_str());
				mysql_real_query(conn, columnExistsQuery.c_str(), (unsigned int)strlen(columnExistsQuery.c_str()));
				result = mysql_store_result(conn);
				int colResultCount = mysql_num_rows(result);
				printf("Col Amount: %i\n", colResultCount);
				if(colResultCount == 0) //The Table exists but the column does not
				{
					
					string createdColumn = "ALTER TABLE \`" + dbname +"\`.\`" + table + "\` ADD COLUMN \`" + col + "\` text NOT NULL;";
					string createdColumnID = "ALTER TABLE \`" + dbname + "\`.\`" + table + "\` ADD COLUMN \`id\` INT NOT NULL AUTO_INCREMENT, ADD PRIMARY KEY (\`id\`);";
					
					if(mysql_real_query(conn, createdColumn.c_str(), (unsigned int)strlen(createdColumn.c_str())) == 0)
					{
						printf("Table created successfully\n");
					}
					else
					{
						printf("failed to create table\n Error Code: %u\n Description: %s\n", mysql_errno(conn), mysql_error(conn));
					}
					if(mysql_real_query(conn, createdColumnID.c_str(), (unsigned int)strlen(createdColumnID.c_str())) == 0)
					{
						printf("Table created successfully\n");
					}
					else
					{
						printf("failed to create table\n Error Code: %u\n Description: %s\n", mysql_errno(conn), mysql_error(conn));
					}
					
				}
				
			}
			else
			{
				printf("Query failed\n");
			}
			
		}
		
		
		mysql_close(conn); //close the database connection
		mysql_library_end(); //stop using the library
	   
		return true;	
	}
	
	return false;
}