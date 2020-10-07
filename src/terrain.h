#ifndef TERRAIN_H_INCLUDED
#define TERRAIN_H_INCLUDED

#include "World3d.h"
#include "wmt.hpp"

/* The shift on a world coordinate to get the tile coordinate */
#define TILE_SHIFT 7
static inline int32_t world_coord(int32_t mapCoord) { return (uint32_t)mapCoord << TILE_SHIFT; }
static inline int32_t map_coord(int32_t worldCoord) { return worldCoord >> TILE_SHIFT; }

class Terrain : public Object3d {
public:
	int w, h;
	int tileHeight[256][256] = {0};
	void GetHeightmapFromMWT(WZmap* m);
};

#endif /* end of include guard: TERRAIN_H_INCLUDED */
