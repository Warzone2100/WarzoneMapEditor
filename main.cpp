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
const char* demopieobjectpath2 = "./vtolfactory_module1.pie";

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

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int objectsCount = 2;
	PIEobject* objects = (PIEobject*)malloc(objectsCount*sizeof(PIEobject));

	objects[0] = ReadPIE((char*)demopieobjectpath, rend);
	objects[1] = ReadPIE((char*)demopieobjectpath2, rend);
	for(int i=0; i<objectsCount; i++) {
		PIEreadTexture(&objects[i], rend);
		PIEprepareGLarrays(&objects[i]);
	}

	unsigned int* textures = (unsigned int*)malloc(objectsCount*sizeof(unsigned int));
	glGenTextures(objectsCount, textures);
	for(int i=0; i<objectsCount; i++) {
		glActiveTexture(GL_TEXTURE0+i);
		PIEbindTexpage(&objects[i]);
	}

	mshader shad("vertex.vs", "fragment.frag");


	unsigned int* VBO_vertices = (unsigned int*)malloc(objectsCount*sizeof(unsigned int));
	unsigned int* VAO_vertices = (unsigned int*)malloc(objectsCount*sizeof(unsigned int));
	glGenVertexArrays(objectsCount, VAO_vertices);
	glGenBuffers(objectsCount, VAO_vertices);
	for(int i=0; i<objectsCount; i++) {
		glBindVertexArray(VAO_vertices[i]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_vertices[i]);
		glBufferData(GL_ARRAY_BUFFER, objects[i].GLvertexesCount, objects[i].GLvertexes, GL_STATIC_DRAW);
		glVertexAttribPointer(glGetAttribLocation(shad.program, "Coordinates"), 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(glGetAttribLocation(shad.program, "Coordinates"));
		glVertexAttribPointer(glGetAttribLocation(shad.program, "TexCoordinates"), 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(glGetAttribLocation(shad.program, "TexCoordinates"));
	}

	bool r=1;
	glEnable(GL_DEPTH_TEST);
	SDL_Event ev;
	Uint32 frame_time_start = 0;
	glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
	float yaw = 0.0f, pitch = 0.0f, roll = 0.0f;
	float fov = 45.0f;
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

		static bool show_window = true;
		static bool ShowTextures = true;
		static int editobject = 0;
		static float RCamX = 0.0f;
		static bool FPSlimiter = true;
		ImGui::SetNextWindowPos({0, 0}, 1);
		ImGui::Begin("##bmain", &show_window,   ImGuiWindowFlags_NoMove |
		 										ImGuiWindowFlags_NoResize |
												ImGuiWindowFlags_NoTitleBar |
												ImGuiWindowFlags_NoResize |
												ImGuiWindowFlags_NoCollapse |
												ImGuiWindowFlags_AlwaysAutoResize |
												ImGuiWindowFlags_NoBackground);
		ImGui::SliderInt("Object", &editobject, 0, objectsCount-1);
		ImGui::SliderFloat("Scale", &objects[editobject].GLscale, 0.0f, 4.0f);
		ImGui::SliderFloat("RotX", &objects[editobject].GLrot[0], 0.0f, 360.0f);
		ImGui::SliderFloat("RotY", &objects[editobject].GLrot[1], 0.0f, 360.0f);
		ImGui::SliderFloat("RotZ", &objects[editobject].GLrot[2], 0.0f, 360.0f);
		ImGui::SliderFloat("PosX", &objects[editobject].GLpos[0], -360.0f, 360.0f);
		ImGui::SliderFloat("PosY", &objects[editobject].GLpos[1], -360.0f, 360.0f);
		ImGui::SliderFloat("PosZ", &objects[editobject].GLpos[2], -360.0f, 360.0f);
		ImGui::SliderFloat("CamX", &RCamX, -360.0f, 360.0f);
		ImGui::SliderFloat("Fov", &fov, -360.0f, 360.0f);
		ImGui::Checkbox("Textures", &ShowTextures);
		ImGui::Checkbox("Fps limit", &FPSlimiter);
		ImGui::Text("%.3f (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();

		shad.use();


		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(direction);

		const float radius = 10.0f;
		float camX = sin(glm::radians(RCamX)) * radius;
		float camZ = cos(glm::radians(RCamX)) * radius;
		glm::mat4 view;
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		// view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		glUniformMatrix4fv(glGetUniformLocation(shad.program, "View"), 1, GL_FALSE, glm::value_ptr(view));

		glm::mat4 projection = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);
		glUniformMatrix4fv(glGetUniformLocation(shad.program, "Projection"), 1, GL_FALSE, glm::value_ptr(projection));

		for(int i=0; i<objectsCount; i++) {
			glUniform1i(glGetUniformLocation(shad.program, "Texture"), i);
			glm::mat4 matrixS = glm::scale(glm::mat4(1.0), glm::vec3(objects[i].GLscale/200));
			glm::mat4 matrixRX = glm::rotate(glm::mat4(1.0f), glm::radians(objects[i].GLrot[0]), glm::vec3(1, 0, 0));
			glm::mat4 matrixRY = glm::rotate(glm::mat4(1.0f), glm::radians(objects[i].GLrot[1]), glm::vec3(0, 1, 0));
			glm::mat4 matrixRZ = glm::rotate(glm::mat4(1.0f), glm::radians(objects[i].GLrot[2]), glm::vec3(0, 0, 1));
			glm::mat4 matrixM = glm::translate(glm::mat4(1.0f), glm::vec3(objects[i].GLpos[0], objects[i].GLpos[1], objects[i].GLpos[2]));
			glUniformMatrix4fv(glGetUniformLocation(shad.program, "Transform"), 1, GL_FALSE, glm::value_ptr(matrixS*matrixM*matrixRX*matrixRY*matrixRZ));
			if(ShowTextures) {
				glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
			} else {
				glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			}
			glBindVertexArray(VAO_vertices[i]);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_vertices[i]);
			glDrawArrays(GL_TRIANGLES, 0, objects[i].GLvertexesCount);
			glFlush();

		}

		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);

		if((Uint32)1000/FPS > SDL_GetTicks()-frame_time_start && FPSlimiter) {
			SDL_Delay(1000/FPS-(SDL_GetTicks()-frame_time_start));
		}
	}

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
