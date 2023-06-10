#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

// TODO (Tauriel#1#06/10/23): Para log de error crear archivo txt de salida. Alamacenar dicho txt en capeta log creada en initSystem. ... //

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
