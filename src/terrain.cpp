#include "terrain.h"

void Terrain::GetHeightmapFromMWT(WZmap* map) {
	if(!map->valid) {
		log_error("WMT failed to read map!");
		return;
	}
	w = map->maptotalx;
	h = map->maptotaly;
	RenderingMode = GL_TRIANGLES;
	GLvertexesCount = (h-1)*(w-1)*2*3*5;
	GLvertexes = (float*)malloc(GLvertexesCount*sizeof(float));
	size_t filled = 0;
	auto addTriangle = [&] (int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3) {
		GLvertexes[filled+0] = world_coord(x1);
		GLvertexes[filled+1] = world_coord(y1);
		GLvertexes[filled+2] = world_coord(z1);
		GLvertexes[filled+3] = 0;
		GLvertexes[filled+4] = 0;
		filled+=5;
		GLvertexes[filled+0] = world_coord(x2);
		GLvertexes[filled+1] = world_coord(y2);
		GLvertexes[filled+2] = world_coord(z2);
		GLvertexes[filled+3] = 0;
		GLvertexes[filled+4] = 0;
		filled+=5;
		GLvertexes[filled+0] = world_coord(x3);
		GLvertexes[filled+1] = world_coord(y3);
		GLvertexes[filled+2] = world_coord(z3);
		GLvertexes[filled+3] = 0;
		GLvertexes[filled+4] = 0;
		filled+=5;
	};
	int scale = 32;
	for(int y=0; y<h; y++) {
		for(int x=0; x<w; x++) {
			tileHeight[x][y] = map->mapheight[y*w+x]/scale;
		}
	}
	for(int y=0; y<h-1; y++) {
		for(int x=0; x<w-1; x++) {
			if(WMT_TileGetTriFlip(map->maptile[y*w+x])) {
				addTriangle(x,   tileHeight[x  ][y  ], y,
							x,   tileHeight[x  ][y+1], y+1,
							x+1, tileHeight[x+1][y  ], y);
				addTriangle(x+1, tileHeight[x+1][y  ], y,
							x+1, tileHeight[x+1][y+1], y+1,
							x,   tileHeight[x  ][y+1], y+1);
			} else {
				addTriangle(x,   tileHeight[x  ][y  ], y,
							x,   tileHeight[x  ][y+1], y+1,
							x+1, tileHeight[x+1][y+1], y+1);
				addTriangle(x,   tileHeight[x  ][y  ], y,
							x+1, tileHeight[x+1][y  ], y,
							x+1, tileHeight[x+1][y+1], y+1);
			}
		}
	}
	FillTextures = false;
}