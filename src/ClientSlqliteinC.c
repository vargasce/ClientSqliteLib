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
#include <stdarg.h>

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
void errorSelectRequet(char *err_msg);
int ExecuteQuery(char *sqlRequest, char **err_msg);
char *BuildSqlRequest(const char* formater, char **err_msg, va_list args);

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
        char *err_message = (char *) malloc(sizeof(char) * (strlen("Cannot open database: %s\n") + strlen(sqlite3_errmsg(db)) + 1));
        sprintf(err_message, "Cannot open database: %s", sqlite3_errmsg(db));
		log->error(err_message);
        sqlite3_close(db);
        free(err_message);
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
            log->warning("La carpeta DB ya existe.");
        } else {
            log->error("Error al crear carpeta DB.");
        }
    }

    result = mkdir(FOLDER_LOGNAME, PERMISSIONS);
    if (result != 0) {
        if (errno == EEXIST) {
            log->warning("La carpeta LOG ya existe.");
        } else {
            log->error("Error al crear carpeta LOG.");

        }
    }
}

int InitDataBase(char *nameDB){

	int existNameBD = -1;
	sqlite3_stmt *res;

	int rc = sqlite3_open(SYSTEM_TABLE, &db);

	if(rc != SQLITE_OK){
        char *err_message = (char *) malloc(sizeof(char) * (strlen("Cannot open database: %s\n") + strlen(sqlite3_errmsg(db)) + 1));
        sprintf(err_message, "Cannot open database: %s", sqlite3_errmsg(db));
		log->error(err_message);
        sqlite3_close(db);
        free(err_message);
        return csl_ERROR;
	}

	char *sqlExisteDB = "SELECT Name FROM Tables WHERE Name = '';";
	sqlExisteDB = insertString(&sqlExisteDB, nameDB, 38);

	rc = sqlite3_prepare_v2(db, sqlExisteDB, -1, &res, 0);

    if (rc != SQLITE_OK) {
        char *err_message = (char *) malloc(sizeof(char) * (strlen("Failed to fetch data: %s") + strlen(sqlite3_errmsg(db)) + 1));
        sprintf(err_message, "Failed to fetch data: %s", sqlite3_errmsg(db));
		log->error(err_message);
        csl_CloseConection();
        sqlite3_free(res);
        free(err_message);
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
        char *err_message = (char *) malloc(sizeof(char) * (strlen("Cannot open database: %s\n") + strlen(sqlite3_errmsg(db)) + 1));
        sprintf(err_message, "Cannot open database: %s", sqlite3_errmsg(db));
		log->error(err_message);
        sqlite3_close(db);
        free(err_message);
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
        err_msg = (char *) malloc(sizeof(char) * (strlen("Cannot open database : ") + strlen(sqlite3_errmsg(db) ) + 1 ));
        sprintf(err_msg,"Cannot open database : %s", sqlite3_errmsg(db));
		log->error(err_msg);
        sqlite3_close(db);
        free(err_msg);
        return csl_ERROR;
	}

	char *sql = "SELECT * FROM Tables;";

	InitPointerResponseSQL();
    rc = sqlite3_exec(db, sql, callback, 0, &err_msg);

    if (rc != SQLITE_OK ) {
        err_msg = (char *) malloc(sizeof(char) * (strlen("Failed to select data. SQL error: \n") + strlen(sqlite3_errmsg(db) ) + 1 ));
        sprintf(err_msg,"Failed to select data. SQL error: \n%s", sqlite3_errmsg(db));
		log->error(err_msg);
		csl_CloseConection();
        return csl_ERROR;
    }

	csl_CloseConection();

	return csl_SUCCESS;
}

int callback(void *NotUsed, int argc, char **argv, char **azColName){

    NotUsed = 0;
    RESPONSE_QUERY->countData ++;

    RESPONSE_QUERY->payload = (Payload *) realloc(RESPONSE_QUERY->payload, sizeof(Payload) * RESPONSE_QUERY->countData);
    RESPONSE_QUERY->payload[RESPONSE_QUERY->countData - 1].argv = (char **) malloc(sizeof(argv));
    RESPONSE_QUERY->payload[RESPONSE_QUERY->countData - 1].azColName = (char **) malloc(sizeof(azColName));
    Payload *newPayload = &(RESPONSE_QUERY->payload[RESPONSE_QUERY->countData - 1]);

    newPayload->argc = argc;
    newPayload->argv = copyStringArray(argv, argc);
    newPayload->azColName = copyStringArray(azColName, argc);

    return 0;
}

void InitPointerResponseSQL(){
    if(RESPONSE_QUERY != NULL){
        csl_FreeResponseQuery(RESPONSE_QUERY);
    }
    RESPONSE_QUERY = (response_query_sqlite*) malloc(sizeof(response_query_sqlite));
    RESPONSE_QUERY->payload = (Payload *) malloc(sizeof(Payload));
    RESPONSE_QUERY->message = NULL;
    RESPONSE_QUERY->success = csl_SUCCESS;
    RESPONSE_QUERY->countData = 0;
}

void errorSelectRequet(char *err_msg){
    RESPONSE_QUERY = (response_query_sqlite*) malloc(sizeof(response_query_sqlite));
    RESPONSE_QUERY->payload = NULL;
    RESPONSE_QUERY->message = (char *) malloc(sizeof(char) * strlen(err_msg));
    strcpy(RESPONSE_QUERY->message, err_msg);
    RESPONSE_QUERY->success = csl_ERROR;
    RESPONSE_QUERY->countData = 0;
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
        log->error("Not exist conection db");
        return csl_ERROR;
    }

    char *err_msg = 0;
    int rc = sqlite3_open(conectionName, &db);

    if(rc != SQLITE_OK){
        char *msj = (char *) malloc(sizeof(char) * (strlen("Failed conection db ") + strlen(sqlite3_errmsg(db)) + 1 ));
        sprintf(msj, "Failed conection db : %s", sqlite3_errmsg(db));
        log->error(msj);
        free(msj);
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
        log->error("Not exist conection db");
        return csl_ERROR;
    }

    char *err_msg = 0;
    int rc = sqlite3_open(conectionName, &db);

    if(rc != SQLITE_OK){
        char *msj = (char *) malloc(sizeof(char) * (strlen("Failed conection db ") + strlen(sqlite3_errmsg(db)) + 1 ));
        sprintf(msj, "Failed conection db : %s", sqlite3_errmsg(db));
        log->error(msj);
        *err = msj;
        return csl_ERROR;
    }

    rc = sqlite3_exec(db, sqlRequest, 0, 0, &err_msg);

    if(rc != SQLITE_OK){
        *err = err_msg;
        log->error(err_msg);
        return csl_ERROR;
    }

    return csl_SUCCESS;
}

/**
*   @brief Insertar datos en tabla con template
*   @param sql Format.
*   @param Error mensage
*   @param Cantidad parametros
*   @param Lista argumentos.
*   @return 1 success -1 error
*/
int csl_QuerySqlInsertFormater(char *format, char **err_msg, int countFormater, ...){
    va_list args1, args2;
    va_start(args1, countFormater);
    va_copy(args2, args1);
    char *error_msg = 0;

    char *sql = BuildSqlRequest(format,&error_msg, args2);

    va_end(args2);
    va_end(args1);

    if(sql == NULL){
        *err_msg = error_msg;
        log->error(error_msg);
        return csl_ERROR;
    }

    int result = csl_QuerySqlInsert(sql, &error_msg);

    if(result == csl_ERROR){
        *err_msg = error_msg;
        log->error(error_msg);
        return csl_ERROR;
    }

    return csl_SUCCESS;
}

/**
*   @brief Insertar datos en tabla, antes realizar conexion a la misma.
            llamar 'csl_SelectResponse()' para obtener datos.
*   @param sql request.
*   @param Error mensage
*   @return 1 success -1 error
*/
int csl_QuerySqlSelectRequest(char *sqlRequest){
    char *msg_response = 0;
    int result = ExecuteQuery(sqlRequest, &msg_response);
    return result;
}

/**
*   @brief Update data
*   @param sql request.
*   @return 1 success -1 error
*/
int csl_QuerySqlUpdateRequest(char *sqlRequest){
    char *msg_response = 0;
    int result = ExecuteQuery(sqlRequest, &msg_response);
    return result;
}

/**
*   @brief Delete data
*   @param sql request.
*   @return 1 success -1 error
*/
int csl_QuerySqlDeleteRequest(char *sqlRequest){
    char *msg_response = 0;
    int result = ExecuteQuery(sqlRequest, &msg_response);
    return result;
}

/**
*   @brief Executa sentencia sql, retorna msg result.
*   @param sql request.
*   @param Error mensage return
*   @return 1 success -1 error
*/
int ExecuteQuery(char *sqlRequest, char **err_msg){

    if(!conectionName){
        char *err = (char *) malloc(sizeof(char) * (strlen("Not exist conection db ") + strlen(sqlite3_errmsg(db)) + 1 ));
        sprintf(err, "Not exist conection db : %s", sqlite3_errmsg(db));
        log->error(err);
        *err_msg = err;
        return csl_ERROR;
    }

    char *error_msg = 0;
    int rc = sqlite3_open(conectionName, &db);

    if(rc != SQLITE_OK){
        error_msg = (char *) malloc( sizeof(char) * (strlen("Failed conection db : ") + strlen(sqlite3_errmsg(db)) + 1 ));
        sprintf(error_msg, "Failed conection db : %s", sqlite3_errmsg(db));
        log->error(error_msg);
        *err_msg = error_msg;
        return csl_ERROR;
    }

    InitPointerResponseSQL();
    rc = sqlite3_exec(db, sqlRequest, callback, 0, &error_msg);

    if (rc != SQLITE_OK ) {
        char *err_reponse = (char *) malloc(sizeof(char) * (strlen("Failed request. \n SQL error :  ") + strlen(error_msg)));
        sprintf(err_reponse, "Failed request. \n SQL error : %s",error_msg);
        errorSelectRequet(err_reponse);
        *err_msg = err_reponse;
        log->error(err_reponse);
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
        log->error("Not exist conection db");
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

    log->debug("Name DB: %s : System table: %s",1, nameDB, SYSTEM_TABLE);

	int existNameBD = -1;
	sqlite3_stmt *res;
	int rc = sqlite3_open(SYSTEM_TABLE, &db);

	char *sqlExisteDB = "SELECT Name FROM Tables WHERE Name = '';";
	sqlExisteDB = insertString(&sqlExisteDB, nameDB, 38);
	rc = sqlite3_prepare_v2(db, sqlExisteDB, -1, &res, 0);

    if (rc != SQLITE_OK) {
        char *err_message = (char *) malloc(sizeof(char) * (strlen("Failed to fetch data: %s") + strlen(sqlite3_errmsg(db)) + 1));
        sprintf(err_message, "Failed to fetch data: %s", sqlite3_errmsg(db));
        log->error(err_message);
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
        char *err_message = (char *) malloc(sizeof(char) * ( strlen("Cannot open database: %s\n") + strlen(sqlite3_errmsg(db)) + 1 ));
        sprintf(err_message, "Cannot open database: %s\n", sqlite3_errmsg(db));
		log->error(err_message);
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
    if(response->countData > 0){
        for (int i = 0; i < response->countData; i++) {
            Payload payload = response->payload[i];
            for (int j = 0; j < payload.argc; j++) {
                free(payload.argv[j]);
                free(payload.azColName[j]);
            }
            free(payload.argv);
            free(payload.azColName);
        }
        free(response->payload);
    }

	free(response->message);
	response->countData = 0;
}

/**
 * @brief Construye y formatea sql.
 * @param sql model formater.
 * @param error mensaje return.
 * @param ... list params.
 * @return char sql.
*/
char *BuildSqlRequest(const char* formater, char **err_msg, va_list args){

    va_list args_copy;
    va_copy(args_copy, args);

    int length = vsnprintf(NULL, 0, formater, args_copy);
    if (length < 0) {
        *err_msg = "Error en la cantidad a formatear.\n";
        return NULL;
    }

    char* buffer = (char*)malloc((length + 1) * sizeof(char));
    if (buffer == NULL) {
        *err_msg = "Error solicitar memoria.\n";
        return NULL;
    }

    int result = vsnprintf(buffer, length + 1, formater, args);

    if (result < 0) {
        *err_msg = "Error al realizar el formato.\n";
        free(buffer);
        return NULL;
    }

    return buffer;
}

void InitLog(){
    if(log == NULL){
        log = CreateLog();
    }
}
