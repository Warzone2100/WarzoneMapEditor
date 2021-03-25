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

struct PIEpoint {
	float x, y, z;
};
struct PIEpolygon {
	int flags;
	int pcount;
	int porder[6];
	float texcoords[12];
};

struct PIEobject {
	struct PIEpoint* points;
	int pointscount;
	struct PIEpolygon* polygons;
	int polygonscount;
	int valid;
	char texturepath[512];
	SDL_Texture* texture;
	int texturewidth, textureheight;
	size_t GLvertexesCount;
	float *GLvertexes;
	int ver;
	float GLpos[3] = {0.0f, 0.0f, 0.0f};
	float GLrot[3] = {0.0f, 0.0f, 0.0f};
	float GLscale = 1.0f;
};

struct PIEobject ReadPIE(char* path, SDL_Renderer* rend);
bool PIEreadTexture(PIEobject* o, SDL_Renderer* rend);
void FreePIE(struct PIEobject* o);
void PIEbindTexpage(PIEobject* o);
void PIEprepareGLarrays(PIEobject* o);
#endif
