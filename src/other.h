/*
    This file is part of WZ2100 Map Editor.
    Copyright (C) 2020-2021  maxsupermanhd
    Copyright (C) 2020-2021  bjorn-ali-goransson

    WZ2100 Map Editor is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    WZ2100 Map Editor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with WZ2100 Map Editor; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef OTHER_H_DEFINED
#define OTHER_H_DEFINED

#include <stdarg.h>
#include <stddef.h>
#include "glad/glad.h"
#include <string>
#include <string.h>

size_t snprcat(char* str, size_t stroffs, size_t strmax, const char* format, ...);
char* sprcatr(char* str, const char* format, ...);
bool equalstr(char* s1, const char* s2);
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam );
void print_configuration();

inline int strncmpl(std::string a, const char* b) {
    return strncmp(a.c_str(), b, strlen(b));
}
inline int strncmpl(char* a, char* b) {
    return strncmp(a, b, strlen(b));
}
inline int strncmpl(char* a, const char* b) {
    return strncmp(a, b, strlen(b));
}
inline int strncmpl(const char* a, char* b) {
    return strncmp(a, b, strlen(b));
}
inline int strncmpl(const char* a, const char* b) {
    return strncmp(a, b, strlen(b));
}
inline int sscanf(std::string a, const char* format, ...) {
    va_list args;
	va_start(args, format);
    int r = vsscanf(a.c_str(), format, args);
    va_end(args);
    return r;
}

#endif /* end of include guard: OTHER_H_DEFINED */
