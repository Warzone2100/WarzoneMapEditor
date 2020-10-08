#ifndef OTHER_H_DEFINED
#define OTHER_H_DEFINED

#include <stdarg.h>

size_t snprcat(char* str, size_t stroffs, size_t strmax, const char* format, ...);
char* sprcatr(char* str, const char* format, ...);
bool equalstr(char* s1, const char* s2)

#endif /* end of include guard: OTHER_H_DEFINED */
