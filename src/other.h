#ifndef OTHER_H_DEFINED
#define OTHER_H_DEFINED

#include <stdarg.h>
#include <stddef.h>
#include "glad/glad.h"

size_t snprcat(char* str, size_t stroffs, size_t strmax, const char* format, ...);
char* sprcatr(char* str, const char* format, ...);
bool equalstr(char* s1, const char* s2);
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam );

#endif /* end of include guard: OTHER_H_DEFINED */
