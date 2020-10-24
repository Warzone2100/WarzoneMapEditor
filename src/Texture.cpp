#include "Texture.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "log.hpp"

void Texture::Load(std::string path, SDL_Renderer *rend) {
	this->path = path;
	log_info("Loading [%s] texture...", this->path.c_str());
	this->tex = IMG_LoadTexture(rend, this->path.c_str());
	SDL_QueryTexture(this->tex, NULL, NULL, &this->w, &this->h);
	glGenTextures(1, &GLid);
	log_info("Loaded [%s] texture.", this->path.c_str());
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
