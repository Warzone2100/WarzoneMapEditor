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

#ifndef TEXTURE_H_DEFINED
#define TEXTURE_H_DEFINED

#include <string>
#include <SDL2/SDL.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>

class Texture {
public:
	GLuint GLid = 0;
	int id = 0;
	std::string path = "";
	SDL_Texture *tex = nullptr;
	int w = -1, h = -1;
	bool valid = false;
	void Load(std::string path);
	void Load(SDL_Texture* texture);
	void Bind(int texid);
	void Bind();
	void Unbind();
	void Free();
};

#endif /* end of include guard: TEXTURE_H_DEFINED */
