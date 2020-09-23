#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "glad/glad.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

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
	SDL_Renderer* rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
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

		// EXAMPLE VERTICES|TEXTURE COORDS
		-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // left
         0.5f, -0.5f, 0.0f, 1.0f, 1.0f, // right
         0.0f,  0.5f, 0.0f, 0.0f, 0.5f,  // top
		// glm::vec3(m.points[0].x, m.points[0].y, m.points[0].z),
		// glm::vec3(m.points[1].x, m.points[1].y, m.points[1].z),
		// glm::vec3(m.points[2].x, m.points[2].y, m.points[2].z),
    };

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned int texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture

	SDL_Texture* newTexture = NULL;
	SDL_Surface* loadedSurf = IMG_Load("page-13-player-buildings.png");
	if(loadedSurf==NULL) {
		printf("[LoadTexture] Loading error: %s\n", IMG_GetError());
	} else {
		//SDL_SetColorKey(loadedSurf, SDL_TRUE, SDL_MapRGB(loadedSurf->format, 128, 112, 119));
		newTexture = SDL_CreateTextureFromSurface(rend, loadedSurf);
		if(newTexture == NULL) {
			printf("[LoadTexture] Converting error: %s\n", IMG_GetError());
		}
		SDL_FreeSurface(loadedSurf);
	}
	float texw, texh;
	SDL_GL_BindTexture(newTexture, &texw, &texh);
	log_info("Tex size: %f %f", texw, texh);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m.texture.width, m.texture.height, 0, GL_RGB, GL_UNSIGNED_BYTE, m.texture.rgbpixels);
	glGenerateMipmap(GL_TEXTURE_2D);

	mshader shad("vertex.vs", "fragment.frag");

	unsigned int VBO_vertices, VAO_vertices;
	glGenVertexArrays(1, &VAO_vertices);
	glGenBuffers(1, &VBO_vertices);
	glBindVertexArray(VAO_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_vertices);
	// glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, m.GLvertexesCount, m.GLvertexes, GL_STATIC_DRAW);

	glVertexAttribPointer(glGetAttribLocation(shad.program, "Coordinates"), 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(glGetAttribLocation(shad.program, "Coordinates"));
	glVertexAttribPointer(glGetAttribLocation(shad.program, "TexCoordinates"), 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(glGetAttribLocation(shad.program, "TexCoordinates"));

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

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shad.use();

		glUniform1i(glGetUniformLocation(shad.program, "Texture"), 0);
		glm::mat4 matrix1 = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f/200));
		glm::mat4 matrix2 = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0));
		glm::mat4 matrix3 = glm::rotate(glm::mat4(1.0f), glm::radians(40.0f), glm::vec3(0, 0, 1));
		glUniformMatrix4fv(glGetUniformLocation(shad.program, "Transform"), 1, GL_FALSE, glm::value_ptr(matrix1*matrix2));

		// glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
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
