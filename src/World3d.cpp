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

#include "World3d.h"

#include "log.hpp"

#include <stdio.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <SDL2/SDL_image.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <unistd.h>
#include <algorithm>
#include <cctype>
#include <string>

#include "stats.h"

Texture* World3d::GetTexture(std::string filepath) {
	for(long unsigned int i=0; i<Textures.size(); i++) {
		if(Textures[i]->valid && Textures[i]->path == filepath) {
			return Textures[i];
		}
	}
	return nullptr;
}

Texture* World3d::GetOrLoadTexture(std::string filepath) {
	Texture* newtex = GetTexture(filepath);
	if(newtex == nullptr) {
		newtex = new Texture();
		newtex->Load(filepath, Renderer);
		newtex->Bind(GetNextTextureId());
		Textures.push_back(newtex);
	}
	return newtex;
}

Object3d* World3d::AddObject(std::string filename, unsigned int Shader) {
	Object3d* creating = new Object3d();
	if(!creating->LoadFromPIE(filename)) {
		return nullptr;
	}
	std::string texpath = "./data/texpages/" + creating->TexturePath;
	creating->UsingTexture = GetOrLoadTexture(texpath);
	creating->PrepareTextureCoords();
	creating->BufferData(Shader);
	creating->pickid = GetNextPickId();
	ObjectsMapped.insert_or_assign(creating->pickid, creating);
	Objects.push_back(creating);
	return creating;
}

int World3d::GetNextTextureId() {
	return texids++;
}

int World3d::GetNextPickId() {
	return pickids++;
}

void World3d::RenderScene(glm::mat4 view) {
	Ter.RenderV(view);
	for(auto &a : Objects) {
		// a->BufferData(ObjectsShader->program);
		a->Render(ObjectsShader->program);
	}
}

void World3d::RenderPickScene(glm::mat4 view) {
	// Ter.RenderV(view);
	this->ObjectsPickShader->use();
	glUniformMatrix4fv(glGetUniformLocation(this->ObjectsPickShader->program, "ViewProjection"), 1, GL_FALSE, glm::value_ptr(view));
	for(auto &a : Objects) {
		a->RenderColorPick(this->ObjectsPickShader->program);
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
	char* datapath = secure_getenv("WZMAP_DATA_PATH")?:(char*)"./data/";
	// data involving ground types 'n' stuff (tm)
	Tst.tileset = map->tileset;
	Tst.LoadTerrainGrounds(datapath);
	Tst.LoadTerrainGroundTypes(datapath, Renderer);
	Tst.AssociateGroundTypesWithTileGrounds();
	Tst.CreateTexturePage(datapath, 128, Renderer);
	Ter.TilesetPtr = &Tst;
	//
	Ter.GetHeightmapFromMWT(this->map);
	Ter.UpdateTexpageCoords();
	Ter.CreateShader();
	Ter.BufferData();
	ObjectsShader = new Shader("./data/shaders/vertex.vs", "./data/shaders/fragment.frag");
	ObjectsPickShader = new Shader("./data/shaders/plaincolor.vs", "./data/shaders/plaincolor.frag");
	for(int i=0; i<this->map->numStructures; i++) {
		WZobject o = this->map->structs[i];
		if(!Sstructures.count(o.name)) {
			log_warn("Failed to load object %.128s! Stat not found", o.name);
			continue;
		}
		if(Sstructures[o.name].structureModel.size() < 1) {
			log_fatal("Structure %.128s with no model?!", o.name);
			continue;
		}
		std::string firstpie = Sstructures[o.name].structureModel[0];
		std::transform(firstpie.begin(), firstpie.end(), firstpie.begin(), [](unsigned char c){ return std::tolower(c); });
		std::string loadpath = "/home/max/warzone2100/data/base/structs/"+firstpie;
		// log_info("Loading [%s]", loadpath.c_str());
		Object3d* a = this->AddObject(loadpath, ObjectsShader->program);
		if(!a) {
			log_error("Unable to load object %s! Failed to add model", o.name);
			continue;
		}
		a->GLpos[0] = -((float)o.x);
		a->GLpos[1] = -((float)o.z)*2;
		a->GLpos[2] = -((float)o.y);
		a->GLrot[1] = (((float)o.rotation[0])/16384)*90;
	}
	for(int i=0; i<this->map->featuresCount; i++) {
		WZfeature o = this->map->features[i];
		if(!Sfeatures.count(o.name)) {
			log_warn("Failed to load object %.128s! Stat not found", o.name);
			continue;
		}
		std::string firstpie = Sfeatures[o.name].model;
		std::transform(firstpie.begin(), firstpie.end(), firstpie.begin(), [](unsigned char c){ return std::tolower(c); });
		std::string loadpath = "/home/max/warzone2100/data/base/features/"+firstpie;
		// log_info("Loading [%s]", loadpath.c_str());
		Object3d* a = this->AddObject(loadpath, ObjectsShader->program);
		if(!a) {
			log_error("Unable to load object %s! Failed to add model", o.name);
			continue;
		}
		a->GLpos[0] = -((float)o.x);
		a->GLpos[1] = -((float)o.z)*2;
		a->GLpos[2] = -((float)o.y);
	}
}

Object3d* World3d::GetPickingObject(int id) {
	if(id == 0) {
		return nullptr;
	}
	if(!this->ObjectsMapped.count(id)) {
		log_warn("Object id %d not found!", id);
		return nullptr;
	}
	return this->ObjectsMapped[id];
}

World3d::~World3d() {
	Ter.~Terrain();
	for(auto a : this->Textures) {
		a->Free();
		delete a;
	}
	for(auto a : this->Objects) {
		a->Free();
		delete a;
	}
	if(ObjectsShader) {
		delete ObjectsShader;
	}
}
