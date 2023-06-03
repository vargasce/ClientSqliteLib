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

Log *CreateLog() {
    Log* log = (Log*) malloc(sizeof(Log));
    log->information = informationFunc;
    log->error = errorFunc;
    log->warning = warningFunc;
    log->debug = debugFunc;
    return log;
}

void informationFunc(const char* message) {
    printf("[Information] %s\n", message);
}

void errorFunc(const char* message) {
    fprintf(stderr, "[Error] %s \n", message);
}

void warningFunc(const char* message) {
    printf("[Warning] %s\n", message);
}

void debugFunc(const char* message) {
    printf("[Debug] %s\n", message);
}
