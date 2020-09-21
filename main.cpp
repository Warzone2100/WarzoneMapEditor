#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "glad/glad.h"
#include <SDL2/SDL.h>

#include "mypng.h"
#include "log.hpp"
#include "myshader.h"
#include "pie.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

int width = 640;
int height = 480;

const char* demopieobjectpath = "./vtolfactory_module1.pie";

void RenderPIE(struct PIEobject o, float rx, float ry, float rz) {
	glTranslatef(0.0f, 0.0f, -35.0f);
	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glRotatef(ry, 0.0f, 1.0f, 0.0f);
	glRotatef(rz, 0.0f, 0.0f, 1.0f);
	glBegin(GL_TRIANGLES);
	//glCullFace(GL_FRONT);
	for(int i=0; i<o.polygonscount; i++) {
		if(o.polygons[i].pcount != 3) {
			log_error("Draw panic!");
			abort();
		}
		for(int j=0; j<o.polygons[i].pcount; j++) {
			glColor3f(rand()%255/(float)255, rand()%255/(float)255, rand()%255/(float)255);
			glVertex3f(o.points[o.polygons[i].porder[j]].x/16, o.points[o.polygons[i].porder[j]].y/16, o.points[o.polygons[i].porder[j]].z/16);
			log_info("%f %f %f", o.points[o.polygons[i].porder[j]].x/16, o.points[o.polygons[i].porder[j]].y/16, o.points[o.polygons[i].porder[j]].z/16);
		}
	}
	glEnd();
}

int main(int argc, char** argv) {
	time_t t;
	srand((unsigned) time(&t));
	log_info("Hello world!");
	PIEobject m = ReadPIE((char*)demopieobjectpath);

	glfwInit();
	log_info("glfw init done");

	if(SDL_Init(SDL_INIT_EVERYTHING)<0) {
		log_fatal("SDL init error: %s", SDL_GetError());
	}

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

	float vertices[] = {
		// FIRST TRIANGLE OF MODEL
        // 0.0f, 1.0f, -0.5f, // left
        // 0.0f, 0.0f, -0.5f, // right
        // 0.0f, 0.0f, -0.25f,  // top

		// EXAMPLE VERTICES
        -0.5f, -0.5f, 0.0f, // left
         0.5f, -0.5f, 0.0f, // right
         0.0f,  0.5f, 0.0f  // top
    };

	mshader shad("vertex.vs", "fragment.frag");
	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glm::mat4 trans = glm::mat4(1.0f);
	// trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
	// trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));
	unsigned int transformLoc = glGetUniformLocation(shad.program, "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));

	printf("A: %.3f, %.3f, %.3f\n", m.points[0].x, m.points[0].y, m.points[0].z);
	printf("B: %.3f, %.3f, %.3f\n", m.points[1].x, m.points[1].y, m.points[1].z);
	printf("C: %.3f, %.3f, %.3f\n", m.points[2].x, m.points[2].y, m.points[2].z);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	bool r=1;
	SDL_Event ev;
	while(r==1) {
		while(SDL_PollEvent(&ev)) {
			switch(ev.type) {
				case SDL_QUIT:
				r = 0;
				break;

				case SDL_KEYDOWN:
				switch(ev.key.keysym.sym) {
				}
				break;
			}
		}

		//trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shad.use();
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, m.GLvertexesCount);

		glFlush();
		SDL_GL_SwapWindow(window);
	}

	FreePIE(&m);
	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
