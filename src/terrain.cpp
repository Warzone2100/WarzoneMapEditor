#include "terrain.h"

#include <dirent.h>
#include <SDL2/SDL_image.h>
#include <errno.h>

#include "other.h"

void Terrain::GetHeightmapFromMWT(WZmap* map) {
	if(!map->valid) {
		log_error("WMT failed to read map!");
		return;
	}
	tileset = map->tileset;
	w = map->maptotalx;
	h = map->maptotaly;
	RenderingMode = GL_TRIANGLES;
	GLvertexesCount = (h-1)*(w-1)*2*3*8;
	GLvertexes = (float*)malloc(GLvertexesCount*sizeof(float));
	size_t filled = 0;
	auto addTriangle = [&] (float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3) {
		GLvertexes[filled+0] = x1*128;
		GLvertexes[filled+1] = y1*128;
		GLvertexes[filled+2] = z1*128;
		GLvertexes[filled+3] = 0;
		GLvertexes[filled+4] = 0;
		GLvertexes[filled+5] = 0;
		GLvertexes[filled+6] = 0;
		GLvertexes[filled+7] = 0;
		GLvertexes[filled+8] = 0;
		filled+=9;
		GLvertexes[filled+0] = x2*128;
		GLvertexes[filled+1] = y2*128;
		GLvertexes[filled+2] = z2*128;
		GLvertexes[filled+3] = 0;
		GLvertexes[filled+4] = 0;
		GLvertexes[filled+5] = 0;
		GLvertexes[filled+6] = 0;
		GLvertexes[filled+7] = 0;
		GLvertexes[filled+8] = 0;
		filled+=9;
		GLvertexes[filled+0] = x3*128;
		GLvertexes[filled+1] = y3*128;
		GLvertexes[filled+2] = z3*128;
		GLvertexes[filled+3] = 0;
		GLvertexes[filled+4] = 0;
		GLvertexes[filled+5] = 0;
		GLvertexes[filled+6] = 0;
		GLvertexes[filled+7] = 0;
		GLvertexes[filled+8] = 0;
		filled+=9;
	};
	float scale = 32.0f;
	for(int y=0; y<h; y++) {
		for(int x=0; x<w; x++) {
			tiles[x][y].height = (float)map->mapheight[y*w+x]/scale;
			tiles[x][y].triflip = WMT_TileGetTriFlip(map->maptile[y*w+x]);
			tiles[x][y].texture = WMT_TileGetTexture(map->maptile[y*w+x]);
			tiles[x][y].rot = WMT_TileGetRotation(map->maptile[y*w+x]);
			tiles[x][y].fx = WMT_TileGetXFlip(map->maptile[y*w+x]);
			tiles[x][y].fy = WMT_TileGetYFlip(map->maptile[y*w+x]);
			tiles[x][y].tt = WMT_TileGetTerrainType(map->maptile[y*w+x], map->ttyptt);
		}
	}
	for(int y=0; y<h-1; y++) {
		for(int x=0; x<w-1; x++) {
			// 0 1
			// 3 2
			if(WMT_TileGetTriFlip(map->maptile[y*w+x])) {
				// 1 2
				// 0
				//
				//   5
				// 3 4
				addTriangle(x,   tiles[x  ][y+1].height, y+1,
							x,   tiles[x  ][y  ].height, y,
							x+1, tiles[x+1][y  ].height, y);
				addTriangle(x,   tiles[x  ][y+1].height, y+1,
							x+1, tiles[x+1][y+1].height, y+1,
							x+1, tiles[x+1][y  ].height, y);
			} else {
				// 0 1
				//   2
				//
				// 3
				// 4 5
				addTriangle(x,   tiles[x  ][y  ].height, y,
							x+1, tiles[x+1][y  ].height, y,
							x+1, tiles[x+1][y+1].height, y+1);
				addTriangle(x,   tiles[x  ][y  ].height, y,
							x,   tiles[x  ][y+1].height, y+1,
							x+1, tiles[x+1][y+1].height, y+1);
			}
		}
	}
	// FillTextures = false;
	return;
}

// Accepts path to directory with textures and qual as quality of tiles
// qual can be 16, 32, 64 or 128
void Terrain::CreateTexturePage(char* basepath, int qual, SDL_Renderer* rend) {
	log_trace("Loading tiles");
	int tilesetnum;
	switch(tileset) {
		case tileset_arizona:
		tilesetnum = 1;
		break;
		case tileset_urban:
		tilesetnum = 2;
		break;
		case tileset_rockies:
		tilesetnum = 3;
		break;
	}
	char* folderpath = sprcatr(NULL, "%stertilesc%dhw-%d/", basepath, tilesetnum, qual);
	log_trace("Folder path to search tiles: [%s]", folderpath);
	int TotalTextures = 0;
	struct TempTextures {
		int n;
		SDL_Texture* t;
		int w, h;
	} textsa[512];
	DIR *d;
	struct dirent *dir;
	d = opendir(folderpath);
	if(!d) {
		log_error("Can not open directory for listing");
		return;
	}
	log_info("Loading tiles from [%s]", folderpath);
	while ((dir = readdir(d)) != NULL) {
		if(equalstr(dir->d_name, ".") || equalstr(dir->d_name, "..")) {
			continue;
		}
		size_t strtileidlen = 512;
		char* strtileid = (char*)malloc(strtileidlen);
		if(sscanf(dir->d_name, "tile-%500[0-9].pn%1[g]%c", strtileid, strtileid+strtileidlen-2, strtileid+strtileidlen-2) != 2) {
			log_warn("Found non-matching file [%s] in texpages directory.", dir->d_name);
			continue;
		}
		char* filep = sprcatr(NULL, "%s%s", folderpath, dir->d_name);
		SDL_Texture* t = IMG_LoadTexture(rend, filep);
		if(t == NULL) {
			log_error("Error opening [%s] tile!", filep);
			log_error("Details: %s (%s)", IMG_GetError(), strerror(errno));
			continue;
		}
		free(filep);
		if(TotalTextures == 511) {
			log_fatal("Yo wtf, more than 512 tiles? Crazy...");
			break;
		}
		textsa[TotalTextures].n = atoi(strtileid);
		textsa[TotalTextures].t = t;
		TotalTextures++;
	}
	closedir(d);
	log_info("Loaded %d tiles.", TotalTextures);
	int mw = 0, mh = 0;
	for(int i=0; i<TotalTextures; i++) {
		SDL_QueryTexture(textsa[i].t, NULL, NULL, &textsa[i].w, &textsa[i].h);
		if(textsa[i].w > mw) {
			mw = textsa[i].w;
		}
		if(textsa[i].h > mh) {
			mh = textsa[i].h;
		}
	}
	UsingTexture = new Texture;
	UsingTexture->tex = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, TotalTextures*mw, mh);
	UsingTexture->w = TotalTextures*mw;
	UsingTexture->h = mh;
	DatasetLoaded = TotalTextures;
	SDL_Texture* savedt = SDL_GetRenderTarget(rend);
	SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
	SDL_SetRenderTarget(rend, UsingTexture->tex);
	SDL_RenderClear(rend);
	for(int i=0; i<TotalTextures; i++) {
		int pn = -1;
		for(int j=0; j<TotalTextures; j++) {
			if(textsa[j].n == i) {
				pn = j;
				break;
			}
		}
		SDL_Rect from = {.x=0, .y=0, .w=textsa[pn].w, .h=textsa[pn].h};
		SDL_Rect to = {.x=i*mw, .y=0, .w=textsa[pn].w, .h=textsa[pn].h};
		SDL_RenderCopy(rend, textsa[pn].t, &from, &to);
		SDL_DestroyTexture(textsa[pn].t);
	}
	SDL_SetRenderTarget(rend, savedt);
	log_info("Tiles max resolution %dx%d", mw, mh);
	free(folderpath);
}

void Terrain::LoadTerrainGrounds(char *basepath) {
	if(basepath == NULL) {
		log_fatal("Base path is null!");
		return;
	}
	int tilesetnum;
	switch(tileset) {
		case tileset_arizona:
		tilesetnum = 1;
		break;
		case tileset_urban:
		tilesetnum = 2;
		break;
		case tileset_rockies:
		tilesetnum = 3;
		break;
	}
	char* filename = sprcatr(NULL, "%stileset/tertilesc%dhwGtype.txt", tilesetnum);
	if(filename == NULL) {
		log_fatal("Terrain ground filename generated is null!");
	}
	FILE* f = fopen(filename, "r");
	if(f == NULL) {
		log_fatal("Failed to open [%s]", filename);
		free(filename);
		return;
	}
	int r = -2;
	int datasetnum = -1, typesnum = -1;
	r = fscanf(f, "tertilesc%dhw,%d", &datasetnum, &typesnum);
	if(r != 2) {
		log_error("scanf failed with %d fields readed instead of %d");
	}
	if(datasetnum != tilesetnum) {
		log_error("File [%s] reported dataset number %d instead of expected %d.", filename, datasetnum, tilesetnum);
	}
	if(typesnum > GTYPESMAX) {
		log_warn("Too many gtypes loading, turncating %d -> %d", typesnum, GTYPESMAX);
		typesnum = GTYPESMAX;
	}
	log_info("Loading %d terrain types...", typesnum);
	for(int i=0; i<typesnum; i++) {
		char tmp[14] = {0};
		r = fscanf(f, "%[^,]%[^,]%[^,]", gtypes[i].groundtype, gtypes[i].pagename, tmp);
		if(r != 3) {
			log_error("fscanf readed %d fields instead of %d on %d element.", r, 3, i);
		}
		gtypes[i].size = atof(tmp);
	}
}

void Terrain::UpdateTexpageCoords() {
	int filled = 0;
	int tw = UsingTexture->w/DatasetLoaded;
	log_info("%d %d %d", tw, UsingTexture->w, DatasetLoaded);
	// // int th = UsingTexture->h;
	auto SetNextTriangle = [&] (float c[2]) {
		GLvertexes[filled+3] = c[0];
		GLvertexes[filled+4] = c[1];
		GLvertexes[filled+5] = 1.0f;
		filled+=9;
	};
	auto SetNextTile = [&] (int j[6], float t[4][2]) {
		for(int i=0; i<6; i++) {
			SetNextTriangle(t[j[i]]);
		}
	};
	for(int y=0; y<h-1; y++) {
		for(int x=0; x<w-1; x++) {
			// 0 1
			// 3 2
			float tex0[4][2] = {{(tiles[x][y].texture+0)/(float)DatasetLoaded, 0.0f},
								{(tiles[x][y].texture+1)/(float)DatasetLoaded, 0.0f},
								{(tiles[x][y].texture+1)/(float)DatasetLoaded, 1.0f},
								{(tiles[x][y].texture+0)/(float)DatasetLoaded, 1.0f}};
			int tord[6];
			if(tiles[x][y].triflip) {
				// 1 2
				// 0
				//
				//   5
				// 3 4
				tord[0] = 3;
				tord[1] = 0;
				tord[2] = 1;
				tord[3] = 3;
				tord[4] = 2;
				tord[5] = 1;
			} else {
				// 0 1
				//   2
				//
				// 3
				// 4 5
				tord[0] = 0;
				tord[1] = 1;
				tord[2] = 2;
				tord[3] = 0;
				tord[4] = 3;
				tord[5] = 2;
			}
			for(int numrot = 0; numrot<tiles[x][y].rot; numrot++) {
				for(int i=0; i<6; i++) {
					tord[i]--;
					if(tord[i] == -1) {
						tord[i] = 3;
					}
				}
			}
			if(tiles[x][y].fx) {
				for(int i=0; i<6; i++) {
					if(tord[i] == 0) {
						tord[i] = 1;
					} else if(tord[i] == 1) {
						tord[i] = 0;
					} else if(tord[i] == 2) {
						tord[i] = 3;
					} else if(tord[i] == 3) {
						tord[i] = 2;
					}
				}
			}
			if(tiles[x][y].fy) {
				for(int i=0; i<6; i++) {
					if(tord[i] == 0) {
						tord[i] = 3;
					} else if(tord[i] == 3) {
						tord[i] = 0;
					} else if(tord[i] == 2) {
						tord[i] = 1;
					} else if(tord[i] == 1) {
						tord[i] = 2;
					}
				}
			}
			SetNextTile(tord, tex0);
		}
	}
}

// Makes up buffers and stores arrays
void Terrain::BufferData(unsigned int shader) {
	glGenVertexArrays(1, &VAOv);
	glGenBuffers(1, &VBOv);
	BindVAO();
	BindVBO();
	glBufferData(GL_ARRAY_BUFFER, GLvertexesCount*sizeof(float), GLvertexes, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(shader, "VertexCoordinates"), 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(glGetAttribLocation(shader, "VertexCoordinates"));
	glVertexAttribPointer(glGetAttribLocation(shader, "TextureCoordinates"), 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(glGetAttribLocation(shader, "TextureCoordinates"));
	glVertexAttribPointer(glGetAttribLocation(shader, "TextureCliffCoordinates"), 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(glGetAttribLocation(shader, "TextureCliffCoordinates"));
}
