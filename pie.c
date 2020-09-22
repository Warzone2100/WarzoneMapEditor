#include "pie.h"
#include "mypng.h"
#include "log.hpp"
#include <stdlib.h>

struct PIEobject ReadPIE(char* path) {
	FILE* f = fopen(path, "r");
	struct PIEobject o;
	o.valid = 0;
	if(f == NULL) {
		log_error("Error opening file");
		return o;
	}

	int ver, type, dummy, pointscount, ret;
	char texturepagepath[512];
	ret = fscanf(f, "PIE %d\nTYPE %d\nTEXTURE %d %s %d %d\nLEVELS %d\nLEVEL %d\nPOINTS %d\n", &ver, &type, &dummy, texturepagepath, &dummy, &dummy, &dummy, &dummy, &pointscount);
	if(ret!=9) {
		log_error("PIE scanf 1 %d", ret);
		abort();
	}
	o.points = (struct PIEpoint*)malloc(pointscount*sizeof(struct PIEpoint));
	for(int i=0; i<pointscount; i++) {
		ret = fscanf(f, "\t%f %f %f\n", &o.points[i].x, &o.points[i].y, &o.points[i].z);
		if(ret!=3) {
			log_error("PIE scanf 2 %d", ret);
			abort();
		}
		//printf("Point %f %f %f\n", o.points[i].x, o.points[i].y, o.points[i].z);
	}
	o.pointscount = pointscount;

	int polycount;
	ret = fscanf(f, "POLYGONS %d", &polycount);
	if(ret!=1) {
		log_error("PIE scanf 3 %d", ret);
		abort();
	}
	o.polygons = (struct PIEpolygon*)malloc(polycount*sizeof(struct PIEpolygon));
	o.polygonscount = polycount;
	for(int i=0; i<polycount; i++) {
		ret = fscanf(f, "\t%d %d", &o.polygons[i].flags, &o.polygons[i].pcount);
		if(ret!=2) {
			log_error("PIE scanf 4 %d (%d)", ret, i);
			abort();
		}
		for(int j=0; j<o.polygons[i].pcount; j++) {
			ret = fscanf(f, " %d", &o.polygons[i].porder[j]);
			if(ret!=1) {
				log_error("PIE scanf 5 %d (%d) (%d)", ret, i, j);
				abort();
			}
		}
		if(o.polygons[i].flags!=200) {
			log_error("Polygons bad");
			abort();
		}
		for(int j=0; j<o.polygons[i].pcount*2; j++) {
			ret = fscanf(f, " %f", &o.polygons[i].texcoords[j]);
			if(ret!=1) {
				log_error("PIE scanf 6 %d (%d) (%d)\n", ret, i, j);
				abort();
			}
		}
	}
	fclose(f);

	char fbufer[1024] = {0};
	snprintf(fbufer, 1023, "./%s", texturepagepath);
	log_info("Loading [%s] texpage...", texturepagepath);
	o.texture = read_png_file(fbufer);
	log_info("Loaded [%s] texpage.", texturepagepath);

	size_t pfillc = 0;
	size_t pfillmax = 0;
	for(int i=0; i<o.polygonscount; i++) {
		for(int j=0; j<o.polygons[i].pcount; j++) {
			pfillmax += 3;

		}
	}
	log_info("%d", pfillmax);
	o.GLvertexes = (float*)malloc(pfillmax*sizeof(float));
	o.GLvertexesCount = pfillmax;
	for(int i=0; i<o.polygonscount; i++) {
		if(o.polygons[i].pcount != 3) {
			log_fatal("Polygon converter error!");
			abort();
		}
		for(int j=0; j<o.polygons[i].pcount; j++) {
			o.GLvertexes[pfillc+0] = o.points[o.polygons[i].porder[j]].x;
			o.GLvertexes[pfillc+1] = o.points[o.polygons[i].porder[j]].y;
			o.GLvertexes[pfillc+2] = o.points[o.polygons[i].porder[j]].z;
			// log_info("%f %f %f", o.GLvertexes[pfillc+0], o.GLvertexes[pfillc+1], o.GLvertexes[pfillc+2]);
			pfillc+=3;
		}
	}
	log_info("%d", pfillc);

	o.valid = 1;
	return o;
}

void FreePIE(struct PIEobject* o) {
	if(o->valid) {
		free(o->polygons);
		o->polygons = NULL;
		free(o->points);
		o->points = NULL;
		free(o->GLvertexes);
		o->GLvertexes = NULL;
	}
	return;
}
