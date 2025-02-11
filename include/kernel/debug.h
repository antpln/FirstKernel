#ifndef DEBUG_H
#define DEBUG_H

#include <stdnoreturn.h>

void panic(const char* message);
void debug(const char* message);
void success(const char* message);
void error(const char* message);
void test(const char* message);


#endif
