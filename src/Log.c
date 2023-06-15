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
#define SIZETIME 70

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

void debugFunc(const char* message) {
    char *messageSend = (char *) malloc(sizeof(char) * (strlen(message) + strlen("[Debug] ") + SIZETIME) );
    char *timeNow = GetTimeNow();
    sprintf(messageSend,"[Debug] [%s] %s \n",timeNow, message);
    printf("[Debug] %s\n", messageSend);
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

