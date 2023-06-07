#include <stdio.h>
#include <stdlib.h>
#include "src/ClientSlqliteinC.h"

int main(){

    char *error_msg = "";

    csl_CreateDataBase("identity.db");
    csl_ListDataBase();
	response_query_sqlite *response = csl_SelectResponse();

	if(response->success == 1){
		for (int i = 0; i < response->payload->argc; i++) {
			printf("%s = %s\n", response->payload->azColName[i], response->payload->argv[i] ? response->payload->argv[i] : "NULL");
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

    char *sqlMassive[] = {
                        "INSERT INTO Company (CompanyName, CompanyLegalID, CompanyStatusId) VALUES('Company 2', '30542342322' , 1);",
                        "INSERT INTO Company (CompanyName, CompanyLegalID, CompanyStatusId) VALUES('Company 3', '30542342322' , 1);",
                        "INSERT INTO Company (CompanyName, CompanyLegalID, CompanyStatusId) VALUES('Company 4', '30542342322' , 1);",
                        "INSERT INTO Company (CompanyName, CompanyLegalID, CompanyStatusId) VALUES('Company 5', '30542342322' , 1);",
                        "INSERT INTO Company (CompanyName, CompanyLegalID, CompanyStatusId) VALUES('Company 6', '30542342 , 1);",
                        "INSERT INTO Company (CompanyName, CompanyLegalID, CompanyStatusId) VALUES('Company 7', '30542342322' , 1);",
                        NULL
                        };

    if( csl_QuerySqlInsert(sql, &error_msg) != 1){
        fprintf(stderr,"Error : %s \n", error_msg);
		//Ctrl error;
    }

    char **sqlList;
    if( csl_QuerySqlInsertMassive( sqlMassive,&error_msg, &sqlList) != 1){
        printf("%s \n", error_msg);
    }

	csl_CloseConection();
    getchar();
    return 0;
}
