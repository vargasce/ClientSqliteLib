#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

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
Log *CreateLog();

#endif // LOG_H_INCLUDED
