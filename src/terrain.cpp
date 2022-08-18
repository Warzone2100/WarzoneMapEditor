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

#include "terrain.h"

#include <dirent.h>
#include <SDL2/SDL_image.h>
#include <errno.h>

#include "other.h"

void Terrain::CreateShader() {
	TerrainShader = new Shader("./data/shaders/TerrainShaderVertex.vs", "./data/shaders/TerrainShaderFragment.frag");
}

void Terrain::GetHeightmapFromMWT(WZmap* map) {
	if(!map->valid) {
		log_error("WMT failed to read map!");
		return;
	}
	w = map->maptotalx;
	h = map->maptotaly;
	RenderingMode = GL_TRIANGLES;
	GLvertexesCount = ((h-1)*(w-1)*2)*3*3*3;
	glVerticesTerrain = (wme_terrain_glvertex_t *) malloc(GLvertexesCount*sizeof(wme_terrain_glvertex_t));
	wme_terrain_glvertex_t * filled = glVerticesTerrain;
	auto addTriangle = [&] (float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3) {
		// float* realloced = (float*)realloc(GLvertexes, (GLvertexesCount+9*3)*sizeof(float));
		// if(realloced == NULL) {
		// 	log_fatal("Realloc gived up at %ld", (GLvertexesCount+9*3)*sizeof(float));
		// 	abort();
		// }
		if((filled - glVerticesTerrain) >= GLvertexesCount) {
			log_fatal("Terrain vertex buffer overflow");
		}
		filled->x = x1*128;
		filled->y = y1*128;
		filled->z = z1*128;
		filled++;

		filled->x = x2*128;
		filled->y = y2*128;
		filled->z = z2*128;
		filled++;

		filled->x = x3*128;
		filled->y = y3*128;
		filled->z = z3*128;
		filled++;
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
	log_info("WMT map exported.");
	return;
}

int GetTerrainTilesetNumber(WZtileset t) {
	switch(t) {
		case tileset_arizona:
		return 1;
		case tileset_urban:
		return 2;
		case tileset_rockies:
		return 3;
	}
	return -1;
}

const char* TerrainTilesetToString(WZtileset t) {
	switch(t) {
		case tileset_arizona:
		return "arizona";
		case tileset_urban:
		return "urban";
		case tileset_rockies:
		return "rockie";
	}
	return "unknown";
}

void Terrain::ConstructGroundAlphas() {
	if(groundalphas) {
		free(groundalphas);
	}
	groundalphas = NULL;
	size_t layersize = w*h*sizeof(float);
	groundalphas = (float*)malloc(layersize*gtypescount);
	if(groundalphas == NULL) {
		log_fatal("Allocation fail");
		exit(1);
	}
	for(int i=0; i<gtypescount; i++) {

	}
}

void Terrain::UpdateTexpageCoords() {
	int filled = 0;
	int tileCount = TilesetPtr->DatasetLoaded;
	auto tilePage = TilesetPtr->GroundTilePage;
	int tw = tilePage->w/tileCount;
	log_info("%d %d %d", tw, tilePage->w, tileCount);
	// // int th = tilePage->h;
	auto SetNextTriangle = [&] (float c[2]) {
		auto vtx = glVerticesTerrain + filled;
		vtx->ux = c[0];
		vtx->uy = c[1];
		filled++;
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
			float tex0[4][2] = {{(tiles[x][y].texture+0)/(float)tileCount, 0.0f},
								{(tiles[x][y].texture+1)/(float)tileCount, 0.0f},
								{(tiles[x][y].texture+1)/(float)tileCount, 1.0f},
								{(tiles[x][y].texture+0)/(float)tileCount, 1.0f}};
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
void Terrain::BufferData() {
	int shader = this->TerrainShader->program;
	glGenVertexArrays(1, &VAOv);
	glGenBuffers(1, &VBOv);
	BindVAO();
	BindVBO();
	// see wme_terrain_glvertex_t
	glBufferData(GL_ARRAY_BUFFER, GLvertexesCount*sizeof(wme_terrain_glvertex_t), glVerticesTerrain, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(shader, "VertexCoordinates"), 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(glGetAttribLocation(shader, "VertexCoordinates"));
	glVertexAttribPointer(glGetAttribLocation(shader, "TextureCoordinates"), 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(glGetAttribLocation(shader, "TextureCoordinates"));
}

void Terrain::RenderV(glm::mat4 view) {
	this->TerrainShader->use();
	glUniformMatrix4fv(glGetUniformLocation(this->TerrainShader->program, "ViewProjection"), 1, GL_FALSE, glm::value_ptr(view));
	this->Render();
}

void Terrain::Render() {
	int shader = this->TerrainShader->program;
	auto tilePage = TilesetPtr->GroundTilePage;
	tilePage->Bind(tilePage->GLid);
	glUniform1i(glGetUniformLocation(shader, "Texture"), tilePage->GLid);
	glUniformMatrix4fv(glGetUniformLocation(shader, "Model"), 1, GL_FALSE, glm::value_ptr(GetMatrix()));
	BindVAO();
	BindVBO();
	if(FillTextures) {
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	} else {
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	}
	glDrawArrays(RenderingMode, 0, GLvertexesCount);
	tilePage->Unbind();
}
