#ifndef PIE_H
#define PIE_H

#include "mypng.h"

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
	struct mpng texture;
	size_t GLvertexesCount;
	float *GLvertexes;
};

struct PIEobject ReadPIE(char* path);
void FreePIE(struct PIEobject* o);

#endif
