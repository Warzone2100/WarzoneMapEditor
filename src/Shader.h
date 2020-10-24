#ifndef MYSHADER_H
#define MYSHADER_H

#include "glad/glad.h"
#include <GL/gl.h>
#include <GL/glu.h>

class Shader {
public:
	unsigned int program;
	Shader(const char* vp, const char* fp);
	~Shader();
	void use();
};
#endif
