#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED
#define SIZETIME 70

#define ARCHIVO "./log/losg.txt"
#include <string.h>
#include <time.h>

typedef struct {
    void (*information)(const char*);
    void (*error)(const char*);
    void (*warning)(const char*);
    void (*debug)(char *stringFormaterDebug, int countFormater, ...);
} Log;

void informationFunc(const char* message);
void errorFunc(const char* message);
void warningFunc(const char* message);
void debugFunc(char *stringFormaterDebug, int counterFormater, ...);
void SaveLog(char *message);
char *GetTimeNow();
Log *CreateLog();

#endif // LOG_H_INCLUDED
