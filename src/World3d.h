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

#ifndef WORLD3D_H_INCLUDED
#define WORLD3D_H_INCLUDED

#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "wmt.hpp"
#include "Object3d.h"
#include "Texture.h"
#include "terrain.h"

class World3d {
private:
	int texids = 0;
	// int GetNextTextureId();
	// Texture* GetTexture(std::string filepath);
	Shader* ObjectsShader = nullptr;
public:
	WZmap* map;
	std::vector<Object3d*> Objects;
	std::vector<Texture*> Textures;
	Tileset Tst;
	Terrain Ter;
	SDL_Renderer *Renderer;
	World3d(WZmap *m, SDL_Renderer *r);
	~World3d();
	// void AddObject(std::string filename, unsigned int);
	void RenderScene(glm::mat4 view);
};

#endif /* end of include guard: WORLD3D_H_INCLUDED */
