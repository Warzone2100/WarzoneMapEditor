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
