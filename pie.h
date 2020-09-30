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
};

struct PIEobject ReadPIE(char* path, SDL_Renderer* rend);
bool PIEreadTexture(PIEobject* o, SDL_Renderer* rend);
void FreePIE(struct PIEobject* o);
void PIEbindTexpage(PIEobject* o);
void PIEprepareGLarrays(PIEobject* o);
#endif
