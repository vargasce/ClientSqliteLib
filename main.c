#include <stdio.h>
#include <stdlib.h>
#include "src/ClientSlqliteinC.h"

int main()
{
    csl_CreateDataBase("identity.db");
    csl_ListDataBase();
	response_query_sqlite *response = csl_SelectResponse();

	if(response->success == 1){
		printf(" Count columns : %d\n",response->payload->argc);

		for (int i = 0; i < response->payload->argc; i++) {
			printf("%s = %s\n", response->payload->azColName[i], response->payload->argv[i] ? response->payload->argv[i] : "NULL");
		}

	}

    csl_FreeResponseQuery(response);

	if( csl_ConectionDB("identity.db") != 1){
		printf("Error created conection\n");
	}

    // TODO (Tauriel#1#06/02/23): Probar utilizar la conexion, metodo para crear tablas en la db
    if (csl_CreateTable("CREATE TABLE Company(Id INT, CompanyName TEXT, CompanyLegalID TEXT, CompanyStatusId INT );") != 1){
        printf("Error created table\n");
    }

	csl_CloseConection();
    getchar();
    return 0;
}
