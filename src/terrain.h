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

#ifndef TERRAIN_H_INCLUDED
#define TERRAIN_H_INCLUDED

#include "wmt.hpp"
#include "Shader.h"
#include "Texture.h"
#include "Object3d.h"

extern char* texpagesPath;

#define GTYPESMAX 15
#define TILEGROUNDSMAX 120

/* The shift on a world coordinate to get the tile coordinate */
#define TILE_SHIFT 7
static inline int32_t world_coord(int32_t mapCoord) { return (uint32_t)mapCoord << TILE_SHIFT; }
static inline int32_t map_coord(int32_t worldCoord) { return worldCoord >> TILE_SHIFT; }

// also see Terrain::BufferData
typedef struct {
	float x, y, z;
	float ux, uy;
} wme_terrain_glvertex_t;

class Terrain : public Object3d {
public:
	Shader* TerrainShader = nullptr;
	struct tileinfo {
		bool triflip;
		float height;
		int texture;
		int rot;
		bool fx, fy;
		WMT_TerrainTypes tt;
	} tiles[256][256];
	int w, h;
	WZtileset tileset;
	int DatasetLoaded;
	struct GroundType {
		char groundtype[80];
		char pagename[256];
		double size; // wif is this for?
		SDL_Texture * tex;
	} gtypes[GTYPESMAX];
	int gtypescount = 0;
	float* groundalphas = NULL;
	struct TileGround {
		char names[4][25] = {0}; // 25 prob. overkill but who cares at this point
		int groundTypes[4];
	} TileGrounds[TILEGROUNDSMAX]; // abstract size, recheck required
	Texture *GroundTilePage = nullptr;
	wme_terrain_glvertex_t * glVerticesTerrain = nullptr;
	void CreateShader();
	void LoadTerrainGrounds(char *basepath);
	void LoadTerrainGroundTypes(char *basepath, SDL_Renderer* rend);
	void AssociateGroundTypesWithTileGrounds();
	void LoadGroundTypesTextures(char *basepath, SDL_Renderer* rend);
	void ConstructGroundAlphas();
	void UpdateTexpageCoords();
	void GetHeightmapFromMWT(WZmap* m);
	void CreateTexturePage(char* basepath, int qual, SDL_Renderer* rend);
	void BufferData();
	void RenderV(glm::mat4 view);
	void Render();
};

#endif /* end of include guard: TERRAIN_H_INCLUDED */
