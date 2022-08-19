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

#include "tileset.h"

#include <dirent.h>
#include <SDL2/SDL_image.h>
#include <errno.h>

#include "other.h"

int GetTerrainTilesetNumber(WZtileset t);
const char* TerrainTilesetToString(WZtileset t);

// Accepts path to directory with textures and qual as quality of tiles
// qual can be 16, 32, 64 or 128
void Tileset::CreateTexturePage(char* basepath, int qual, SDL_Renderer* rend) {
	log_info("Loading tiles");
	int tilesetnum = GetTerrainTilesetNumber(tileset);
	char* folderpath = sprcatr(NULL, "%stexpages/tertilesc%dhw-%d/", basepath, tilesetnum, qual);
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
		SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
		textsa[TotalTextures].n = atoi(strtileid);
		textsa[TotalTextures].t = t;
		TotalTextures++;
		free(strtileid);
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
	GroundTilePage = new Texture();
	SDL_Texture* tex = SDL_CreateTexture(rend, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET | SDL_TEXTUREACCESS_STREAMING, TotalTextures*mw, mh);
	if(tex == NULL) {
		log_fatal("SDL failed to create texture: %s", SDL_GetError());
		abort();
	}
	GroundTilePage->w = TotalTextures*mw;
	GroundTilePage->h = mh;
	DatasetLoaded = TotalTextures;
	SDL_Texture* savedt = SDL_GetRenderTarget(rend);
	SDL_SetRenderTarget(rend, tex);
	SDL_RenderClear(rend);
	for(int i=0; i<TILEGROUNDSMAX; i++) {
		int pn = -1;
		for(int j=0; j<TotalTextures; j++) {
			if(textsa[j].n == i) {
				pn = j;
				break;
			}
		}
		if (pn == -1)
			continue;
		SDL_Rect from = {.x=0, .y=0, .w=textsa[pn].w, .h=textsa[pn].h};
		SDL_Rect to = {.x=i*mw, .y=0, .w=textsa[pn].w, .h=textsa[pn].h};
		for (int j = 0; j < 4; j++) {
			SDL_Rect toQ = to;
			toQ.w /= 2; toQ.h /= 2;
			if ((j & 1))
				toQ.x += toQ.w;
			if ((j & 2))
				toQ.y += toQ.h;
			int gt = TileGrounds[i].groundTypes[j];
			if ((gt >= 0) && (gt < GTYPESMAX) && (gtypes[gt].tex))
				SDL_RenderCopy(rend, gtypes[gt].tex, &from, &toQ);
		}
		SDL_RenderCopy(rend, textsa[pn].t, &from, &to);
	}
	for(int i=0; i<TotalTextures; i++) {
		SDL_DestroyTexture(textsa[i].t);
	}
	SDL_SetRenderTarget(rend, savedt);
	GroundTilePage->Load(tex);
	log_info("Loaded terrain texpage to %d", GroundTilePage->GLid);
	// GroundTilePage->Bind(0);
	log_info("Tiles max resolution %dx%d", mw, mh);
	free(folderpath);
}

void Tileset::LoadTerrainGrounds(char* basepath) {
	if(basepath == NULL) {
		log_fatal("Base path is null!");
		return;
	}
	char* filename = sprcatr(NULL, "%stileset/%sground.txt", basepath, TerrainTilesetToString(this->tileset));
	if(filename == NULL) {
		log_fatal("Terrain grounds filename generated is null!");
	}
	FILE* f = fopen(filename, "r");
	if(f == NULL) {
		log_fatal("Failed to open [%s]", filename);
		free(filename);
		return;
	}
	int count = -1;
	while (fgetc(f) > 44); // skip to the comma because fscanf keeps failing
	int ret = fscanf(f, "%d\n", &count);
	if(ret != 1) {
		log_error("*ground.txt header fscanf failed with %d fields readed instead of 1", ret);
	}
	char lineBuf[80];
	for(int i=0; i<count; i++) {
		char* b = fgets(lineBuf, 80, f);
		if(b == NULL) {
			log_fatal("fgets failed (%d)", strerror(errno));
			abort();
		}
		const char* scanfmt = "%23[^,],%23[^,],%23[^,],%23[^\n]";
		int scanned = sscanf(b, scanfmt, TileGrounds[i].names[0], TileGrounds[i].names[1], TileGrounds[i].names[2], TileGrounds[i].names[3]);
		if(scanned != 4) {
			log_fatal("sscanf failed, scanned %d out of 4", scanned);
			log_fatal("parsing line [%s] with format [%s]", b, scanfmt);
			abort();
		}
	}
	fclose(f);
	free(filename);
}

void Tileset::LoadTerrainGroundTypes(char *basepath, SDL_Renderer* rend) {
	if(basepath == NULL) {
		log_fatal("Base path is null!");
		return;
	}
	int tilesetnum = GetTerrainTilesetNumber(this->tileset);
	char* filename = sprcatr(NULL, "%stileset/tertilesc%dhwGtype.txt", basepath, tilesetnum);
	if(filename == NULL) {
		log_fatal("Terrain ground types filename generated is null!");
	}
	FILE* f = fopen(filename, "r");
	if(f == NULL) {
		log_fatal("Failed to open [%s]", filename);
		free(filename);
		return;
	}
	int r = -2;
	int typesnum = -1;
	while (fgetc(f) > 44); // skip to the comma because fscanf keeps failing
	r = fscanf(f, "%d\n", &typesnum);
	if(r != 1) {
		log_error("hwGtype header fscanf failed, returned %d", r);
	}
	if(typesnum > GTYPESMAX) {
		log_warn("Too many gtypes loading, turncating %d -> %d", typesnum, GTYPESMAX);
		typesnum = GTYPESMAX;
	}
	log_info("Loading %d terrain types...", typesnum);
	gtypescount = typesnum;
	char lineBuf[80];
	for(int i=0; i<typesnum; i++) {
		char* b = fgets(lineBuf, 80, f);
		if(b == NULL) {
			log_fatal("fgets failed (%d)", strerror(errno));
			abort();
		}
		char n[20];
		memset(n, 0, 20);
		const char* scanfmt = "%79[^,],%255[^,],%19[^\n]";
		int scanned = sscanf(b, scanfmt, gtypes[i].groundtype, gtypes[i].pagename, n);
		if(scanned != 3) {
			log_fatal("sscanf failed, scanned %d out of 3", scanned);
			log_fatal("parsing line [%s] with format [%s]", b, scanfmt);
			abort();
		}
		gtypes[i].size = atof(n);
	}
	fclose(f);
	free(filename);
	// TODO free this
	log_info("Ground types loaded.");
	LoadGroundTypesTextures(basepath, rend);
}

void Tileset::AssociateGroundTypesWithTileGrounds() {
	for (int i = 0; i < TILEGROUNDSMAX; i++) {
		auto & tg = TileGrounds[i];
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < gtypescount; k++) {
				auto & gt = gtypes[k];
				if (!strcmp(tg.names[j], gt.groundtype)) {
					// log_info("Association %i : GT %s", i, gt.groundtype);
					tg.groundTypes[j] = k;
					break;
				}
			}
		}
	}
}

void Tileset::LoadGroundTypesTextures(char* basepath, SDL_Renderer* rend) {
	for(int i=0; i<gtypescount; i++) {
		char* path = sprcatr(NULL, "%stexpages/%s", basepath, gtypes[i].pagename);
		log_debug("%02d Loading page [%s]", i, path);
		gtypes[i].tex = IMG_LoadTexture(rend, path);
		if (!gtypes[i].tex) {
			log_error("Failed to load ground type %i's page [%s], expect oddities", i, path);
			log_error("Details: %s (%s)", IMG_GetError(), strerror(errno));
		}
		free(path);
	}
	log_info("Ground textures loaded");
	// TODO free this
}

