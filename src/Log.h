#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

#define ARCHIVO "./log/losg.txt"
#include <string.h>
#include <time.h>

typedef struct {
    void (*information)(const char*);
    void (*error)(const char*);
    void (*warning)(const char*);
    void (*debug)(const char*);
} Log;

void informationFunc(const char* message);
void errorFunc(const char* message);
void warningFunc(const char* message);
void debugFunc(const char* message);
void SaveLog(char *message);
char *GetTimeNow();
Log *CreateLog();

#endif // LOG_H_INCLUDED
