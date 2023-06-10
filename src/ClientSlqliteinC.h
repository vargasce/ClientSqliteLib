
#include <sqlite3.h>

typedef struct {
	int argc;  		  //number columns
	char **argv;      //fields in the row.
	char **azColName; //column names
} Payload;

typedef struct {
	int success;      // 1 success, 0 error
	Payload *payload;
	int countData;    //cantidad de tuplas
	char *message;
} response_query_sqlite;

int csl_CreateDataBase(char *nameDB);
int csl_ConectionDB(char *nameDB);
void csl_CloseConection();
int csl_ListDataBase();
int csl_CreateTable(char *sqlRequest);
int csl_QuerySqlInsert(char *sqlRequest, char **err_msg);
int csl_QuerySqlInsertMassive(char **sqlRequest, char **err_msg, char ***sqlList);
int csl_QuerySqlSelectRequest(char *sqlRequest);
void csl_FreeResponseQuery(response_query_sqlite *response);
response_query_sqlite *csl_SelectResponse();
