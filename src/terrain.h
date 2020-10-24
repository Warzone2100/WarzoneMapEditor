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
private:
	Shader* TerrainShader = nullptr;
public:
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
		Texture* tex;
	} gtypes[GTYPESMAX];
	Texture* waterpage;
	Terrain();
	~Terrain();
	void LoadTerrainGrounds(char *basepath);
	void UpdateTexpageCoords();
	void GetHeightmapFromMWT(WZmap* m);
	void CreateTexturePage(char* basepath, int qual, SDL_Renderer* rend);
	void BufferData();
	void RenderV(glm::mat4 view);
};

#endif /* end of include guard: TERRAIN_H_INCLUDED */
