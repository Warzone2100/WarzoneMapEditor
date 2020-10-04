#include "World3d.h"

#include "log.hpp"

#include <stdio.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <SDL2/SDL_image.h>

Object3d::Object3d() {
	GLvertexes.clear();
	GLpos = {0.0f, 0.0f, 0.0f};
	GLrot = {0.0f, 0.0f, 0.0f};
	GLscale = 0.0f;
	Texture = -1;
	Visible = false;
}

bool Object3d::LoadFromPIE(std::string filepath) {
	FILE* f = fopen(filepath.c_str(), "r");
	if(f == NULL) {
		log_error("Error opening file");
		return false;
	}
	int type, dummy, pointscount, ret, ver;
	char texturepagepath[512];
	ret = fscanf(f, "PIE %d\nTYPE %d\nTEXTURE %d %s %d %d\nLEVELS %d\nLEVEL %d\nPOINTS %d\n", &ver, &type, &dummy, texturepagepath, &dummy, &dummy, &dummy, &dummy, &pointscount);
	if(ret!=9) {
		log_error("PIE scanf 1 %d", ret);
		abort();
	}
	this->TexturePath = texturepagepath;
	std::vector<glm::vec3> points;
	for(int i=0; i<pointscount; i++) {
		glm::vec3 newpoint;
		ret = fscanf(f, "\t%f %f %f\n", &newpoint.x, &newpoint.y, &newpoint.z);
		if(ret!=3) {
			log_error("PIE scanf 2 %d", ret);
			abort();
		}
		points.push_back(newpoint);
	}

	int polycount;
	ret = fscanf(f, "POLYGONS %d", &polycount);
	if(ret!=1) {
		log_error("PIE scanf 3 %d", ret);
		abort();
	}

	std::vector<PIEpolygon> polygons;
	for(int i=0; i<polycount; i++) {
		PIEpolygon newpolygon;
		ret = fscanf(f, "\t%d %d", &newpolygon.flags, &newpolygon.pcount);
		if(ret!=2) {
			log_error("PIE scanf 4 %d (%d)", ret, i);
			abort();
		}
		for(int j=0; j<newpolygon.pcount; j++) {
			ret = fscanf(f, " %d", &newpolygon.porder[j]);
			if(ret!=1) {
				log_error("PIE scanf 5 %d (%d) (%d)", ret, i, j);
				abort();
			}
		}
		if(newpolygon.flags!=200) {
			log_error("Polygons bad");
			abort();
		}
		for(int j=0; j<newpolygon.pcount*2; j++) {
			ret = fscanf(f, " %f", &newpolygon.texcoords[j]);
			if(ret!=1) {
				log_error("PIE scanf 6 %d (%d) (%d)\n", ret, i, j);
				abort();
			}
		}
	}
	fclose(f);
	return true;
}

Texture::Texture(std::string path, SDL_Renderer *rend) {
	this->path = path;
	log_info("Loading [%s] texture...", this->path.c_str());
	SDL_Surface* loadedSurf = IMG_Load(this->path.c_str());
	if(loadedSurf==NULL) {
		log_fatal("Texture Loading error: %s\n", IMG_GetError());
		return;
	} else {
		this->tex = SDL_CreateTextureFromSurface(rend, loadedSurf);
		if(this->tex == NULL) {
			log_fatal("Texture converting error: %s\n", IMG_GetError());
			return;
		}
		SDL_FreeSurface(loadedSurf);
	}
	SDL_QueryTexture(this->tex, NULL, NULL, &this->w, &this->h);
	log_info("Loaded [%s] texture.", this->path.c_str());
	return;
}

void Texture::GenerateTexture() {
	glGenTextures(1, &GLid);
	return;
}

Texture* World3d::GetTexture(std::string filepath) {
	for(int i=0; i<Textures.size(); i++) {
		if(Textures[i].valid && Textures[i].path == filepath) {
			return &Textures[i];
		}
	}
}

void World3d::AddTexture(std::string filepath) {
	Texture newtex(filepath, Renderer);
	if(newtex.valid) {
		Textures.push_back(newtex);
	}
}

void World3d::AddObject(std::string filename) {
	Object3d creating;
	creating.LoadFromPIE(filename);
}
