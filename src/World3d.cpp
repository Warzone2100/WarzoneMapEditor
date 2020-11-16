#include "World3d.h"

#include "log.hpp"

#include <stdio.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <SDL2/SDL_image.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <unistd.h>

// // Search in textures, maybe we already loaded it...
// Texture* World3d::GetTexture(std::string filepath) {
// 	for(long unsigned int i=0; i<Textures.size(); i++) {
// 		if(Textures[i]->valid && Textures[i]->path == filepath) {
// 			return Textures[i];
// 		}
// 	}
// 	return nullptr;
// }
//
// void World3d::AddObject(std::string filename, unsigned int Shader) {
// 	return;
// 	Object3d creating;
// 	creating.LoadFromPIE(filename);
// 	Texture* found = GetTexture(creating.TexturePath);
// 	if(found != nullptr) {
// 		creating.UsingTexture = found;
// 	} else {
// 		Texture* newtex = new Texture();
// 		newtex->Load(creating.TexturePath, Renderer);
// 		// newtex.Bind(GetNextTextureId());
// 		Textures.push_back(&newtex);
// 		creating.UsingTexture = &Textures[Textures.size()-1];
// 	}
// 	creating.PrepareTextureCoords();
// 	creating.BufferData(Shader);
// 	Objects.push_back(creating);
// }
//
// int World3d::GetNextTextureId() {
// 	return texids++;
// }

void World3d::RenderScene(glm::mat4 view) {
	Ter.RenderV(view);
	for(auto &a : Objects) {
		a->Render(ObjectsShader->program);
	}
}

World3d::World3d(WZmap* m, SDL_Renderer *r) {
	Renderer = r;
	Objects.clear();
	Textures.clear();
	if(!m->valid) {
		log_error("Not valid map!");
		abort();
	}
	this->map = m;
	Ter.GetHeightmapFromMWT(this->map);
	char* datapath = secure_getenv("WZMAP_DATA_PATH")?:(char*)"./data/";
	Ter.CreateTexturePage(datapath, 128, Renderer);
	Ter.LoadTerrainGrounds(datapath);
	Ter.LoadTerrainGroundTypes(datapath);
	Ter.UpdateTexpageCoords();
	Ter.CreateShader();
	Ter.BufferData();
	ObjectsShader = new Shader("./data/vertex.vs", "./data/fragment.frag");
}

World3d::~World3d() {
	Ter.~Terrain();
	for(auto a : this->Textures) {
		a->~Texture();
		delete a;
	}
	if(ObjectsShader) {
		delete ObjectsShader;
	}
}
