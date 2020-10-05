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
#include "imgui/ImGuiFileDialog.h"

#include "log.hpp"
#include "myshader.h"
#include "pie.h"
#include "World3d.h"
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

	Object3d obj;
	obj.LoadFromPIE(demopieobjectpath);
	Texture tex;
	tex.Load(obj.TexturePath, rend);
	tex.Bind(0);
	obj.UsingTexture = &tex;
	obj.PrepareTextureCoords();
	mshader shad("vertex.vs", "fragment.frag");
	obj.BufferData(shad.program);

	glm::mat4 Projection = glm::perspective(glm::radians(70.0f), (float) width / (float)height, 0.1f, 100.0f);
	glm::vec3 cameraPosition(0, 100, 300);
	glm::vec3 cameraRotation(-45, 0, 0);
	auto View =
		glm::scale(glm::mat4(1.0), glm::vec3(0.01f)) *
		glm::translate(glm::mat4(1), -cameraPosition) *
		glm::rotate(glm::mat4(1), glm::radians(-cameraRotation.x), glm::vec3(1, 0, 0)) *
		glm::rotate(glm::mat4(1), glm::radians(-cameraRotation.y), glm::vec3(0, 1, 0)) *
		glm::rotate(glm::mat4(1), glm::radians(-cameraRotation.z), glm::vec3(0, 0, 1)) *
		glm::mat4(1);
	auto viewProjection = Projection * View;

	bool r=1;
	glEnable(GL_DEPTH_TEST);
	SDL_Event ev;
	Uint32 frame_time_start = 0;
	while(r==1) {
		frame_time_start = SDL_GetTicks();
		while(SDL_PollEvent(&ev)) {
			ImGui_ImplSDL2_ProcessEvent(&ev);
			switch(ev.type) {
				case SDL_QUIT:
				r = 0;
				break;

				case SDL_WINDOWEVENT:
				if(ev.window.event == SDL_WINDOWEVENT_RESIZED || ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
					width = ev.window.data1;
					height = ev.window.data2;
					Projection = glm::perspective(glm::radians(70.0f), (float) width / (float)height, 0.1f, 100.0f);
					viewProjection = Projection * View;
				}
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
		static bool FPSlimiter = true;
		ImGui::SetNextWindowPos({0, 0}, 1);
		ImGui::Begin("##bmain", &show_window,   ImGuiWindowFlags_NoMove |
		 										ImGuiWindowFlags_NoResize |
												ImGuiWindowFlags_NoTitleBar |
												ImGuiWindowFlags_NoResize |
												ImGuiWindowFlags_NoCollapse |
												ImGuiWindowFlags_AlwaysAutoResize |
												ImGuiWindowFlags_NoBackground);
		// if(ImGui::Button("Load object")) {
		// 	igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".pie", ".");
		// }
		// if (igfd::ImGuiFileDialog::Instance()->FileDialog("ChooseFileDlgKey")) {
		// 	if (igfd::ImGuiFileDialog::Instance()->IsOk == true) {
		// 		unsigned long long load_start = SDL_GetTicks();
		// 		std::string filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
		// 		std::string filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath();
		// 		PIEobject* nobjects = (PIEobject*)realloc(objects, (objectsCount+1)*sizeof(PIEobject));
		// 		if(nobjects == NULL) {
		// 			log_fatal("Memeory realloc failed!");
		// 		}
		// 		objects = nobjects;
		// 		objects[objectsCount] = ReadPIE((char*)filePathName.c_str(), rend);
		// 		PIEreadTexture(&objects[objectsCount], rend);
		// 		PIEprepareGLarrays(&objects[objectsCount]);
		// 		unsigned int* ntextures = (unsigned int*)realloc(textures, (objectsCount+1)*sizeof(unsigned int));
		// 		if(ntextures == NULL) {
		// 			log_fatal("Memeory realloc failed!");
		// 		}
		// 		textures = ntextures;
		// 		glGenTextures(1, &textures[objectsCount]);
		// 		glActiveTexture(GL_TEXTURE0+objectsCount);
		// 		PIEbindTexpage(&objects[objectsCount]);
		// 		objectsCount++;
		// 		log_info("Object loading complete at %ldms", SDL_GetTicks()-load_start);
		// 	}
		// 	igfd::ImGuiFileDialog::Instance()->CloseDialog("ChooseFileDlgKey");
		// }
		// ImGui::SliderInt("Object", &editobject, 0, objectsCount-1);
		ImGui::Checkbox("Textures", &ShowTextures);
		ImGui::Checkbox("Fps limit", &FPSlimiter);
		ImGui::Text("%.3f (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();

		shad.use();

		glUniformMatrix4fv(glGetUniformLocation(shad.program, "ViewProjection"), 1, GL_FALSE, glm::value_ptr(viewProjection));

		// for(int i=0; i<objectsCount; i++) {
		// 	glm::vec3 pos = {objects[i].GLpos[0], objects[i].GLpos[1], objects[i].GLpos[2]};
		// 	auto Model =
		// 		glm::translate(glm::mat4(1), -pos) *
		// 		glm::rotate(glm::mat4(1), glm::radians(-objects[i].GLrot[0]), glm::vec3(1, 0, 0)) *
		// 		glm::rotate(glm::mat4(1), glm::radians(-objects[i].GLrot[1]), glm::vec3(0, 1, 0)) *
		// 		glm::rotate(glm::mat4(1), glm::radians(-objects[i].GLrot[2]), glm::vec3(0, 0, 1)) *
		// 		glm::mat4(1);
		// 	glUniform1i(glGetUniformLocation(shad.program, "Texture"), i);
		// 	glUniformMatrix4fv(glGetUniformLocation(shad.program, "Model"), 1, GL_FALSE, glm::value_ptr(Model));
		// 	if(ShowTextures) {
		// 		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		// 	} else {
		// 		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		// 	}
		// 	glBindVertexArray(VAO_vertices[i]);
		// 	glBindBuffer(GL_ARRAY_BUFFER, VBO_vertices[i]);
		// 	glDrawArrays(GL_TRIANGLES, 0, objects[i].GLvertexesCount);
		// 	glFlush();
		//
		// }
		obj.Render(shad.program);
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
