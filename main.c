#include <stdio.h>
#include <stdlib.h>
#include "src/ClientSlqliteinC.h"

int main(){

    char *error_msg = "";

    csl_CreateDataBase("identity.db");
    csl_ListDataBase();
	response_query_sqlite *response = csl_SelectResponse();

	if(response->success == 1){
	    for (int i = 0; i < response->countData; i++) {
            Payload payloadR = response->payload[i];
            for (int i = 0; i < payloadR.argc; i++) {
                printf("%s = %s\n", payloadR.azColName[i], payloadR.argv[i] ? payloadR.argv[i] : "NULL");
            }
        }
	}

    csl_FreeResponseQuery(response);

	if( csl_ConectionDB("identity.db") != 1){
		//Ctrl error;
	}

    if( csl_CreateTable("CREATE TABLE Company(Id INTEGER PRIMARY KEY, CompanyName TEXT, CompanyLegalID TEXT, CompanyStatusId INT );") != 1){
		//Ctrl error;
    }

    char *sql = "INSERT INTO Company (CompanyName, CompanyLegalID, CompanyStatusId) VALUES('Company 1', '30542342322' , 1);";

    char *sqlFormater = "INSERT INTO Company (CompanyName, CompanyLegalID, CompanyStatusId) VALUES('%s', '%s', %d);";

    char *sqlMassive[] = {
                        "INSERT INTO Company (CompanyName, CompanyLegalID, CompanyStatusId) VALUES('Company 2', '30542342322' , 1);",
                        "INSERT INTO Company (CompanyName, CompanyLegalID, CompanyStatusId) VALUES('Company 3', '30542342322' , 1);",
                        "INSERT INTO Company (CompanyName, CompanyLegalID, CompanyStatusId) VALUES('Company 4', '30542342322' , 1);",
                        "INSERT INTO Company (CompanyName, CompanyLegalID, CompanyStatusId) VALUES('Company 5', '30542342322' , 1);",
                        "INSERT INTO Company (CompanyName, CompanyLegalID, CompanyStatusId) VALUES('Company 6', '30542342322' , 1);",
                        "INSERT INTO Company (CompanyName, CompanyLegalID, CompanyStatusId) VALUES('Company 7', '30542342322' , 1);",
                        NULL
                        };

    if(csl_QuerySqlInsertFormater(sqlFormater, &error_msg, 3, "Company 312313", "3409429321", 10) != 1){
        printf("%s\n", error_msg);
    }

    if( csl_QuerySqlInsert(sql, &error_msg) != 1){
        fprintf(stderr,"Error : %s \n", error_msg);
		//Ctrl error;
    }

    char **sqlList;
    if( csl_QuerySqlInsertMassive( sqlMassive,&error_msg, &sqlList) != 1){
        printf("%s \n", error_msg);
    }

    char *sqlRequest = "SELECT * FROM Company;";
    csl_QuerySqlSelectRequest(sqlRequest);
    response_query_sqlite *responseRequest = csl_SelectResponse();

	if(responseRequest->success == 1){

	    for (int i = 0; i < responseRequest->countData; i++) {
            Payload payloadR = responseRequest->payload[i];
            for (int i = 0; i < payloadR.argc; i++) {
                printf("%s = %s\n", payloadR.azColName[i], payloadR.argv[i] ? payloadR.argv[i] : "NULL");
            }
        }

	}else{
        printf("%s \n", responseRequest->message);
	}

	csl_FreeResponseQuery(responseRequest);

	char *sql_update = "UPDATE Company set CompanyName = 'Company 100' WHERE Id = 4;";
	int responseUpdate = csl_QuerySqlUpdateRequest(sql_update);
	response_query_sqlite *responseRequestUpdate = csl_SelectResponse();

	if(responseUpdate != 1){
        printf("update : %s", responseRequestUpdate->message);
	}

	if(responseRequestUpdate->success != 1){
        printf("Otra manera de validar el error.\n Error : %s \n", responseRequestUpdate->message);
	}

    csl_FreeResponseQuery(responseRequestUpdate);

	csl_CloseConection();
    getchar();
    return 0;
}
