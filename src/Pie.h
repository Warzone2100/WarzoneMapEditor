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
	int flags;
	int pcount;
	int porder[3];
	float texcoords[6];
};

struct PIEshadowpolygon {
	int flags;
	int pcount;
	int porder[3];
};

struct PIEanimframe {
	int frame;
	glm::vec3 pos;
	glm::vec3 rot;
	glm::vec3 scale;
};

class PIElevel {
public:
	glm::vec3* points;
	int pointscount;
	glm::vec3* normals;
	int normalscount;
	PIEpolygon* polygons;
	int polygonscount;
	glm::vec3* connectors;
	int connectorscount;
	PIEanimframe* anim;
	int animtime;
	int animcycles;
	int animframes;
	glm::vec3* shadowpoints;
	int shadowpointscount;
	PIEshadowpolygon* shadowpolygons;
	int shadowpolygonscount;
	~PIElevel();
};

class PIEmodel {
	int ver = 0;
	int type;
	bool interpolate = true;
	std::string texturename;
	int tsizew, tsizeh;
	std::string normalname;
	std::string specularname;
	std::string events[3];
	PIElevel* levels;
	// starts from 0! level 1 becomes 0 etc!
	int levelscount;
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
