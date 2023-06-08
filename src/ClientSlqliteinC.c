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
#include "Log.h"
#include <sys/stat.h>
#include <errno.h>

#define PERMISSIONS 0777
#define csl_ERROR -1
#define csl_SUCCESS 1
#define ERROR_CONECTION_NAME_INVALID 2
#define FOLDER_DBNAME "db"
#define FOLDER_LOGNAME "log"
#define SYSTEM_TABLE "./db/system.db"
#define NAMETABLES "Tables"

//Internal functions
int InitSystem();
char *insertString(char** original, char* insert, int pos);
int InitDataBase(char *nameDB);
int Create_DB(char *nameDB);
void InserNameDBtoSystem(char *nameDB);
int callback(void *NotUsed, int argc, char **argv, char **azColName);
void InitLog();
void InitFolderDb();
char** copyStringArray(char** array, int size);
int countPoiterString(char** arrayString);

//SIN USO
void clearTerminal();
char *insertNameDB();

//Vars global system
sqlite3 *db = NULL;
char *conectionName = NULL;
response_query_sqlite *RESPONSE_QUERY = NULL;
Log *log = NULL;

/**
 * @brief Create db con el nombre indicado por parametro.
 * @param Nombre para la db.
 * @return 1 = sucees. -1 = error.
*/
int csl_CreateDataBase(char *nameDB){

    InitLog();

	if( InitSystem() != csl_ERROR){
		InitDataBase(nameDB);
		Create_DB(nameDB);
	}

	return csl_SUCCESS;
}

int InitSystem(){

    InitFolderDb();
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
        log->warning(err_msg);
        sqlite3_free(err_msg);
	}

	csl_CloseConection();

	return csl_SUCCESS;
}

void InitFolderDb(){
    int result = mkdir(FOLDER_DBNAME, PERMISSIONS);
    if (result != 0) {
        if (errno == EEXIST) {
            log->warning("La carpeta ya existe.");
        } else {
            log->error("Error al crear carpeta.");
        }
        printf("\tFolder : %s\n", FOLDER_DBNAME);
    }

    result = mkdir(FOLDER_LOGNAME, PERMISSIONS);
    if (result != 0) {
        if (errno == EEXIST) {
            log->warning("La carpeta ya existe.");
        } else {
            log->error("Error al crear carpeta.");

        }
        printf("\tFolder : %s\n", FOLDER_LOGNAME);
    }
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

    InitLog();

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
*   @brief Crear tabla en base de datos, antes realizar conexion a la misma.
*   @param sql request (char *sql = "create table ....").
*   @return 1 success -1 error
*/
int csl_CreateTable(char *sqlRequest){

    if(!conectionName){
        log->error("No existe conexion con la db");
        return csl_ERROR;
    }

    char *err_msg = 0;
    int rc = sqlite3_open(conectionName, &db);

    if(rc != SQLITE_OK){
        char *msj = "Failed conection db ";
        strcat( msj, sqlite3_errmsg(db));
        log->error(msj);
        return csl_ERROR;
    }

    rc = sqlite3_exec(db, sqlRequest, 0, 0, &err_msg);

    if(rc != SQLITE_OK){
        log->warning(err_msg);
        return csl_ERROR;
    }

    log->information("Succes create table.");
    return csl_SUCCESS;
}

/**
*   @brief Insertar datos en tabla, antes realizar conexion a la misma.
*   @param sql request.
*   @param Error mensage
*   @return 1 success -1 error
*/
int csl_QuerySqlInsert(char *sqlRequest, char **err){

    if(!conectionName){
        log->error("No existe conexion con la db");
        return csl_ERROR;
    }

    char *err_msg = 0;
    int rc = sqlite3_open(conectionName, &db);

    if(rc != SQLITE_OK){
        char *msj = "Failed conection db ";
        strcat( msj, sqlite3_errmsg(db));
        log->error(msj);
        return csl_ERROR;
    }

    rc = sqlite3_exec(db, sqlRequest, 0, 0, &err_msg);

    if(rc != SQLITE_OK){
        *err = err_msg;
        return csl_ERROR;
    }

    return csl_SUCCESS;
}

/**
*   @brief Insertar datos en tabla masivo, antes realizar conexion a la misma.
*   @param sql request.
*   @param Error mensage
*   @param Error list sql
*   @return 1 success -1 error
*/
int csl_QuerySqlInsertMassive(char **sqlRequest, char **err_msg, char ***sqlList){

    if(!conectionName){
        log->error("No existe conexion con la db");
        return csl_ERROR;
    }

    int countQuerys = countPoiterString(sqlRequest), result = 0, hasError = 0;
    char *error_msg = 0;
    char *error_send = (char *) malloc(sizeof(char) * 1);
    char **error_list = (char **) malloc(sizeof(char *) * (countQuerys+1) );
    memset(error_send, 0, strlen(error_send));

    for(int i = 0; i < countQuerys; i++){
        result = csl_QuerySqlInsert(sqlRequest[i], &error_msg);
        if(result == csl_ERROR){
            hasError = 1;
            error_list[i] = (char *) malloc( sizeof(char) * strlen(sqlRequest[i]) + 1);
            strcpy(error_list[i], sqlRequest[i]);
            log->error(error_list[i]);
            error_send = (char *) realloc(error_send, sizeof(char) * (strlen(error_send) + strlen(error_msg) + 1));
            error_send[strlen(error_send) + strlen(error_msg)] = '\n';
            strcat(error_send, error_msg);
            log->error(error_msg);
        }
    }

    *err_msg = error_send;
    error_list[countQuerys] = NULL;
    *sqlList = error_list;

    if(hasError == 1){
        return csl_ERROR;
    }

    return csl_SUCCESS;
}

int countPoiterString(char** arrayString){
    char **pointer = arrayString;
    int count = 0;

    for (int i = 0; pointer[i] != NULL; i++) {
        count++;
    }
    return count;
}

/**
 * @brief Guarda puntero de conexion db enviado por parametro para proximas consultar.
 * @param Nombre conexion.
 * @return 1 = sucees. -1 = error.
*/
int csl_ConectionDB(char *nameDB){

    InitLog();

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

    log->information("Succes create conexion db.");

	return csl_SUCCESS;
}

/**
 *	@brief Tomando como inicio un puntero char, inserta un string en la posicion indicada.
 *  @param original string.
 *  @param insert string.
 *  @param posicion insert string.
 *  @return nuevo puntero con el string y el insert.
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
		char *err_send = "Failed to Insert table - \n";
		strcat(err_send, err_msg);
		log->error(err_send);
		sqlite3_free(err_msg);
	}
	//int last_id = sqlite3_last_insert_rowid(db);
	//printf("The last Id of the inserted row is %d\n", last_id);
}

void clearTerminal() {
    if (system("clear") == -1) {
        log->error("Error al limpiar la terminal");
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

void InitLog(){
    if(log == NULL){
        log = CreateLog();
    }
}
