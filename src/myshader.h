#ifndef MYSHADER_H
#define MYSHADER_H

#include "glad/glad.h"
#include <GL/gl.h>
#include <GL/glu.h>

class mshader {
	public:
		unsigned int program;
		mshader(const char* vp, const char* fp);
		void use();
};
#endif
