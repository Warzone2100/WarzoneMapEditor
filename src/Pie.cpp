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

#include "Pie.h"
#include "log.hpp"
#include "other.h"
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>

void PIElevel::InitAtZero() {
	this->points = nullptr;
	this->pointscount = 0;
	this->normals = nullptr;
	this->normalscount = 0;
	this->polygons = nullptr;
	this->polygonscount = 0;
	this->connectors = nullptr;
	this->connectorscount = 0;
	this->anim = nullptr;
	this->animtime = 0;
	this->animcycles = 0;
	this->animframes = 0;
	this->shadowpoints = nullptr;
	this->shadowpointscount = 0;
	this->shadowpolygons = nullptr;
	this->shadowpolygonscount = 0;
}

PIElevel::~PIElevel() {
	if(this->points) {
		free(points);
	}
	if(this->normals) {
		free(this->normals);
	}
	if(this->connectors) {
		free(this->connectors);
	}
	if(this->anim) {
		free(this->anim);
	}
	if(this->shadowpoints) {
		free(this->shadowpoints);
	}
	if(this->shadowpolygons) {
		free(this->shadowpolygons);
	}
}

PIEmodel::~PIEmodel() {
	if(this->levels) {
		for(int i = 0; i < this->levelscount; i++) {
			this->levels[i].~PIElevel();
		}
	}
}

PIEmodel::PIEmodel() {
}

class AutoFreeFileHandle {
public:
	FILE* f = NULL;
	AutoFreeFileHandle(FILE* f) : f(f) {}
	~AutoFreeFileHandle() {
		if(f) {
			fclose(f);
		}
	}
	AutoFreeFileHandle() = delete;
	AutoFreeFileHandle(const AutoFreeFileHandle&) = delete;
	AutoFreeFileHandle& operator=(const AutoFreeFileHandle&) = delete;
};

bool PIEmodel::ReadPIE(std::string path) {
	AutoFreeFileHandle fh(fopen(path.c_str(), "r"));
	FILE* f = fh.f; // this is dirty hack to not call fclose on every return
	if(f == NULL) {
		log_error("Error opening file [%s]: %s", strerror(errno));
		return false;
	}
	log_trace("Loading PIE [%s]", path.c_str());

	int ret = fscanf(f, "PIE %d\nTYPE %d\n", &this->ver, &this->type);
	if(ret != 2) {
		log_error("Failed to parse first 2 lines of PIE, ret %d", ret);
		return false;
	}

	int snum = 3;
	int nowlevel = -1;
	char* cstr = NULL;
	size_t len = 0;
	ssize_t read = 0;
	while((read = getline(&cstr, &len, f)) != -1) {
		if(cstr == NULL) {
			log_fatal("Something really bad happened to getline... [%s]", strerror(errno));
			return false;
		}
		std::string str = std::string(cstr);

		free(cstr);  // this will result in a bunch of reallocations but oh well
		cstr = NULL; // however now I don't need to clean anything up on every
		len = 0;     // return so I guess it's a win for me...

		if(!strncmpl(str, "INTERPOLATE")) {
			int v;
			int r = sscanf(str, "INTERPOLATE %d", &v);
			if(r != 1) {
				log_error("PIE READ [%s] INTERPOLATE line %d ret %d", path.c_str(), snum, r);
				return false;
			}
			this->interpolate = v;
		} else if(!strncmpl(str, "TEXTURE")) {
			char* texname; // technically we must free that pointer
			int r = sscanf(str, "TEXTURE %*d %ms %d %d", &texname, &this->tsizeh, &this->tsizew);
			if(r != 3) { // but what happens if that pointer never actually allocated
				log_error("PIE READ [%s] TEXTURE line %d ret %d", path.c_str(), snum, r);
				// free(texname); double free?
				return false;
			}
			this->texturename = std::string(texname);
			free(texname);
		} else if(!strncmpl(str, "NORMALMAP")) {
			char* texname;
			int r = sscanf(str, "NORMALMAP %*d %ms", &texname);
			if(r != 1) {
				log_error("PIE READ [%s] NORMALMAP line %d ret %d", path.c_str(), snum, r);
				return false;
			}
			this->normalname = std::string(texname);
			free(texname);
		} else if(!strncmpl(str, "SPECULARMAP")) {
			char* texname;
			int r = sscanf(str, "SPECULARMAP %*d %ms", &texname);
			if(r != 1) {
				log_error("PIE READ [%s] SPECULARMAP line %d ret %d", path.c_str(), snum, r);
				return false;
			}
			this->specularname = std::string(texname);
			free(texname);
		} else if(!strncmpl(str, "EVENT")) {
			int t;
			char* file;
			int r = sscanf(str, "EVENT %d %ms", &t, &file);
			if(r != 2) {
				log_error("PIE READ [%s] EVENT line %d ret %d", path.c_str(), snum, r);
				return false;
			}
			if(t < 0 || t > 3) {
				log_error("Not supported PIE [%s] EVENT %d line %d ret %d", path.c_str(), t, snum, r);
			} else {
				this->events[t] = std::string(file);
			}
			free(file);
		} else if(!strncmpl(str, "LEVELS ")) {
			int r = sscanf(str, "LEVELS %d", &this->levelscount);
			if(r != 1) {
				log_error("PIE READ [%s] LEVELS line %d ret %d", path.c_str(), snum, r);
				return false;
			}
			this->levels = (PIElevel*)malloc(sizeof(PIElevel)*this->levelscount);
			if(this->levels == NULL) {
				log_error("PIE READ [%s] LEVELS %d line %d ret %d", path.c_str(), this->levelscount, snum, r);
				log_fatal("Failed to allocate levels!!! [%s]", strerror(errno));
				return false;
			}
			for(int i = 0; i < this->levelscount; i++) {
				this->levels[i].InitAtZero();
			}
		} else if(!strncmpl(str, "LEVEL ")) {
			int newlevel = -1;
			int r = sscanf(str, "LEVEL %d", &nowlevel);
			if(r != 1) {
				log_error("PIE READ [%s] LEVEL line %d ret %d", path.c_str(), snum, r);
				return false;
			}
			nowlevel = nowlevel - 1;
			log_trace("Set level %d", nowlevel);
		} else if(!strncmpl(str, "MATERIALS ")) {
		} else if(!strncmpl(str, "SHADERS ")) {
		} else if(!strncmpl(str, "POINTS ")) {
			if(nowlevel < 0) {
				log_error("PIE READ [%s] POINTS line %d level is outside of logical range (%d)", nowlevel);
				return false;
			}
			int r = sscanf(str, "POINTS %d", &this->levels[nowlevel].pointscount);
			if(r != 1) {
				log_error("PIE READ [%s] POINTS line %d ret %d level %d", path.c_str(), snum, r, nowlevel);
				return false;
			}
			this->levels[nowlevel].points = (glm::vec3*)malloc(sizeof(glm::vec3)*this->levels[nowlevel].pointscount);
			if(this->levels[nowlevel].points == NULL) {
				log_error("PIE READ [%s] POINTS line %d level %d malloc failed, no memory left?!", path.c_str(), snum, nowlevel);
				return false;
			}
			log_trace("Allocated points for level %d at %#016x", nowlevel, this->levels[nowlevel].points);
			char* pstr = NULL;
			size_t plen = 0;
			ssize_t pread;
			for(int pointnum = 0; pointnum < this->levels[nowlevel].pointscount; pointnum++) {
				pread = getline(&pstr, &plen, f);
				if(pread == -1) {
					log_error("PIE READ [%s] POINTS line %d level %d getline failed %d %d %s", path.c_str(), snum, pread, nowlevel, errno, strerror(errno));
					free(pstr);
					return false;
				}
				int pr = sscanf(pstr, "\t%f %f %f", &this->levels[nowlevel].points[pointnum].x, &this->levels[nowlevel].points[pointnum].y, &this->levels[nowlevel].points[pointnum].z);
				if(pr != 3) {
					log_error("PIE READ [%s] POINTS line %d level %d sscanf failed %d", path.c_str(), snum, nowlevel, pr);
					free(pstr);
					return false;
				}
				snum++;
			}
			free(pstr);
		} else if(!strncmpl(str, "NORMALS ")) {
			if(nowlevel < 0) {
				log_error("PIE READ [%s] NORMALS line %d level is outside of logical range (%d)", nowlevel);
				return false;
			}
			int r = sscanf(str, "NORMALS %d", &this->levels[nowlevel].normalscount);
			if(r != 1) {
				log_error("PIE READ [%s] NORMALS line %d ret %d level %d", path.c_str(), snum, r, nowlevel);
				return false;
			}
			this->levels[nowlevel].normals = (glm::vec3*)malloc(sizeof(glm::vec3)*this->levels[nowlevel].normalscount*3);
			if(this->levels[nowlevel].normals == NULL) {
				log_error("PIE READ [%s] NORMALS line %d ret %d level %d malloc failed, no memory left?!", path.c_str(), snum, r, nowlevel);
				return false;
			}
			char* pstr = NULL;
			size_t plen = 0;
			ssize_t pread;
			for(int normalnum = 0; normalnum < this->levels[nowlevel].normalscount; normalnum++) {
				pread = getline(&pstr, &plen, f);
				if(pread != -1) {
					log_error("PIE READ [%s] NORMALS line %d level %d getline failed %d %s", path.c_str(), snum, nowlevel, errno, strerror(errno));
					free(pstr);
					return false;
				}
				int pr = sscanf(pstr, "\t%f %f %f %f %f %f %f %f %f",
						&this->levels[nowlevel].normals[normalnum*3+0].x,
						&this->levels[nowlevel].normals[normalnum*3+0].y,
						&this->levels[nowlevel].normals[normalnum*3+0].z,
						&this->levels[nowlevel].normals[normalnum*3+1].x,
						&this->levels[nowlevel].normals[normalnum*3+1].y,
						&this->levels[nowlevel].normals[normalnum*3+1].z,
						&this->levels[nowlevel].normals[normalnum*3+2].x,
						&this->levels[nowlevel].normals[normalnum*3+2].y,
						&this->levels[nowlevel].normals[normalnum*3+2].z);
				if(pr != 9) {
					log_error("PIE READ [%s] NORMALS line %d level %d sscanf failed %d", path.c_str(), snum, nowlevel, pr);
					free(pstr);
					return false;
				}
				snum++;
			}
			free(pstr);
		} else if(!strncmpl(str, "POLYGONS ")) {
			if(nowlevel < 0) {
				log_error("PIE READ [%s] POLYGONS line %d level is outside of logical range (%d)", nowlevel);
				return false;
			}
			int r = sscanf(str, "POLYGONS %d", &this->levels[nowlevel].polygonscount);
			if(r != 1) {
				log_error("PIE READ [%s] POLYGONS line %d ret %d level %d", path.c_str(), snum, r, nowlevel);
				return false;
			}
			this->levels[nowlevel].polygons = (PIEpolygon*)malloc(sizeof(PIEpolygon)*this->levels[nowlevel].polygonscount);
			if(this->levels[nowlevel].polygons == NULL) {
				log_error("PIE READ [%s] POLYGONS line %d ret %d level %d malloc failed, no memory left?!", path.c_str(), snum, r, nowlevel);
				return false;
			}
			char* pstr = NULL;
			size_t plen = 0;
			ssize_t pread;
			for(int polygonnum = 0; polygonnum < this->levels[nowlevel].polygonscount; polygonnum++) {
				pread = getline(&pstr, &plen, f);
				if(pread == -1) {
					log_error("PIE READ [%s] POLYGONS line %d level %d getline failed %d %s", path.c_str(), snum, nowlevel, errno, strerror(errno));
					free(pstr);
					return false;
				}
				int pr = sscanf(pstr, "\t%d %d", &this->levels[nowlevel].polygons[polygonnum].flags, &this->levels[nowlevel].polygons[polygonnum].pcount);
				if(pr != 2) {
					log_error("PIE READ [%s] POLYGONS line %d level %d sscanf flag failed %d", path.c_str(), snum, nowlevel, pr);
					free(pstr);
					return false;
				}
				int ffff = this->levels[nowlevel].polygons[polygonnum].flags;
				if(ffff != 200 && ffff != 4200) {
					log_error("PIE READ [%s] POLYGONS line %d level %d flag is not 200 %d", path.c_str(), snum, nowlevel, this->levels[nowlevel].polygons[polygonnum].flags);
					free(pstr);
					return false;
				}
				if(this->levels[nowlevel].polygons[polygonnum].pcount != 3) {
					log_error("PIE READ [%s] POLYGONS line %d level %d wrong polygon pcount %d", path.c_str(), snum, nowlevel, this->levels[nowlevel].polygons[polygonnum].pcount);
					free(pstr);
					return false;
				}
				int prr = sscanf(pstr, "\t%*d %*d %d %d %d", &this->levels[nowlevel].polygons[polygonnum].porder[0],
															 &this->levels[nowlevel].polygons[polygonnum].porder[1],
															 &this->levels[nowlevel].polygons[polygonnum].porder[2]);
				if(prr != 3) {
					log_error("PIE READ [%s] POLYGONS line %d level %d sscanf porder failed %d", path.c_str(), snum, nowlevel, prr);
					free(pstr);
					return false;
				}
				const char* polyformat;
				if(ffff == 4000 || ffff == 4200) {
					polyformat = "\t%*d %*d %*d %*d %*d %*d %*d %*d %*d %f %f %f %f %f %f";
				} else {
					polyformat = "\t%*d %*d %*d %*d %*d %f %f %f %f %f %f";
				}
				int prrr = sscanf(pstr, polyformat,
					&this->levels[nowlevel].polygons[polygonnum].texcoords[0],
					&this->levels[nowlevel].polygons[polygonnum].texcoords[1],
					&this->levels[nowlevel].polygons[polygonnum].texcoords[2],
					&this->levels[nowlevel].polygons[polygonnum].texcoords[3],
					&this->levels[nowlevel].polygons[polygonnum].texcoords[4],
					&this->levels[nowlevel].polygons[polygonnum].texcoords[5]);
				if(prrr != 6) {
					log_error("PIE READ [%s] POLYGONS line %d level %d sscanf texcoords failed %d", path.c_str(), snum, nowlevel, prrr);
					free(pstr);
					return false;
				}
				snum++;
			}
			free(pstr);
		} else if(!strncmpl(str, "CONNECTORS ")) {
		} else if(!strncmpl(str, "ANIMOBJECT ")) {
		}
		snum++;
	}
	return true;
}
