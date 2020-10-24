#ifndef TEXTURE_H_DEFINED
#define TEXTURE_H_DEFINED

#include <string>
#include <SDL2/SDL.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>

class Texture {
public:
	unsigned int GLid = 0;
	int id = 0;
	std::string path = "";
	SDL_Texture *tex = nullptr;
	int w = -1, h = -1;
	bool valid = false;
	void Load(std::string path, SDL_Renderer *rend);
	void Load(SDL_Texture* texture);
	void Bind(int texid);
	void Bind();
	void Unbind();
	void Free();
};

#endif /* end of include guard: TEXTURE_H_DEFINED */
