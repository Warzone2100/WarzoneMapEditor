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

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "log.hpp"

void Texture::Load(std::string path, SDL_Renderer *rend) {
	this->path = path;
	log_info("Loading [%s] texture...", this->path.c_str());
	this->tex = IMG_LoadTexture(rend, this->path.c_str());
	if(this->tex == NULL) {
		log_error("Failed to load texture [%s]: %s", path.c_str(), IMG_GetError());
		abort();
	}
	SDL_QueryTexture(this->tex, NULL, NULL, &this->w, &this->h);
	glGenTextures(1, &GLid);
	log_info("Loaded [%s] texture. (w%f h%f)", this->path.c_str(), this->w, this->h);
	this->valid = true;
	return;
}

void Texture::Load(SDL_Texture* texture) {
	this->path = "";
	this->tex = texture;
	SDL_QueryTexture(this->tex, NULL, NULL, &this->w, &this->h);
	glGenTextures(1, &GLid);
}

void Texture::Bind() {
	glActiveTexture(GL_TEXTURE0+this->id);
	float texw, texh;
	if(SDL_GL_BindTexture(this->tex, &texw, &texh)) {
		log_error("Failed to bind SDL_Texture: %s", SDL_GetError());
		abort();
	}
	if(texw != 1.0f || texh != 1.0f) {
		log_warn("Texture sizes seems to be wrong: %f %f", texw, texh);
	}
	return;
}

void Texture::Bind(int texid) {
	this->id = texid;
	Bind();
	return;
}

void Texture::Unbind() {
	if(SDL_GL_UnbindTexture(this->tex)) {
		log_error("Failed to unbind SDL_Texture: %s", SDL_GetError());
	}
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
