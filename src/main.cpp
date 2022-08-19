/*
    This file is part of WZ2100 Map Editor.
    Copyright (C) 2020-2021  maxsupermanhd
    Copyright (C) 2020-2021  bjorn-ali-goransson

    WZ2100 Map Editor is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    WZ2100 Map Editor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with WZ2100 Map Editor; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

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
#include "Shader.h"
#include "Pie.h"
#include "World3d.h"
#include "terrain.h"
#include "args.h"
#include "other.h"
#include "stats.h"
#include "Models.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

#define FPS 60

int width = 1280;
int height = 720;

const char* demopieobjectpath = "./data/blbrbgen.pie";
const char* demopieobjectpath2 = "./data/vtolfactory_module1.pie";
const char* TilesetStrings[] = {"Arizona", "Urban", "Rockies"};

int main(int argc, char** argv) {
	log_set_level(2);
	ProcessArgs(argc, argv);
	time_t t;
	srand((unsigned) time(&t));
	print_configuration();

	if(glfwInit() != GLFW_TRUE) {
		char* ep;
		int er = glfwGetError((const char**)&ep);
		log_fatal("Failed to initialize GLFW: code %d [%s]", er, ep);
		return 1;
	}
	log_info("GLFW init done.");

	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		log_fatal("SDL init error: %s", SDL_GetError());
		return 1;
	}
	log_info("SDL init done.");
	if(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1)) {log_fatal("Failed to set SDL_GL_DOUBLEBUFFER: %s", SDL_GetError()); return 1;}
	if(SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8)) 	{log_fatal("Failed to set SDL_GL_RED_SIZE: %s", SDL_GetError()); return 1;}
	if(SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8))	{log_fatal("Failed to set SDL_GL_GREEN_SIZE: %s", SDL_GetError()); return 1;}
	if(SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8)) 	{log_fatal("Failed to set SDL_GL_BLUE_SIZE: %s", SDL_GetError()); return 1;}
	if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE)) {log_fatal("Failed to set SDL_GL_CONTEXT_PROFILE_MASK: %s", SDL_GetError()); return 1;}
	if(SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL,    1)) {log_fatal("Failed to set SDL_GL_ACCELERATED_VISUAL: %s", SDL_GetError()); return 1;}
	if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3)) {log_fatal("Failed to set SDL_GL_CONTEXT_MAJOR_VERSION: %s", SDL_GetError()); return 1;}
	if(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3)) {log_fatal("Failed to set SDL_GL_CONTEXT_MINOR_VERSION: %s", SDL_GetError()); return 1;}
	log_info("SDL attributes set.");
	SDL_Window* window = SDL_CreateWindow("Warzone Map Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	if(window == NULL) {
		log_fatal("Failed to create window: %s", SDL_GetError());
		return 1;
	}
	SDL_SetWindowResizable(window, SDL_TRUE);
	log_info("SDL window created.");
	SDL_Renderer* rend = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if(rend == NULL) {
		log_fatal("Failed to create renderer: %s", SDL_GetError());
		return 1;
	}
	SDL_GLContext glcontext = SDL_GL_CreateContext(window);
	if(!glcontext) {
		log_fatal("gl context");
	}
	if(SDL_GL_LoadLibrary(NULL)) {
		log_fatal("Failed to load OpenGL library: %s", SDL_GetError());
		return 1;
	}
	log_info("SDL loaded GL library.");
	if(!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
		log_fatal("gladLoadGL failed");
		abort();
	}
	log_info("GLAD provided with SDL loader");
	log_info("OpenGL:   %s", glGetString(GL_VERSION));
	log_info("Vendor:   %s", glGetString(GL_VENDOR));
	log_info("Renderer: %s", glGetString(GL_RENDERER));
	if(SDL_GL_SetSwapInterval(1)) {
		log_warn("Failed to set vsync!");
	}
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
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(window, glcontext);
	const char* glsl_version = "#version 130";
	ImGui_ImplOpenGL3_Init(glsl_version);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(0.0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int drawfb;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (int*)&drawfb);
	log_info("Framebuffer %d is for drawing", drawfb);
	unsigned int idfb[3];
	glGenFramebuffers(3, (unsigned int*)&idfb);
	log_info("Framebuffer %d and %d for ids and %d for preview", idfb[0], idfb[1], idfb[2]);


	log_info("Loading stats...");
	if(!ParseStats("/home/max/warzone2100/data/base/stats/")) {
		return 1;
	}
	log_info("Stats loaded, opening map...");

	WZmap *map = (WZmap*)malloc(sizeof(WZmap));
	WMT_ReadMap(secure_getenv("OPENMAP")?:(char*)"./data/8c-Stone-Jungle-E.wz", map);
	if(!map->valid) {
		log_error("Failed to open map!");
		abort();
	}

	World3d World(map, rend);

	glm::ivec2 mousePosition(0, 0);
	glm::vec3 cameraPosition(148.939896, 960.000000, 81.356361);
	glm::vec3 cameraRotation(-53.500000, -135.500000, 0.000000);
	glm::vec3 cameraVelocity = {0, 0, 0};
	float cameraSpeed = 2.0f;
	glm::mat4 viewProjection;
	glm::ivec2 cameraMapPosition;
	float cameraFOV = 75.0f;
	glEnablei(GL_BLEND, 0);

	auto cameraUpdate = [&] () {
		cameraMapPosition.x = glm::clamp((int)(map_coord(cameraPosition.x)), 0, World.Ter.w);
		cameraMapPosition.y = glm::clamp((int)(map_coord(cameraPosition.z)), 0, World.Ter.h);
		viewProjection = glm::perspective(glm::radians(cameraFOV), (float) width / (float)height, 30.0f, 100000.0f) *
			glm::rotate(glm::mat4(1), glm::radians(-cameraRotation.x), glm::vec3(1, 0, 0)) *
			glm::rotate(glm::mat4(1), glm::radians(-cameraRotation.y), glm::vec3(0, 1, 0)) *
			glm::rotate(glm::mat4(1), glm::radians(-cameraRotation.z), glm::vec3(0, 0, 1)) *
			glm::translate(glm::mat4(1), -cameraPosition) *
			glm::mat4(1);
	};
	cameraUpdate();
	bool cursorTrapped = false;

	bool running = true;
	glEnable(GL_DEPTH_TEST);
	SDL_Event ev;
	Uint32 frame_time_start = 0;
	log_info("Entering render loop...");
	int camBaseVelocity = 8;
	Object3d* lastHoverObject = nullptr;
	bool hoverPick = false;
	while(running) {
		frame_time_start = SDL_GetTicks();
		while(SDL_PollEvent(&ev)) {
			ImGui_ImplSDL2_ProcessEvent(&ev);
			switch(ev.type) {
				case SDL_QUIT:
				running = false;
				break;

				case SDL_MOUSEMOTION:
				if(!io.WantCaptureMouse) {
					mousePosition.x = ev.motion.x;
					mousePosition.y = ev.motion.y;
					if(cursorTrapped) {
						cameraRotation.x -= ev.motion.yrel/2.0f;
						cameraRotation.y -= ev.motion.xrel/2.0f;
						cameraUpdate();
					}
				}
				break;

				case SDL_MOUSEWHEEL:
				if(!io.WantCaptureMouse) {
					cameraPosition.y -= ev.wheel.y*128;
				}
				break;

				case SDL_WINDOWEVENT:
				if(ev.window.event == SDL_WINDOWEVENT_RESIZED || ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
					width = ev.window.data1;
					height = ev.window.data2;
					cameraUpdate();
				}
				break;
				case SDL_MOUSEBUTTONDOWN:
				if(!io.WantCaptureMouse) {
					switch(ev.button.button) {
						case SDL_BUTTON_RIGHT:
						if(!cursorTrapped) {
							SDL_SetRelativeMouseMode(SDL_TRUE);
						}
						cursorTrapped = true;
						break;
					}
				}
				break;
				case SDL_MOUSEBUTTONUP:
				if(!io.WantCaptureMouse) {
					switch(ev.button.button) {
						case SDL_BUTTON_RIGHT:
						cursorTrapped = false;
						SDL_SetRelativeMouseMode(SDL_FALSE);
						break;
					}
				}
				break;
				case SDL_KEYDOWN:
				switch(ev.key.keysym.sym) {
					case SDLK_LCTRL:
					camBaseVelocity = 8*4;
					break;
					case SDLK_ESCAPE:
					running = false;
					break;
					case SDLK_w:
					cameraVelocity.z = -camBaseVelocity;
					break;
					case SDLK_a:
					cameraVelocity.x = -camBaseVelocity;
					break;
					case SDLK_e:
					cameraVelocity.y = camBaseVelocity;
					break;
					case SDLK_s:
					cameraVelocity.z = camBaseVelocity;
					break;
					case SDLK_d:
					cameraVelocity.x = camBaseVelocity;
					break;
					case SDLK_q:
					cameraVelocity.y = -camBaseVelocity;
					break;
					default:
					break;
				}
				break;
				case SDL_KEYUP:
				switch(ev.key.keysym.sym) {
					case SDLK_LCTRL:
					camBaseVelocity = 8;
					break;
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
					default:
					break;
				}
				break;
				break;
			}
		}

		SDL_PumpEvents();

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

		static bool ShowOverlay = true;
		static bool FPSlimiter = true;
		static bool ShowDemoWindow = false;
		static bool ShowDemoWindowMetrics = false;
		static bool ShowDebugger = false;
		static bool ShowTerrainTypesDebugger = false;
		static bool ShowTileDebugger = false;
		static bool ShowStructureEditor = false;
		static int StructureEditorN = 0;
		static bool ShowWorldRenderObjectsDebugger = false;
		static bool ShowSelectedObject = false;
		static int ShowGLTextureN = 0;
		static bool ShowGLTexturesDebugger = false;
		if(ImGui::BeginMainMenuBar()) {
			if(ImGui::BeginMenu("Debuggers")) {
				ImGui::MenuItem("Overlay", NULL, &ShowOverlay);
				ImGui::MenuItem("TTypes", NULL, &ShowTerrainTypesDebugger);
				ImGui::MenuItem("Tile", NULL, &ShowTileDebugger);
				ImGui::MenuItem("Structure", NULL, &ShowStructureEditor);
				ImGui::MenuItem("World - Structures", NULL, &ShowWorldRenderObjectsDebugger);
				ImGui::MenuItem("OpenGL Textures", NULL, &ShowGLTexturesDebugger);
				ImGui::EndMenu();
			}
			if(ImGui::BeginMenu("Misc")) {
				ImGui::MenuItem("Demo window", NULL, &ShowDemoWindow);
				ImGui::MenuItem("GUI metrics", NULL, &ShowDemoWindowMetrics);
				ImGui::EndMenu();
			}
			if(ImGui::BeginMenu("Objects")) {
				ImGui::MenuItem("Selected object", NULL, &ShowSelectedObject);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
		if(ShowDemoWindow) {ImGui::ShowDemoWindow(&ShowDemoWindow);}
		if(ShowDemoWindowMetrics) {ImGui::ShowMetricsWindow(&ShowDemoWindowMetrics);}
		if(ShowTerrainTypesDebugger) {
			ImGui::SetNextWindowSize({150, 260});
			ImGui::Begin("Terrain types", &ShowTerrainTypesDebugger);
			ImGui::Text("Version: %d", map->ttypver);
			ImGui::Text("Count: %d", map->ttypnum);
			if(ImGui::Button("Add")) {
				if(ImGui::BeginPopupModal("Add terrain type")){
					ImGui::EndPopup();
				}
			}
			ImGui::Separator();
			char num[10] = {0};
			for(int i=0; i<map->ttypnum; i++) {
				snprintf(num, 10, "%d", i);
				int j = map->ttyptt[i];
				ImGui::InputInt(num, &j);
				if(j != (int)map->ttyptt[i]) {
					map->ttyptt[i] = (short unsigned int)j;
				}
			}
			ImGui::End();
		}
		if(ShowTileDebugger) {
			ImGui::Begin("Tile debugger", &ShowTileDebugger);
			// ImGui::Text("Tile x%d y%d", mouseTilePosition[0], mouseTilePosition[1]);
			// struct Terrain::tileinfo t = World.Ter.tiles[mouseTilePosition[0]][mouseTilePosition[1]];
			// ImGui::Text("Flip: %c:%c Rot: %d", t.fx?'X':'_', t.fy?'Y':'_', t.rot);
			// ImGui::Text("Texture: %3d Flip: %c", t.texture, t.triflip?'Y':'N');
			// ImGui::Text("Height: %f", t.height);
			// ImGui::Text("TT: %s", WMT_TerrainTypesStrings[(int)t.tt]);
			ImGui::End();
		}
		if(ShowStructureEditor) {
			ImGui::Begin("Structure editor", &ShowStructureEditor);
			ImGui::Text("Structure version: %d", World.map->structVersion);
			if(!World.map->structs) {
				ImGui::Text("Structure pointer is NULL!");
			} else {
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Object:");
				ImGui::SameLine();
				float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
				ImGui::PushButtonRepeat(true);
				if(ImGui::ArrowButton("##left", ImGuiDir_Left) && StructureEditorN > 0) {StructureEditorN--;}
				ImGui::SameLine(0.0f, spacing);
				if(ImGui::ArrowButton("##right", ImGuiDir_Right) && StructureEditorN < World.map->numStructures) {StructureEditorN++;}
				ImGui::PopButtonRepeat();
				ImGui::SameLine();
				ImGui::Text("%d/%d", StructureEditorN, World.map->numStructures);
				WZobject o = World.map->structs[StructureEditorN];
				ImGui::InputText("Name", o.name, 127);
				int oid = o.id, ox = o.x, oy = o.y, oz = o.z, odir = o.direction, opl = o.player;
				ImGui::InputInt("Id", &oid);
				ImGui::InputInt("X", &ox);
				ImGui::InputInt("Y", &oy);
				ImGui::InputInt("Z", &oz);
				ImGui::InputInt("Direction", &odir);
				ImGui::InputInt("Player", &opl);
				ImGui::InputInt3("Rotation", o.rotation);
			}
			ImGui::End();
		}
		if(ShowWorldRenderObjectsDebugger) {
			ImGui::Begin("World structures debugger", &ShowWorldRenderObjectsDebugger);
			long objsize = World.Objects.size();
			ImGui::Text("Structures count: %ld", objsize);
			for(int i = 0; i < objsize; i++) {
				ImGui::Text("Object: %d, pick %d", i, World.Objects[i]->pickid);
				char label[120] = {0};
				snprintf(label, 119, "GLpos %d", i);
				ImGui::InputFloat3(label, (float*)&World.Objects[i]->GLpos.x);
				snprintf(label, 119, "GLrot %d", i);
				ImGui::InputFloat3(label, (float*)&World.Objects[i]->GLrot.x);
			}
			ImGui::End();
		}
		if(ShowGLTexturesDebugger) {
			ImGui::Begin("Texture viewer", &ShowGLTexturesDebugger);
			ImGui::InputInt("textureid", &ShowGLTextureN);
			ImGui::Image((void*)(intptr_t)(ShowGLTextureN), ImVec2(512, 512));
			ImGui::End();
		}

		// glBindFramebuffer(GL_FRAMEBUFFER, idfb[0]);
		// if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		// 	log_fatal("Framebuffer not ready!");
		// 	continue;
		// }

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		World.RenderPickScene(viewProjection);
		glFlush();
		glFinish();
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		unsigned char data[4];
		int mreadx, mready, windw, windh;
		SDL_GetMouseState(&mreadx, &mready);
		SDL_GetWindowSize(window, &windw, &windh); // for some reason framebuffer is Y-flipped
		glReadPixels(mreadx, windh-mready, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
		Object3d* prev = World.GetPickingObject(data[0] + data[1]*256 + data[2]*256*256);
		if(ShowSelectedObject) {
			ImGui::Begin("Selected object", &ShowSelectedObject);
			if(prev == nullptr) {
				ImGui::Text("No object selected");
			} else {
				ImGui::Text("Object %d", prev->pickid);
				ImGui::InputFloat3("GLpos", (float*)&prev->GLpos.x);
				ImGui::InputFloat3("GLrot", (float*)&prev->GLrot.x);
				ImGui::Text("Object's texture path: %s", prev->TexturePath.c_str());
				if(prev->UsingTexture == nullptr) {
					ImGui::Text("UsingTexture is NULL");
				} else {
					ImGui::Text("Loaded texture valid? %s", prev->UsingTexture->valid ? "yes" : "no");
					ImGui::Text("Loaded texture ID: %d", prev->UsingTexture->id);
					ImGui::Text("Loaded texture path: %s", prev->UsingTexture->path.c_str());
					ImGui::Text("Loaded texture OpenGL ID: %d", prev->UsingTexture->GLid);
					ImGui::Text("Loaded texture size: w%d:h%d", prev->UsingTexture->w, prev->UsingTexture->h);
					ImGui::Image((void*)(intptr_t)(prev->UsingTexture->GLid), ImVec2(prev->UsingTexture->w, prev->UsingTexture->h));
					// ImGui::Image((void*)(intptr_t)(GL_TEXTURE0+prev->UsingTexture->GLid), ImVec2(prev->UsingTexture->w, prev->UsingTexture->h));
					// ImGui::Image((void*)(intptr_t)(0), ImVec2(prev->UsingTexture->w, prev->UsingTexture->h));
				}
			}
			ImGui::End();
		}
		if(lastHoverObject != nullptr && prev != lastHoverObject) {
			lastHoverObject->DoColorMix = false;
		}
		if(prev != nullptr) {
			prev->DoColorMix = true;
		}
		lastHoverObject = prev;

		// glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawfb);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		World.RenderScene(viewProjection);

		if(ShowOverlay) {
			ImGui::SetNextWindowPos({0, ImGui::GetItemRectSize()[1]}, 1);
			ImGui::Begin("##bmain", &ShowOverlay,   ImGuiWindowFlags_NoMove |
			 										ImGuiWindowFlags_NoResize |
													ImGuiWindowFlags_NoTitleBar |
													ImGuiWindowFlags_NoResize |
													ImGuiWindowFlags_NoCollapse |
													ImGuiWindowFlags_AlwaysAutoResize |
													ImGuiWindowFlags_NoBackground);
			ImGui::Checkbox("Fps limit", &FPSlimiter);
			ImGui::Checkbox("Fill textures", &World.Ter.FillTextures);
			ImGui::Text("%.3f (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text("Cam map pos: %3d %3d", cameraMapPosition.x, cameraMapPosition.y);
			ImGui::Text("Cam fov: %f", cameraFOV);
			ImGui::Text("Dataset: %s", TilesetStrings[map->tileset]);
			ImGui::Text("Pixel data: %d %d %d %d", data[0], data[1], data[2], data[3]);
			if(ImGui::Button("Print camera pos")) {
	            log_info("Camera:\n\
	glm::vec3 cameraPosition(%f, %f, %f);\n\
	glm::vec3 cameraRotation(%f, %f, %f);", cameraPosition.x, cameraPosition.y, cameraPosition.z,
	                                        cameraRotation.x, cameraRotation.y, cameraRotation.z);
	        }
			ImGui::End();
		}

		ImGui::Render();
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
	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(window);
	SDL_Quit();

	glfwTerminate();

	WMT_FreeMap(map);
	FreeModels();

	return 0;
}
