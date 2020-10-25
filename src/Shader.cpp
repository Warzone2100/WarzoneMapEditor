#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Shader.h"
#include "log.hpp"

Shader::Shader(const GLchar* vp, const GLchar* fp) {
	log_info("Opening files...");
	FILE *vf = fopen(vp, "r"), *ff = fopen(fp, "r");
	if(vf == NULL) {
		log_fatal("Cannot open vertex shader file");
		abort();
	}
	if(ff == NULL) {
		log_fatal("Cannot open fragment shader file");
		abort();
	}
	char* vcode = NULL;
	size_t vcodelen = 0;
	char* fcode = NULL;
	size_t fcodelen = 0;
	size_t readed = 0;
	const size_t nbuflen = 128;
	char nbuf[nbuflen] = {0};
	while((readed = fread(nbuf, 1, nbuflen, vf)) > 0) {
		vcode = (char*)realloc(vcode, vcodelen+readed);
		if(vcode == NULL) {
			log_fatal("Alloc fail");
			abort();
		}
		memcpy(vcode+vcodelen, nbuf, readed);
		vcodelen+=readed;
		memset(nbuf, 0, nbuflen);
	}
	fclose(vf);
	vcode[vcodelen-1] = '\0';
	while((readed = fread(nbuf, 1, nbuflen, ff)) > 0) {
		fcode = (char*)realloc(fcode, fcodelen+readed);
		if(fcode == NULL) {
			log_fatal("Alloc fail");
			abort();
		}
		memcpy(fcode+fcodelen, nbuf, readed);
		fcodelen+=readed;
		memset(nbuf, 0, nbuflen);
	}
	fclose(ff);
	fcode[fcodelen-1] = '\0';
	GLuint v, f;
	GLint s;
	GLchar l[512];
	v = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(v, 1, &vcode, NULL);
	glCompileShader(v);
	glGetShaderiv(v, GL_COMPILE_STATUS, &s);
	if(!s) {
		glGetShaderInfoLog(v, 512, NULL, l);
		log_fatal("Shader compile error: %s", l);
	}
	f = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(f, 1, &fcode, NULL);
	glCompileShader(f);
	glGetShaderiv(f, GL_COMPILE_STATUS, &s);
	if(!s) {
		glGetShaderInfoLog(f, 512, NULL, l);
		log_fatal("Shader compile error: %s", l);
	}
	this->program = glCreateProgram();
	glAttachShader(this->program, v);
	glAttachShader(this->program, f);
	glLinkProgram(this->program);
	glGetProgramiv(this->program, GL_LINK_STATUS, &s);
	if(!s) {
		glGetProgramInfoLog(this->program, 512, NULL, l);
		log_fatal("Shader link error: %s", l);
	}
	glDetachShader(this->program, v);
	glDetachShader(this->program, f);
	glDeleteShader(v);
	glDeleteShader(f);
	log_info("Shader [%s] [%s] loaded.", vp, fp);
	free(vcode);
	free(fcode);
}

void Shader::use() {
	glUseProgram(this->program);
}

Shader::~Shader() {
	glDeleteProgram(this->program);
}
