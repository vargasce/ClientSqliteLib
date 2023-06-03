/*
 * ClientSlqliteinC.c
 *
 * Copyright 2023  <kali@kali>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "ClientSlqliteinC.h"

#define csl_ERROR -1
#define csl_SUCCESS 1
#define ERROR_CONECTION_NAME_INVALID 2
#define SYSTEM_TABLE "./db/system.db"
#define NAMETABLES "Tables"

//Internal functions
int InitSystem();
char *insertString(char** original, char* insert, int pos);
int InitDataBase(char *nameDB);
int Create_DB(char *nameDB);
void InserNameDBtoSystem(char *nameDB);
int callback(void *NotUsed, int argc, char **argv, char **azColName);
char** copyStringArray(char** array, int size);

//SIN USO
void clearTerminal();
char *insertNameDB();

//Vars global system
sqlite3 *db = NULL;
char *conectionName = NULL;
response_query_sqlite *RESPONSE_QUERY = NULL;

/**
 * @brief Create db con el nombre indicado por parametro.
 * @param Nombre para la db.
 * @return 1 = sucees. -1 = error.
*/
int csl_CreateDataBase(char *nameDB){

	if( InitSystem() != csl_ERROR){
		InitDataBase(nameDB);
		Create_DB(nameDB);
	}

	return csl_SUCCESS;
}

int InitSystem(){

	char *err_msg;

	int rc = sqlite3_open(SYSTEM_TABLE, &db);

	if(rc != SQLITE_OK){
		fprintf(stderr, "Cannot open database: %s\n",sqlite3_errmsg(db));
        sqlite3_close(db);
        return csl_ERROR;
	}

	char *sql = "CREATE TABLE  (Id INTEGER PRIMARY KEY, Name TEXT);";
	char * sqlAdd = insertString(&sql, NAMETABLES, 13);

	rc = sqlite3_exec(db, sqlAdd, 0, 0, &err_msg);

	if(rc != SQLITE_OK){
		//fprintf(stderr, "Failed to create table\n");
        //fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
	}

	csl_CloseConection();

	return csl_SUCCESS;
}

int InitDataBase(char *nameDB){

	int existNameBD = -1;
	sqlite3_stmt *res;

	int rc = sqlite3_open(SYSTEM_TABLE, &db);

	if(rc != SQLITE_OK){
		fprintf(stderr, "Cannot open database: %s\n",sqlite3_errmsg(db));
        sqlite3_close(db);
        return csl_ERROR;
	}

	char *sqlExisteDB = "SELECT Name FROM Tables WHERE Name = '';";
	sqlExisteDB = insertString(&sqlExisteDB, nameDB, 38);

	rc = sqlite3_prepare_v2(db, sqlExisteDB, -1, &res, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
        csl_CloseConection();
        sqlite3_free(res);
        return csl_ERROR;
    }

    rc = sqlite3_step(res);

    if (rc == SQLITE_ROW) {
        char *result = (char*)sqlite3_column_text(res, 0);
        if(strcmp(result,nameDB) == 0){
			existNameBD = 1;
		}
    }

    sqlite3_finalize(res);

	if(existNameBD != 1){
		InserNameDBtoSystem(nameDB);
	}

	csl_CloseConection();

	return csl_SUCCESS;
}

int Create_DB(char *nameDB){

	int rc = sqlite3_open(nameDB, &db);

	if(rc != SQLITE_OK){
		fprintf(stderr, "Cannot open database: %s\n",sqlite3_errmsg(db));
        sqlite3_close(db);
        return csl_ERROR;
	}

	csl_CloseConection();

	return csl_SUCCESS;
}

/**
 * @brief Obtiene lista db y la guarda en el puntero  de 'response_query_sqlite'
 * @return 1 = sucees. -1 = error.
*/
int csl_ListDataBase(){

	int rc = sqlite3_open(SYSTEM_TABLE, &db);
	char *err_msg;

	if(rc != SQLITE_OK){
		fprintf(stderr, "Cannot open database: %s\n",sqlite3_errmsg(db));
        sqlite3_close(db);
        return csl_ERROR;
	}

	char *sql = "SELECT * FROM Tables;";

    rc = sqlite3_exec(db, sql, callback, 0, &err_msg);

    if (rc != SQLITE_OK ) {
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);
		csl_CloseConection();
        return csl_ERROR;
    }

	csl_CloseConection();

	return csl_SUCCESS;
}

int callback(void *NotUsed, int argc, char **argv, char **azColName){

	NotUsed = 0;
    RESPONSE_QUERY = (response_query_sqlite*) malloc(sizeof(response_query_sqlite));
    RESPONSE_QUERY->payload = (Payload *) malloc(sizeof(Payload));
    RESPONSE_QUERY->payload->argv = (char **) malloc(sizeof(argv));
    RESPONSE_QUERY->payload->azColName = (char **) malloc(sizeof(azColName));

    RESPONSE_QUERY->message = NULL;
    RESPONSE_QUERY->success = csl_SUCCESS;
    RESPONSE_QUERY->payload->argc = argc;
    RESPONSE_QUERY->payload->argv = copyStringArray(argv, argc);
    RESPONSE_QUERY->payload->azColName = copyStringArray(azColName, argc);

    return 0;
}

/**
 * @brief Retorna puntero de la consulta a db
 * @return *response_query_sqlite
*/
response_query_sqlite *csl_SelectResponse(){
	return RESPONSE_QUERY;
}

/**
 * @brief Limpia puntero y conexion de la db
 * @return void.
*/
void csl_CloseConection(){
	sqlite3_close(db);
	conectionName = NULL;
	db = NULL;
}

/**
 * @brief Guarda puntero de conexion db enviado por parametro para proximas consultar.
 * @param Nombre conexion.
 * @return 1 = sucees. -1 = error.
*/
int csl_ConectionDB(char *nameDB){

	int existNameBD = -1;
	sqlite3_stmt *res;
	int rc = sqlite3_open(SYSTEM_TABLE, &db);

	char *sqlExisteDB = "SELECT Name FROM Tables WHERE Name = '';";
	sqlExisteDB = insertString(&sqlExisteDB, nameDB, 38);
	rc = sqlite3_prepare_v2(db, sqlExisteDB, -1, &res, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
        csl_CloseConection();
        sqlite3_free(res);
        return csl_ERROR;
    }

    rc = sqlite3_step(res);

    if (rc == SQLITE_ROW) {
        char *result = (char*)sqlite3_column_text(res, 0);
        if(strcmp(result,nameDB) == 0){
			existNameBD = 1;
		}
    }

    sqlite3_finalize(res);
    csl_CloseConection();

    if(existNameBD != 1){
		return csl_ERROR;
	}

	rc = sqlite3_open(nameDB, &db);
	conectionName = (char *) malloc(strlen(nameDB));
	strcpy(conectionName, nameDB);

	if(rc != SQLITE_OK){
		fprintf(stderr, "Cannot open database: %s\n",sqlite3_errmsg(db));
		csl_CloseConection();
		return csl_ERROR;
	}

	return csl_SUCCESS;
}

/**
 *	Tomando como inicio un puntero char, inserta un string
 *  en la posicion indicada.
 *  retorna un nuevo puntero con el string y el insert.
 **/
char *insertString(char** original, char* insert, int pos){

    int originalLen = strlen(*original);
    int insertLen = strlen(insert);
    int newLen = originalLen + insertLen + 1;
    int posResto = pos + insertLen;

    char* aux = (char*) malloc(newLen * sizeof(char));
    memset(aux,' ', newLen);
    strcpy(aux, *original);

    char* temp = (char*) malloc(newLen * sizeof(char));
    memset(temp,' ',newLen);

    strncpy(temp, aux, pos);
    strcpy(temp + pos, insert);

    temp = (char*) realloc(temp, originalLen * sizeof(char));

    strcpy(temp + posResto, aux + pos);

	return temp;
}

void InserNameDBtoSystem(char *nameDB){

	char *err_msg;
	char *sql = "INSERT INTO (Name) VALUES ('');";
	sql = insertString(&sql, NAMETABLES, 12);
	sql = insertString(&sql, nameDB, 28 + strlen(NAMETABLES));

	int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

	if(rc != SQLITE_OK){
		printf("%s\n", sql);
		fprintf(stderr, "Failed to Insert table\n");
		fprintf(stderr, "SQL error: %s\n", err_msg);
		sqlite3_free(err_msg);
	}

	int last_id = sqlite3_last_insert_rowid(db);
	printf("The last Id of the inserted row is %d\n", last_id);

}

void clearTerminal() {
    if (system("clear") == -1) {
        perror("Error al limpiar la terminal");
        exit(EXIT_FAILURE);
    }
}

char *insertNameDB(){

	char *buffer = (char *) malloc(20);
	char *db = ".db\0";

	if (buffer==NULL)return NULL;

	getchar();
	memset(buffer,0,20);
	fgets(buffer,15,stdin);
	buffer[strlen(buffer) - 1] = ' ';
	strcat(buffer,db);

	return buffer;
}

char** copyStringArray(char** array, int size) {
    char** copy = (char**)malloc(size * sizeof(char*));
    for (int i = 0; i < size; i++) {
        copy[i] = strdup(array[i]);
    }
    return copy;
}

/**
 * @brief LIbera memoria puntero 'response_query_sqlite'.
 * @param puntero 'response_query_sqlite'.
 * @return void.
*/
void csl_FreeResponseQuery(response_query_sqlite *response){
    response->payload->argc = -9;
    response->payload->argv = NULL;
    response->payload->azColName = NULL;
    free(response->payload);
	free(response);
}
