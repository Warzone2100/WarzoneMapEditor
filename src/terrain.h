#ifndef TERRAIN_H_INCLUDED
#define TERRAIN_H_INCLUDED

#include "wmt.hpp"
#include "Shader.h"
#include "Texture.h"
#include "Object3d.h"

extern char* texpagesPath;

#define GTYPESMAX 15

/* The shift on a world coordinate to get the tile coordinate */
#define TILE_SHIFT 7
static inline int32_t world_coord(int32_t mapCoord) { return (uint32_t)mapCoord << TILE_SHIFT; }
static inline int32_t map_coord(int32_t worldCoord) { return worldCoord >> TILE_SHIFT; }

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
		unsigned int tex;
	} gtypes[GTYPESMAX];
	int gtypescount = 0;
	float* groundalphas = NULL;
	struct TileGround {
		char names[4][25] = {0}; // 25 prob. overkill but who cares at this point
	} TileGrounds[120]; // abstract size, recheck required
	Texture *GroundTexpage = nullptr;
	void CreateShader();
	void LoadTerrainGrounds(char *basepath);
	void LoadTerrainGroundTypes(char *basepath);
	void LoadGroundTypesTextures(char *basepath);
	void ConstructGroundAlphas();
	void UpdateTexpageCoords();
	void GetHeightmapFromMWT(WZmap* m);
	void CreateTexturePage(char* basepath, int qual, SDL_Renderer* rend);
	void BufferData();
	void RenderV(glm::mat4 view);
	void Render();
};

#endif /* end of include guard: TERRAIN_H_INCLUDED */
