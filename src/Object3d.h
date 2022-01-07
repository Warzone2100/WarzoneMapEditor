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

#ifndef OBJECT3D_H_DEFINED
#define OBJECT3D_H_DEFINED

#include <string>
#include "glad/glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Texture.h"

class Object3d {
public:
	float* GLvertexes;
	size_t GLvertexesCount;
	glm::vec3 GLpos;
	glm::vec3 GLrot;
	float GLscale;
	Texture* UsingTexture = nullptr;
	bool Visible;
	std::string TexturePath;
	unsigned int VAOv, VBOv;
	int RenderingMode = GL_TRIANGLES;
	bool FillTextures = true;
	Object3d();
	bool LoadFromPIE(std::string filepath);
	void PrepareTextureCoords();
	void BufferData(unsigned int shader);
	void BindVAO();
	void BindVBO();
	glm::mat4 GetMatrix();
	void Render(unsigned int shader);
	void Free();
};

#endif /* end of include guard: OBJECT3D_H_DEFINED */
