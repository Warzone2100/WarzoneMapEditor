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

#include "Texture.h"
#include "other.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <execinfo.h>
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "log.hpp"

void Texture::Load(std::string path) {
	this->path = path;
	log_info("Loading [%s] texture...", this->path.c_str());
	stbi_set_flip_vertically_on_load(true);
	int lw, lh, nrChannels;
	unsigned char *data = stbi_load(path.c_str(), &lw, &lh, &nrChannels, STBI_rgb_alpha);
	if(data == NULL) {
		log_error("stbi load texture fail");
	}

	// log_info("Bytes: %2x %2x %2x %2x %2x %2x %2x %2x", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]);

	unsigned int gl = 0;
	glGenTextures(1, &gl);
	this->GLid = gl;

	// unsigned char* d = malloc()

	// const char* fname = sprcatr(NULL, "texture%d", GLid);
	// FILE* f = fopen(fname, "w");
	// fprintf(f, "P3\n%d %d\n255\n", lw, lh);
	// for(int dh = 0; dh < lh; dh++) {
	// 	for(int dw = 0; dw < lw; dw++) {
	// 		fprintf(f, "%d %d %d\n", data[(lw*dh+dw)*4], data[(lw*dh+dw)*4+1], data[(lw*dh+dw)*4+2]);
	// 	}
	// }
	// fclose(f);

	log_info("Channels: %d", nrChannels);
	glBindTexture(GL_TEXTURE_2D, this->GLid); 
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, lw, lh, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	log_info("Loaded [%s] texture. w%d h%d GLID %d", this->path.c_str(), lw, lh, this->GLid);
	this->w = lw;
	this->h = lh;
	this->valid = true;
	return;
}

void Texture::Load(SDL_Texture* texture) {
	this->path = "";
	this->tex = texture;
	if(SDL_QueryTexture(this->tex, NULL, NULL, &this->w, &this->h)) {
		log_error("Failed to query texture: %s", SDL_GetError());
	}
	void* pixels = NULL;
	int pitch = 0;
	if(SDL_LockTexture(this->tex, NULL, &pixels, &pitch)) {
		log_error("Failed to lock texture: %s", SDL_GetError());
	}
	log_info("Texture pitch: %d", pitch);
	glGenTextures(1, &GLid);
	glBindTexture(GL_TEXTURE_2D, this->GLid); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->w, this->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	log_info("Loaded texture from memory GLID %d", GLid);
	this->valid = true;
}

void Texture::Bind() {
	// glActiveTexture(GL_TEXTURE0+this->id);
	// float texw, texh;
	// if(SDL_GL_BindTexture(this->tex, &texw, &texh)) {
	// 	log_error("Failed to bind SDL_Texture: %s", SDL_GetError());
	// 	abort();
	// }
	// if(texw != 1.0f || texh != 1.0f) {
	// 	log_warn("Texture sizes seems to be wrong: %f %f", texw, texh);
	// }
	// // glGenerateMipmap(GL_TEXTURE_2D);
	// // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP);

	// glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR);
	// glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR);
	// return;
}

void Texture::Bind(int texid) {
	// this->id = texid;
	// Bind();
	return;
}

void Texture::Unbind() {
	// if(SDL_GL_UnbindTexture(this->tex)) {
	// 	log_error("Failed to unbind SDL_Texture: %s (texture ID was %d, GLid was %d)", SDL_GetError(), this->id, this->GLid);
	// }
	return;
}

void Texture::Free() {
	if(tex != nullptr) {
		Unbind();
		SDL_DestroyTexture(tex);
		glDeleteTextures(1, &GLid);
	}
	return;
}
