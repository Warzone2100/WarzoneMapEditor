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

#ifndef PIE_H
#define PIE_H

#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

struct PIEpolygon {
	int flags = 0;
	int pcount = 0;
	int porder[3];
	float texcoords[6];
};

struct PIEshadowpolygon {
	int flags = 0;
	int pcount = 0;
	int porder[3];
};

struct PIEanimframe {
	int frame = 0;
	glm::vec3 pos;
	glm::vec3 rot;
	glm::vec3 scale;
};

class PIElevel {
public:
	glm::vec3* points = nullptr;
	int pointscount = 0;
	glm::vec3* normals = nullptr;
	int normalscount = 0;
	PIEpolygon* polygons = nullptr;
	int polygonscount = 0;
	glm::vec3* connectors = nullptr;
	int connectorscount = 0;
	PIEanimframe* anim = nullptr;
	int animtime = 0;
	int animcycles = 0;
	int animframes = 0;
	glm::vec3* shadowpoints = nullptr;
	int shadowpointscount = 0;
	PIEshadowpolygon* shadowpolygons = nullptr;
	int shadowpolygonscount = 0;
	void InitAtZero();
	~PIElevel();
};

class PIEmodel {
public:
	int ver = 0;
	int type;
	bool interpolate = true;
	std::string texturename = "";
	int tsizew = 0, tsizeh = 0;
	std::string normalname = "";
	std::string specularname = "";
	std::string events[3] = {"", "", ""};
	PIElevel* levels = nullptr;
	// starts from 0! level 1 becomes 0 etc!
	int levelscount = 0;
	PIEmodel();
	bool ReadPIE(std::string path);
	~PIEmodel();
};

// struct PIEobject ReadPIE(char* path, SDL_Renderer* rend);
// bool PIEreadTexture(PIEobject* o, SDL_Renderer* rend);
// void FreePIE(struct PIEobject* o);
// void PIEbindTexpage(PIEobject* o);
// void PIEprepareGLarrays(PIEobject* o);
#endif
