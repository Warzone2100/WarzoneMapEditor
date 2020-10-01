#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include "glad/glad.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "log.hpp"
#include "myshader.h"
#include "pie.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

#define FPS 60

int width = 640;
int height = 480;

const char* demopieobjectpath = "./blbrbgen.pie";

int main(int argc, char** argv) {
	time_t t;
	srand((unsigned) time(&t));
	log_info("Hello world!");

	glfwInit();
	log_info("glfw init done");

	if(SDL_Init(SDL_INIT_EVERYTHING)<0) {
		log_fatal("SDL init error: %s", SDL_GetError());
	}
	if(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1)) {log_fatal("attr error");}
	if(SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5)) {log_fatal("attr error");}
	if(SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6)) {log_fatal("attr error");}
	if(SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5)) {log_fatal("attr error");}
	if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3)) {log_fatal("attr error");}

	SDL_Window* window = SDL_CreateWindow("3d", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	SDL_SetWindowResizable(window, SDL_TRUE);
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


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(window, glcontext);
	const char* glsl_version = "#version 130";
	ImGui_ImplOpenGL3_Init(glsl_version);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0);
	glShadeModel(GL_SMOOTH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (float)width/(float)height, 0.1f, 400.0f);
	glMatrixMode(GL_MODELVIEW);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	PIEobject m = ReadPIE((char*)demopieobjectpath, rend);
	PIEreadTexture(&m, rend);
	PIEprepareGLarrays(&m);

	unsigned int texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	PIEbindTexpage(&m);

	mshader shad("vertex.vs", "fragment.frag");
	unsigned int VBO_vertices, VAO_vertices;
	glGenVertexArrays(1, &VAO_vertices);
	glGenBuffers(1, &VBO_vertices);
	glBindVertexArray(VAO_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_vertices);
	glBufferData(GL_ARRAY_BUFFER, m.GLvertexesCount, m.GLvertexes, GL_STATIC_DRAW);

	glVertexAttribPointer(glGetAttribLocation(shad.program, "Coordinates"), 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(glGetAttribLocation(shad.program, "Coordinates"));
	glVertexAttribPointer(glGetAttribLocation(shad.program, "TexCoordinates"), 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(glGetAttribLocation(shad.program, "TexCoordinates"));

	bool r=1;
	glEnable(GL_DEPTH_TEST);
	SDL_Event ev;
	int numFrames = 0;
	Uint32 frame_time_start = 0;
	Uint32 startclock = 0;
	Uint32 deltaclock = 0;
	double fps = 0;
	while(r==1) {
		frame_time_start = SDL_GetTicks();
		while(SDL_PollEvent(&ev)) {
			ImGui_ImplSDL2_ProcessEvent(&ev);
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
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window);
		ImGui::NewFrame();

		static float Scale = 1.0f;
		static float RotX = 0.0f;
		static float RotY = 115.052f;
        static float RotZ = 350.722f;
		static float PosX = 66.804f;
		static float PosY = -77.938f;
        static float PosZ = 0.0f;
		static bool show_window = true;
		static bool ShowTextures = true;

		ImGui::SetNextWindowPos({0, 0}, 1);
		ImGui::Begin("##bmain", &show_window,   ImGuiWindowFlags_NoMove |
		 										ImGuiWindowFlags_NoResize |
												ImGuiWindowFlags_NoTitleBar |
												ImGuiWindowFlags_NoResize |
												ImGuiWindowFlags_NoCollapse |
												ImGuiWindowFlags_AlwaysAutoResize |
												ImGuiWindowFlags_NoBackground);
		ImGui::SliderFloat("Scale", &Scale, 0.0f, 4.0f);
		ImGui::SliderFloat("RotX", &RotX, 0.0f, 360.0f);
		ImGui::SliderFloat("RotY", &RotY, 0.0f, 360.0f);
		ImGui::SliderFloat("RotZ", &RotZ, 0.0f, 360.0f);
		ImGui::SliderFloat("PosX", &PosX, -360.0f, 360.0f);
		ImGui::SliderFloat("PosY", &PosY, -360.0f, 360.0f);
		ImGui::SliderFloat("PosZ", &PosZ, -360.0f, 360.0f);
		ImGui::Checkbox("Textures", &ShowTextures);
		ImGui::Text("%.3f (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();

		shad.use();

		glUniform1i(glGetUniformLocation(shad.program, "Texture"), 0);

		glm::mat4 matrixS = glm::scale(glm::mat4(1.0), glm::vec3(Scale/200));
		glm::mat4 matrixRX = glm::rotate(glm::mat4(1.0f), glm::radians(RotX), glm::vec3(1, 0, 0));
		glm::mat4 matrixRY = glm::rotate(glm::mat4(1.0f), glm::radians(RotY), glm::vec3(0, 1, 0));
		glm::mat4 matrixRZ = glm::rotate(glm::mat4(1.0f), glm::radians(RotZ), glm::vec3(0, 0, 1));
		glm::mat4 matrixM = glm::translate(glm::mat4(1.0f), glm::vec3(PosX,PosY,PosZ));
		glUniformMatrix4fv(glGetUniformLocation(shad.program, "Transform"), 1, GL_FALSE, glm::value_ptr(matrixS*matrixRX*matrixRY*matrixRZ*matrixM));
		if(ShowTextures) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		} else {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		glDrawArrays(GL_TRIANGLES, 0, m.GLvertexesCount);

		glFlush();
		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);
		if((Uint32)1000/FPS > SDL_GetTicks()-frame_time_start) {
			SDL_Delay(1000/FPS-(SDL_GetTicks()-frame_time_start));
		}
		deltaclock = SDL_GetTicks() - startclock;
		startclock = SDL_GetTicks();
		if(numFrames == FPS) {
			if ( deltaclock != 0 )
				fps = 1000 / deltaclock;
			numFrames=0;
		}
		numFrames++;
	}

	FreePIE(&m);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
