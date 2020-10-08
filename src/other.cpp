#include "other.h"

#include "log.hpp"
#include <string.h>
#include <stdlib.h>

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
