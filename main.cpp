#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "glad/glad.h"
#include <SDL2/SDL.h>


#include "mypng.h"
#include "log.hpp"
#include "myshader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

int width = 640;
int height = 480;

const char* demopieobjectpath = "/home/max/warzone2100/data/base/structs/vtolfactory_module1.pie";

struct PIEpoint {
	float x, y, z;
};
struct PIEpolygon {
	int flags;
	int pcount;
	int porder[6];
	float texcoords[12];
};


struct PIEobject {
	struct PIEpoint* points;
	int pointscount;
	struct PIEpolygon* polygons;
	int polygonscount;
	int valid;
	struct mpng texture;
};

struct PIEobject ReadPIE() {
	FILE* f = fopen(demopieobjectpath, "r");
	struct PIEobject o;
	o.valid = 0;
	if(f == NULL) {
		log_error("Error opening file");
		return o;
	}

	int ver, type, dummy, pointscount, ret;
	char texturepagepath[512];
	ret = fscanf(f, "PIE %d\nTYPE %d\nTEXTURE %d %s %d %d\nLEVELS %d\nLEVEL %d\nPOINTS %d\n", &ver, &type, &dummy, texturepagepath, &dummy, &dummy, &dummy, &dummy, &pointscount);
	if(ret!=9) {
		log_error("PIE scanf 1 %d", ret);
		abort();
	}
	o.points = (struct PIEpoint*)malloc(pointscount*sizeof(struct PIEpoint));
	for(int i=0; i<pointscount; i++) {
		ret = fscanf(f, "\t%f %f %f\n", &o.points[i].x, &o.points[i].y, &o.points[i].z);
		if(ret!=3) {
			log_error("PIE scanf 2 %d", ret);
			abort();
		}
		//printf("Point %f %f %f\n", o.points[i].x, o.points[i].y, o.points[i].z);
	}
	o.pointscount = pointscount;

	int polycount;
	ret = fscanf(f, "POLYGONS %d", &polycount);
	if(ret!=1) {
		log_error("PIE scanf 3 %d", ret);
		abort();
	}
	o.polygons = (struct PIEpolygon*)malloc(polycount*sizeof(struct PIEpolygon));
	o.polygonscount = polycount;
	for(int i=0; i<polycount; i++) {
		ret = fscanf(f, "\t%d %d", &o.polygons[i].flags, &o.polygons[i].pcount);
		if(ret!=2) {
			log_error("PIE scanf 4 %d (%d)", ret, i);
			abort();
		}
		for(int j=0; j<o.polygons[i].pcount; j++) {
			ret = fscanf(f, " %d", &o.polygons[i].porder[j]);
			if(ret!=1) {
				log_error("PIE scanf 5 %d (%d) (%d)", ret, i, j);
				abort();
			}
		}
		if(o.polygons[i].flags!=200) {
			log_error("Polygons bad");
			abort();
		}
		for(int j=0; j<o.polygons[i].pcount*2; j++) {
			ret = fscanf(f, " %f", &o.polygons[i].texcoords[j]);
			if(ret!=1) {
				log_error("PIE scanf 6 %d (%d) (%d)\n", ret, i, j);
				abort();
			}
		}
	}
	fclose(f);
	char fbufer[1024] = {0};
	snprintf(fbufer, 1023, "/home/max/warzone2100/data/base/texpages/%s", texturepagepath);
	o.texture = read_png_file(fbufer);
	o.valid = 1;
	return o;
}

void FreePIE(struct PIEobject* o) {
	if(o->valid) {
		free(o->polygons);
		o->polygons = NULL;
		free(o->points);
		o->points = NULL;
	}
	return;
}

void RenderPIE(struct PIEobject o, float rx, float ry, float rz) {
	glTranslatef(0.0f, 0.0f, -35.0f);
	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glRotatef(ry, 0.0f, 1.0f, 0.0f);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);
	glBegin(GL_TRIANGLES);
	//glCullFace(GL_FRONT);
	glColor3f(255, 255, 255);
	for(int i=0; i<o.polygonscount; i++) {
		if(o.polygons[i].pcount != 3) {
			log_error("Draw panic!");
			abort();
		}
		for(int j=0; j<o.polygons[i].pcount; j++) {
			glVertex3f(o.points[o.polygons[i].porder[j]].x/16, o.points[o.polygons[i].porder[j]].y/16, o.points[o.polygons[i].porder[j]].z/16);
		}
	}
	glEnd();
}

int main(int argc, char** argv) {
	time_t t;
	srand((unsigned) time(&t));
	log_info("Hello world!");
	PIEobject m = ReadPIE();
	log_info("Reading done");

	glfwInit();

	if(SDL_Init(SDL_INIT_EVERYTHING)<0) {
		log_fatal("SDL init error: %s", SDL_GetError());
	}
	if(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1)) {log_fatal("attr error");}
	if(SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5)) {log_fatal("attr error");}
	if(SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6)) {log_fatal("attr error");}
	if(SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5)) {log_fatal("attr error");}
	if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3)) {log_fatal("attr error");}

	SDL_Window* window = SDL_CreateWindow("3d", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	SDL_GLContext glcontext = SDL_GL_CreateContext(window);
	if(!glcontext) {
		log_fatal("gl context");
	}
	if(!gladLoadGL()) {
		log_fatal("gladLoadGL failed");
		abort();
	}
	log_info("OpenGL %d.%d", GLVersion.major, GLVersion.minor);
	if(GLAD_GL_EXT_framebuffer_multisample) {
		log_info("Supporting framebuffer multisample");
	}
	if(GLAD_GL_VERSION_3_0) {
		log_info("Supporting 3.0");
	}


	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (float)width/(float)height, 0.1f, 400.0f);
	glMatrixMode(GL_MODELVIEW);

	// glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	// glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	// glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
	// glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	// glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
	// glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
	// glm::mat4 view;
	// view = glm::lookAt( glm::vec3(0.0f, 0.0f, 3.0f),
	// 					glm::vec3(0.0f, 0.0f, 0.0f),
	// 					glm::vec3(0.0f, 1.0f, 0.0f));

	float vertices[] = {
	        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	    };
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

	unsigned int texture1;
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// log_info("%d %d %d %d", m.texture.width, m.texture.height, m.texture.rgbpixels[0], m.texture.rgbpixels[m.texture.width*m.texture.height*3-1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m.texture.width, m.texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m.texture.rgbpixels);
	// log_info("glGenerateMipmap is %p", glGenerateMipmap);
	glGenerateMipmap(GL_TEXTURE_2D);

	mshader shad("vertex.vs", "fragment.frag");
	glUniform1i(glGetUniformLocation(shad.program, "texture1"), 1);
	bool r=1;
	SDL_Event ev;
	float crx = 30.0f, cry = 0.0f, crz = 0.0f;
	float cpx = 0.0f, cpy = 0.0f, cpz = -7.0f;
	while(r==1) {
		while(SDL_PollEvent(&ev)) {
			switch(ev.type) {
				case SDL_QUIT:
				r = 0;
				break;

				case SDL_KEYDOWN:
				switch(ev.key.keysym.sym) {
					case SDLK_w:
					cpx += 1.0f;
					break;
					case SDLK_s:
					cpx -= 1.0f;
					break;
					case SDLK_e:
					cpy += 1.0f;
					break;
					case SDLK_d:
					cpy -= 1.0f;
					break;
					case SDLK_r:
					cpz += 1.0f;
					break;
					case SDLK_f:
					cpz -= 1.0f;
					break;
				}
				break;
			}
		}

		// int mx, my;
		// SDL_GetMouseState(&mx, &my);
		// crx = 360/mx;
		// cry = 360/my;
		// crz = 360/(mx/my)*2;
		// crx += 0.2f;
		cry += 0.4f;
		// crz += 0.6f;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
		shad.use();
		GLfloat radius = 10.0f;
		GLfloat camX = sin(SDL_GetTicks()) * radius;
		GLfloat camZ = cos(SDL_GetTicks()) * radius;
		glm::mat4 view = glm::lookAt(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.0));
		glm::mat4 projection = glm::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 400.0f);
		GLint modelLoc = glGetUniformLocation(shad.program, "model");
		GLint viewLoc = glGetUniformLocation(shad.program, "view");
		GLint projLoc = glGetUniformLocation(shad.program, "projection");
		glm::mat4 model = glm::translate(model, glm::vec3(0.0f, 0.0f, -35.0f));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		glDrawArrays(GL_TRIANGLES, 0, 36);
		//RenderPIE(m, crx, cry, crz);

		glFlush();
		SDL_GL_SwapWindow(window);
	}
	FreePIE(&m);
	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
