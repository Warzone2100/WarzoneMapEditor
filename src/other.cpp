#include "other.h"

#include "log.hpp"
#include <string.h>
#include <stdlib.h>

size_t snprcat(char* str, size_t stroffs, size_t strmax, const char* format, ...) {
	va_list args;
	va_start(args, format);
	size_t newoffs = vsnprintf(str+stroffs, strmax-stroffs-1, format, args) + stroffs;
	va_end(args);
	if(newoffs > strmax - 1) {
		printf("Format turncation warning!\n");
	}
	return newoffs;
}

char* sprcatr(char* str, const char* format, ...) {
	const size_t chunksize = 4096*2;
	size_t origlenth = 0;
	if(str != NULL) {
		origlenth = strlen(str);
	}
	char tmpbuf[chunksize] = {0};
	va_list args;
	va_start(args, format);
	size_t used = vsnprintf(tmpbuf, chunksize-1, format, args);
	va_end(args);
	if(used == chunksize-1) {
		printf("Panic not enouth tmp buffer on sprcatr!\n");
	}
	char* newstr = (char*)realloc(str, origlenth+used+1);
	if(newstr == NULL) {
		printf("No enouth memory! (realloc sprcatr fail)\n");
		return str;
	}
	memcpy(newstr+origlenth, tmpbuf, used);
	newstr[origlenth+used] = '\0';
	return newstr;
}

bool equalstr(char* s1, const char* s2) {
	if(strlen(s1) != strlen(s2))
		return false;
	for(unsigned int i=0; i<strlen(s1); i++) {
		if(s1[i] != s2[i])
			return false;
	}
	return true;
}

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam ) {
	char* debugmsg = sprcatr(NULL, "GL CALLBACK: ");
	switch(source) {
		case GL_DEBUG_SOURCE_API:
		debugmsg = sprcatr(debugmsg, "api\n");
		break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		debugmsg = sprcatr(debugmsg, "window system\n");
		break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
		debugmsg = sprcatr(debugmsg, "shader compiler\n");
		break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:
		debugmsg = sprcatr(debugmsg, "3rd party\n");
		break;
		case GL_DEBUG_SOURCE_APPLICATION:
		debugmsg = sprcatr(debugmsg, "application\n");
		break;
		case GL_DEBUG_SOURCE_OTHER:
		debugmsg = sprcatr(debugmsg, "other\n");
		break;
		default:
		debugmsg = sprcatr(debugmsg, "unknown\n");
		break;
	}
	switch(type) {
		case GL_DEBUG_TYPE_ERROR:
		debugmsg = sprcatr(debugmsg, "Error: ");
		break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		debugmsg = sprcatr(debugmsg, "Depricated: ");
		break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		debugmsg = sprcatr(debugmsg, "Undefined behavior: ");
		break;
		case GL_DEBUG_TYPE_PORTABILITY:
		debugmsg = sprcatr(debugmsg, "Portability: ");
		break;
		case GL_DEBUG_TYPE_PERFORMANCE:
		debugmsg = sprcatr(debugmsg, "Performance: ");
		break;
		case GL_DEBUG_TYPE_MARKER:
		debugmsg = sprcatr(debugmsg, "Type marker: ");
		break;
		case GL_DEBUG_TYPE_PUSH_GROUP:
		debugmsg = sprcatr(debugmsg, "Group push: ");
		break;
		case GL_DEBUG_TYPE_POP_GROUP:
		debugmsg = sprcatr(debugmsg, "Pop group: ");
		break;
		case GL_DEBUG_TYPE_OTHER:
		debugmsg = sprcatr(debugmsg, "Other: ");
		break;
	}
	debugmsg = sprcatr(debugmsg, "[%3d] ", id);
	switch(severity) {
		case GL_DEBUG_SEVERITY_HIGH:
		debugmsg = sprcatr(debugmsg, "HIGH ");
		break;
		case GL_DEBUG_SEVERITY_MEDIUM:
		debugmsg = sprcatr(debugmsg, "Medium ");
		break;
		case GL_DEBUG_SEVERITY_LOW:
		debugmsg = sprcatr(debugmsg, "low ");
		break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
		debugmsg = sprcatr(debugmsg, "warn ");
		break;
	}
	debugmsg = sprcatr(debugmsg, " :::\n[%s]", message);
	log_info(debugmsg);
	free(debugmsg);
}
