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
const char* TilesetStrings[] = {"Arizona", "Urban", "Rockies"};

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
	log_info("OpenGL %s", glGetString(GL_VERSION));
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
	mshader TerrainShader("./data/TerrainShaderVertex.vs", "./data/TerrainShaderFragment.frag");
	obj.UsingTexture = &tex;
	obj.PrepareTextureCoords();
	Terrain ter;
	WZmap map;
	WMT_ReadMap(secure_getenv("OPENMAP")?:(char*)"./data/8c-Stone-Jungle-E.wz", &map);
	ter.GetHeightmapFromMWT(&map);
	ter.CreateTexturePage(secure_getenv("TEXPAGES_PATH")?:(char*)"./data/texpages/", 128, rend);
	ter.UpdateTexpageCoords();
	ter.BufferData(TerrainShader.program);
	obj.BufferData(shad2.program);

	mshader TileSelectionShader("./data/TileSelectionShader.vs", "./data/TileSelectionShader.frag");
	std::vector<glm::vec3> TileSelectionVertexArray = {
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f,  0.0f, 128.0f },
		{ 128.0f, 0.0f, 128.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 128.0f, 0.0f, 128.0f },
		{ 128.0f,  0.0f, 0.0f },
	};
	unsigned int TileSelectionVertexArrayObject;
	unsigned int TileSelectionVertexBufferObject;
	glGenVertexArrays(1, &TileSelectionVertexArrayObject);
	glBindVertexArray(TileSelectionVertexArrayObject);
	glGenBuffers(1, &TileSelectionVertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, TileSelectionVertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, TileSelectionVertexArray.size() * 3 * sizeof(float), &TileSelectionVertexArray[0], GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(TileSelectionShader.program, "VertexCoordinates"), 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(glGetAttribLocation(TileSelectionShader.program, "VertexCoordinates"));

	glm::ivec2 mousePosition(0, 0);
	glm::vec3 cameraPosition(-249.569931, 2752.000000, 1513.794312);
	glm::vec3 cameraRotation(-51.000000, -104.000000, 0.000000);
	glm::vec3 cameraVelocity = {0, 0, 0};
	float cameraSpeed = 2.0f;
	glm::mat4 viewProjection;
	glm::ivec2 cameraMapPosition;
	float cameraFOV = 75.0f;
	glEnablei(GL_BLEND, 0);

	glm::ivec3 tileScreenCoords[256][256];
	long visibleTilesUpdateTime = 0;
	auto visibleTilesUpdate = [&] () {
		for(int y = 0; y < ter.h; y++) {
			for(int x = 0; x < ter.w; x++) {
				auto projectedPosition = glm::vec4(viewProjection * glm::vec4(world_coord(x), world_coord(ter.tiles[x][y].height), world_coord(y), 1.f));
				const float xx = projectedPosition.x / projectedPosition.w;
				const float yy = projectedPosition.y / projectedPosition.w;
				int screenX = (.5 + .5 * xx) * width;
				int screenY = (.5 - .5 * yy) * height;
				int screenZ = projectedPosition.w;

				tileScreenCoords[x][y] = glm::ivec3(screenX, screenY, screenZ);
			}
		}
	};

	glm::ivec2 mouseTilePosition(0, 0);
	auto mouseTilePositionUpdate = [&] () {
		for(int y = 0; y < ter.h - 1; y++) {
			for(int x = 0; x < ter.w - 1; x++) {
				auto aa = tileScreenCoords[x][y];
				auto ba = tileScreenCoords[x + 1][y];
				auto ab = tileScreenCoords[x][y + 1];
				auto bb = tileScreenCoords[x + 1][y + 1];

				int minX = std::min((int)aa.x, std::min((int)ba.x, std::min((int)ab.x, (int)bb.x)));
				int maxX = std::max((int)aa.x, std::max((int)ba.x, std::max((int)ab.x, (int)bb.x)));
				int minY = std::min((int)aa.y, std::min((int)ba.y, std::min((int)ab.y, (int)bb.y)));
				int maxY = std::max((int)aa.y, std::max((int)ba.y, std::max((int)ab.y, (int)bb.y)));

				if(mousePosition.x < minX) {
					continue;
				}
				if(mousePosition.x > maxX) {
					continue;
				}
				if(mousePosition.y < minY) {
					continue;
				}
				if(mousePosition.y > maxY) {
					continue;
				}

				mouseTilePosition = { x, y };
			}
		}
	};

	auto cameraUpdate = [&] () {
		cameraMapPosition.x = glm::clamp((int)(map_coord(cameraPosition.x)), 0, ter.w);
		cameraMapPosition.y = glm::clamp((int)(map_coord(cameraPosition.z)), 0, ter.h);
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
	int TextureDebuggerTriangleX = 0;
	int TextureDebuggerTriangleY = 0;
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
					mouseTilePositionUpdate();

					if(cursorTrapped) {
						cameraRotation.x -= ev.motion.yrel/2.0f;
						cameraRotation.y -= ev.motion.xrel/2.0f;
						cameraUpdate();
					}
				}
				break;

				case SDL_MOUSEWHEEL:
				if(!io.WantCaptureMouse) {
					cameraPosition.y += ev.wheel.y*128;
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
					case SDLK_ESCAPE:
					running = false;
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
		// if(visibleTilesUpdateTime < SDL_GetTicks()){
			visibleTilesUpdate();
			// visibleTilesUpdateTime = SDL_GetTicks() + 1000;
		// }

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window);
		ImGui::NewFrame();

		static bool show_window = true;
		static bool FPSlimiter = true;
		static bool ShowTextureDebugger = 0;
		static bool ShowTerrainTypesDebugger = 0;
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
		ImGui::Text("Dataset: %s", TilesetStrings[map.tileset]);
		if(ImGui::Button("Texture assigner")) {
			ShowTextureDebugger = true;
		}
		if(ImGui::Button("Terrain types")) {
			ShowTerrainTypesDebugger = true;
		}
		if(ImGui::Button("Print camera pos")) {
            log_info("Camera:\n\
glm::vec3 cameraPosition(%f, %f, %f);\n\
glm::vec3 cameraRotation(%f, %f, %f);", cameraPosition.x, cameraPosition.y, cameraPosition.z,
                                        cameraRotation.x, cameraRotation.y, cameraRotation.z);
        }
		ImGui::End();
		if(ShowTextureDebugger) {
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
				ter.BufferData(TerrainShader.program);
			}
			ImGui::End();
		}
		if(ShowTerrainTypesDebugger) {
			ImGui::SetNextWindowSize({150, 260});
			ImGui::Begin("Terrain types", &ShowTerrainTypesDebugger, ImGuiWindowFlags_NoResize);
			ImGui::Text("Version: %d", map.ttypver);
			ImGui::Text("Count: %d", map.ttypnum);
			ImGui::Separator();
			char num[10] = {0};
			for(int i=0; i<map.ttypnum; i++) {
				snprintf(num, 10, "%d", i);
				int j = map.ttyptt[i];
				ImGui::InputInt(num, &j);
				if(j != (int)map.ttyptt[i]) {
					map.ttyptt[i] = (short unsigned int)j;
				}
			}
			ImGui::End();
		}

		TerrainShader.use();
		glUniformMatrix4fv(glGetUniformLocation(TerrainShader.program, "ViewProjection"), 1, GL_FALSE, glm::value_ptr(viewProjection));
		ter.Render(TerrainShader.program);

		shad2.use();
		glUniformMatrix4fv(glGetUniformLocation(shad2.program, "ViewProjection"), 1, GL_FALSE, glm::value_ptr(viewProjection));
		obj.Render(shad2.program);

		if(mouseTilePosition.x != -1){
			glm::ivec2 mouseTileWorldCoordinates = { world_coord(mouseTilePosition.x), world_coord(mouseTilePosition.y) };
			auto aa = world_coord(ter.tiles[mouseTilePosition.x][mouseTilePosition.y].height);
			auto ab = world_coord(ter.tiles[mouseTilePosition.x + 1][mouseTilePosition.y].height);
			auto ba = world_coord(ter.tiles[mouseTilePosition.x][mouseTilePosition.y + 1].height);
			auto bb = world_coord(ter.tiles[mouseTilePosition.x + 1][mouseTilePosition.y + 1].height);
			
			TileSelectionVertexArray[0] = { mouseTileWorldCoordinates.x + 0.0f, aa, mouseTileWorldCoordinates.y + 0.0f };
			TileSelectionVertexArray[1] = { mouseTileWorldCoordinates.x + 0.0f, ba, mouseTileWorldCoordinates.y + 128.0f };
			TileSelectionVertexArray[2] = { mouseTileWorldCoordinates.x + 128.0f, bb, mouseTileWorldCoordinates.y + 128.0f };
			TileSelectionVertexArray[3] = { mouseTileWorldCoordinates.x + 0.0f, aa, mouseTileWorldCoordinates.y + 0.0f };
			TileSelectionVertexArray[4] = { mouseTileWorldCoordinates.x + 128.0f, bb, mouseTileWorldCoordinates.y + 128.0f };
			TileSelectionVertexArray[5] = { mouseTileWorldCoordinates.x + 128.0f, ab, mouseTileWorldCoordinates.y + 0.0f };

			glBindBuffer(GL_ARRAY_BUFFER, TileSelectionVertexBufferObject);
			glBufferData(GL_ARRAY_BUFFER, TileSelectionVertexArray.size() * 3 * sizeof(float), &TileSelectionVertexArray[0], GL_STATIC_DRAW);

			TileSelectionShader.use();
			glUniformMatrix4fv(glGetUniformLocation(TileSelectionShader.program, "ViewProjection"), 1, GL_FALSE, glm::value_ptr(viewProjection));
			glBindVertexArray(TileSelectionVertexArrayObject);
			glDisable(GL_DEPTH_TEST);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glEnable(GL_DEPTH_TEST);
		}

		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);

		if((Uint32)1000/FPS > SDL_GetTicks()-frame_time_start && FPSlimiter) {
			SDL_Delay(1000/FPS-(SDL_GetTicks()-frame_time_start));
		}
	}
	obj.Free();
	ter.Free();
	// tex.Free();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(window);
	SDL_Quit();
	glfwTerminate();
	WMT_FreeMap(&map);
	return 0;
}
