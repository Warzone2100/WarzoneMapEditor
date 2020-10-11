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
#include "ImGuiFileDialog.h"

#include "wmt.hpp"

#include "log.hpp"
#include "myshader.h"
#include "pie.h"
#include "World3d.h"
#include "terrain.h"
#include "args.h"
#include "other.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

#define FPS 60

int width = 640;
int height = 480;

const char* demopieobjectpath = "./data/blbrbgen.pie";
const char* demopieobjectpath2 = "./data/vtolfactory_module1.pie";

int main(int argc, char** argv) {
	log_set_level(2);
	ProcessArgs(argc, argv);
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
	log_info("%s", SDL_GetError());
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
	glEnable              ( GL_DEBUG_OUTPUT );
	glDebugMessageCallback( MessageCallback, 0 );

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(window, glcontext);
	const char* glsl_version = "#version 130";
	ImGui_ImplOpenGL3_Init(glsl_version);

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClearDepth(0.0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	Object3d obj;
	obj.LoadFromPIE(demopieobjectpath);
	Texture tex;
	tex.Load("./data/"+obj.TexturePath, rend);
	mshader shad2("./data/vertex.vs", "./data/fragment.frag");
	mshader shad("./data/vertex.vs", "./data/fragment.frag");
	obj.UsingTexture = &tex;
	obj.PrepareTextureCoords();
	Terrain ter;
	WZmap map;
	WMT_ReadMap(secure_getenv("OPENMAP")?:(char*)"./data/8c-Stone-Jungle-E.wz", &map);
	ter.GetHeightmapFromMWT(&map);
	ter.CreateTexturePage(secure_getenv("TEXPAGES_PATH")?:(char*)"./data/texpages/", 128, rend);
	ter.UpdateTexpageCoords();
	ter.BufferData(shad.program);
	obj.BufferData(shad2.program);

	glm::vec3 cameraPosition(0, 2000, 1000);
	glm::vec3 cameraRotation(-45, 0, 0);
	glm::vec3 cameraVelocity = {0, 0, 0};
	float cameraSpeed = 2.0f;
	glm::mat4 viewProjection;
	glm::ivec2 cameraMapPosition;
	float cameraFOV = 75.0f;
	glEnablei(GL_BLEND, 0);

	// glm::ivec3 tileScreenCoords[256][256];
	auto cameraUpdate = [&] () {
		cameraMapPosition.x = glm::clamp((int)(map_coord(cameraPosition.x)), 0, ter.w);
		cameraMapPosition.y = glm::clamp((int)(map_coord(cameraPosition.z)), 0, ter.h);
		// for(int y = 0; y < ter.h; y++) {
		// 	for(int x = 0; x < ter.w; x++) {
		// 		auto projectedPosition = glm::vec4(viewProjection * glm::vec4(world_coord(x), world_coord(ter.tileHeight[y][x]), world_coord(y), 1.f));
		// 		const float xx = projectedPosition.x / projectedPosition.w;
		// 		const float yy = projectedPosition.y / projectedPosition.w;
		// 		int screenX = (.5 + .5 * xx) * width;
		// 		int screenY = (.5 - .5 * yy) * height;
		// 		int screenZ = projectedPosition.w;
		//
		// 		tileScreenCoords[x][y] = glm::ivec3(screenX, screenY, screenZ);
		// 	}
		// }
		// printf("position of %i, %i, %i : %i, %i (%i)\n", 0, 0, ter.tileHeight[0][0], tileScreenCoords[0][0].x, tileScreenCoords[0][0].y, tileScreenCoords[0][0].z);

		viewProjection = glm::perspective(glm::radians(cameraFOV), (float) width / (float)height, 30.0f, 100000.0f) *
			glm::rotate(glm::mat4(1), glm::radians(-cameraRotation.x), glm::vec3(1, 0, 0)) *
			glm::rotate(glm::mat4(1), glm::radians(-cameraRotation.y), glm::vec3(0, 1, 0)) *
			glm::rotate(glm::mat4(1), glm::radians(-cameraRotation.z), glm::vec3(0, 0, 1)) *
			glm::translate(glm::mat4(1), -cameraPosition) *
			glm::mat4(1);
	};
	cameraUpdate();
	int CursorTrapWasX, CursorTrapWasY;
	bool cursorTrapped = false;

	bool r=1;
	int showtexture = 0;
	glEnable(GL_DEPTH_TEST);
	SDL_Event ev;
	Uint32 frame_time_start = 0;
	log_info("Entering render loop...");
	bool ShowTextureDebugger = true;
	int TextureDebuggerTriangleX = 0;
	int TextureDebuggerTriangleY = 0;
	char* mask = NULL;
	for(int y=0; y<15; y++) {
		for(int x=0; x<15; x++) {
			mask = sprcatr(mask, "%c", ter.tiles[x][y].rot+'0');
		}
		mask = sprcatr(mask, "\n");
	}
	while(r==1) {
		frame_time_start = SDL_GetTicks();
		while(SDL_PollEvent(&ev)) {
			ImGui_ImplSDL2_ProcessEvent(&ev);
			switch(ev.type) {
				case SDL_QUIT:
				r = 0;
				break;

				case SDL_MOUSEMOTION:
				if(cursorTrapped) {
					cameraRotation.x -= ev.motion.yrel/2;
					cameraRotation.y -= ev.motion.xrel/2;
					cameraUpdate();
				}
				break;

				case SDL_MOUSEWHEEL:
				cameraFOV += ev.wheel.y*4;
				break;

				case SDL_WINDOWEVENT:
				if(ev.window.event == SDL_WINDOWEVENT_RESIZED || ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
					width = ev.window.data1;
					height = ev.window.data2;
					cameraUpdate();
				}
				break;
				case SDL_KEYDOWN:
				switch(ev.key.keysym.sym) {
					case SDLK_ESCAPE:
					r = 0;
					break;
					case SDLK_w:
					cameraVelocity.z = -8;
					break;
					case SDLK_a:
					cameraVelocity.x = -8;
					break;
					case SDLK_e:
					cameraVelocity.y = 8;
					break;
					case SDLK_s:
					cameraVelocity.z = 8;
					break;
					case SDLK_d:
					cameraVelocity.x = 8;
					break;
					case SDLK_q:
					cameraVelocity.y = -8;
					break;
					case SDLK_SPACE:
					if(!cursorTrapped) {
						SDL_SetRelativeMouseMode(SDL_TRUE);
						SDL_GetMouseState(&CursorTrapWasX, &CursorTrapWasY);
					}
					cursorTrapped = true;
					break;
					default:
					break;
				}
				break;
				case SDL_KEYUP:
				switch(ev.key.keysym.sym) {
					case SDLK_w:
					cameraVelocity.z = 0;
					break;
					case SDLK_a:
					cameraVelocity.x = 0;
					break;
					case SDLK_e:
					cameraVelocity.y = 0;
					break;
					case SDLK_s:
					cameraVelocity.z = 0;
					break;
					case SDLK_d:
					cameraVelocity.x = 0;
					break;
					case SDLK_q:
					cameraVelocity.y = 0;
					break;
					case SDLK_SPACE:
					cursorTrapped = false;
					SDL_SetRelativeMouseMode(SDL_FALSE);
					SDL_WarpMouseInWindow(window, CursorTrapWasX, CursorTrapWasY);
					log_info("Cursor restored at %d:%d", CursorTrapWasX, CursorTrapWasY);
					break;
					default:
					break;
				}
				break;
				break;
			}
		}

		if(cameraVelocity.x != 0 || cameraVelocity.y != 0 || cameraVelocity.z != 0){
			cameraPosition.x += glm::sin(glm::radians(cameraRotation.y))*cameraSpeed*cameraVelocity.z;
			// cameraPosition.y -= glm::sin(glm::radians(cameraRotation.x))*cameraSpeed*cameraVelocity.z;
			cameraPosition.z += glm::cos(glm::radians(cameraRotation.y))*cameraSpeed*cameraVelocity.z;
			cameraPosition.y += cameraSpeed*cameraVelocity.y;
			cameraPosition.x += glm::cos(glm::radians(cameraRotation.y))*cameraSpeed*cameraVelocity.x;
			cameraPosition.z -= glm::sin(glm::radians(cameraRotation.y))*cameraSpeed*cameraVelocity.x;
		}
		cameraUpdate();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window);
		ImGui::NewFrame();

		static bool show_window = true;
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
		ImGui::Checkbox("Fps limit", &FPSlimiter);
		ImGui::Checkbox("Wireframe", &ter.FillTextures);
		ImGui::Text("%.3f (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Text("Cam map pos: %3d %3d", cameraMapPosition.x, cameraMapPosition.y);
		ImGui::Text("Cam fov: %f", cameraFOV);
		if(ImGui::Button("Texture assigner")) {
			ShowTextureDebugger = true;
		}
		ImGui::End();
		ImGui::Begin("Texture Viewer", &ShowTextureDebugger, 0);
		ImGui::InputInt("TileX", &TextureDebuggerTriangleX, 1, 1);
		ImGui::InputInt("TileY", &TextureDebuggerTriangleY, 1, 1);
		int GLVpos = (TextureDebuggerTriangleY*ter.w + TextureDebuggerTriangleX)*30;
		ImGui::InputFloat2("V0", &ter.GLvertexes[GLVpos+0 +3], "%f");
		ImGui::InputFloat2("V1", &ter.GLvertexes[GLVpos+5 +3], "%f");
		ImGui::InputFloat2("V2", &ter.GLvertexes[GLVpos+10+3], "%f");
		ImGui::InputFloat2("V3", &ter.GLvertexes[GLVpos+15+3], "%f");
		ImGui::InputFloat2("V4", &ter.GLvertexes[GLVpos+20+3], "%f");
		ImGui::InputFloat2("V5", &ter.GLvertexes[GLVpos+25+3], "%f");
		ImGui::Value("Flip", ter.tiles[TextureDebuggerTriangleX][TextureDebuggerTriangleY].triflip);
		ImGui::Value("Rot", ter.tiles[TextureDebuggerTriangleX][TextureDebuggerTriangleY].rot);
		ImGui::Value("xflip", ter.tiles[TextureDebuggerTriangleX][TextureDebuggerTriangleY].fx);
		ImGui::Value("yflip", ter.tiles[TextureDebuggerTriangleX][TextureDebuggerTriangleY].fy);
		if(ImGui::Button("Buffer")) {
			ter.BufferData(shad.program);
		}
		ImGui::Text("%s", mask);
		ImGui::End();
		shad.use();
		glUniformMatrix4fv(glGetUniformLocation(shad.program, "ViewProjection"), 1, GL_FALSE, glm::value_ptr(viewProjection));
		ter.Render(shad.program);

		shad2.use();
		glUniformMatrix4fv(glGetUniformLocation(shad2.program, "ViewProjection"), 1, GL_FALSE, glm::value_ptr(viewProjection));
		obj.Render(shad2.program);

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
