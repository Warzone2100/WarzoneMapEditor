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
			tiles[x][y].height = map->mapheight[y*w+x]/scale;
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
			if(WMT_TileGetTriFlip(map->maptile[y*w+x])) {
				addTriangle(x,   tiles[x  ][y  ].height, y,
							x,   tiles[x  ][y+1].height, y+1,
							x+1, tiles[x+1][y  ].height, y);
				addTriangle(x+1, tiles[x+1][y  ].height, y,
							x+1, tiles[x+1][y+1].height, y+1,
							x,   tiles[x  ][y+1].height, y+1);
			} else {
				addTriangle(x,   tiles[x  ][y  ].height, y,
							x,   tiles[x  ][y+1].height, y+1,
							x+1, tiles[x+1][y+1].height, y+1);
				addTriangle(x,   tiles[x  ][y  ].height, y,
							x+1, tiles[x+1][y  ].height, y,
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
	SDL_SetRenderTarget(rend, UsingTexture->tex);
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

void Terrain::UpdateTexpageCoords() {
	int filled = 0;
	int tw = UsingTexture->w/DatasetLoaded;
	// // int th = UsingTexture->h;
	// for(int y=0; y<h-1; y++) {
	// 	for(int x=0; x<w-1; x++) {
	// 		if(tiles[x][y].triflip) {
				GLvertexes[filled+3] = (tiles[0][0].texture/DatasetLoaded);
				GLvertexes[filled+4] = 0;
				filled+=5;
				GLvertexes[filled+3] = ((tiles[0][0].texture+1)/DatasetLoaded);
				GLvertexes[filled+4] = 0;
				filled+=5;
				GLvertexes[filled+3] = (tiles[0][0].texture/DatasetLoaded);
				GLvertexes[filled+4] = 1;
				filled+=5;
				// filled+=15;
	// 		}
	// 	}
	// }
}
