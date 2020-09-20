#ifndef MYSHADER_H
#define MYSHADER_H

#include "glad/glad.h"
#include <GL/gl.h>
#include <GL/glu.h>

class mshader {
	public:
		GLuint program;
		mshader(const GLchar* vp, const GLchar* fp);
		void use();
};
#endif
