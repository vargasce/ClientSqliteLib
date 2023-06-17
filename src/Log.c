/***************************************************************
 *  Project:
 *  Function:
 ***************************************************************
 *  Author: Cristian
 *  Name: Cristian
 ***************************************************************
 *
 *  Copyright 2007 by Cristian Vargas
 *
 ***************************************************************/

#include "Log.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

Log *CreateLog() {
    Log* log = (Log*) malloc(sizeof(Log));
    log->information = informationFunc;
    log->error = errorFunc;
    log->warning = warningFunc;
    log->debug = debugFunc;
    return log;
}

void informationFunc(const char* message) {
    char *messageSend = (char *) malloc(sizeof(char) * (strlen(message) + strlen("[Information] ") + SIZETIME) );
    char *timeNow = GetTimeNow();
    sprintf(messageSend,"[Information] [%s] %s\n",timeNow, message);
    SaveLog(messageSend);
}

void errorFunc(const char* message) {
    char *messageSend = (char *) malloc(sizeof(char) * (strlen(message) + strlen("[Error] ") + SIZETIME) );
    char *timeNow = GetTimeNow();
    sprintf(messageSend,"[Error] [%s] %s \n",timeNow, message);
    printf("%s \n", messageSend);
    SaveLog(messageSend);
}

void warningFunc(const char* message) {
    char *messageSend = (char *) malloc(sizeof(char) * (strlen(message) + strlen("[Warning] ") + SIZETIME) );
    char *timeNow = GetTimeNow();
    sprintf(messageSend,"[Warning] [%s] %s \n",timeNow, message);
    SaveLog(messageSend);
}

void debugFunc(char *stringFormaterDebug, int countFormater, ...) {
    va_list args1, args2;
    va_start(args1, countFormater);
    va_copy(args2, args1);

    int length = vsnprintf(NULL, 0, stringFormaterDebug, args2);
    if (length < 0) {
        printf("[LOG->DEBUG] Error en la cantidad a formatear.\n");
        return NULL;
    }

    char* buffer = (char*)malloc((length + 1) * sizeof(char));
    if (buffer == NULL) {
        printf("[LOG->DEBUG] Error solicitar memoria\n");
        return NULL;
    }

    int resultFormater = vsnprintf(buffer, length + 1, stringFormaterDebug, args1);

    if (resultFormater < 0) {
    printf("[LOG->DEBUG] Error al realizar el formato.\n");
        free(buffer);
        return NULL;
    }

    va_end(args2);
    va_end(args1);

    char *messageSend = (char *) malloc(sizeof(char) * (strlen(buffer) + strlen("[Debug] ") + SIZETIME) );
    char *timeNow = GetTimeNow();
    sprintf(messageSend,"[Debug] [%s] %s \n",timeNow, buffer);
    printf("%s", messageSend);
    SaveLog(messageSend);
}

void SaveLog(char *message){
    FILE *archivo = fopen(ARCHIVO, "a+");
    if (archivo != NULL) {
        fputs(message, archivo);
        fclose(archivo);
    }
}

char *GetTimeNow(){

    time_t t = time(NULL);
    struct tm tiempoLocal = *localtime(&t);
    static char fechaHora[SIZETIME];
    char *formato = "%Y-%m-%d %H:%M:%S";

    int bytesEscritos = strftime(fechaHora, sizeof fechaHora, formato, &tiempoLocal);

    return fechaHora;
}

