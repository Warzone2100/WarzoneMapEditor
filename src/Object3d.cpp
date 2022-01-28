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

#include "Object3d.h"

#include <vector>

#include "log.hpp"
#include "Models.h"

Object3d::Object3d() {
	GLvertexes = NULL;
	GLpos = {0.0f, 0.0f, 0.0f};
	GLrot = {0.0f, 0.0f, 0.0f};
	GLscale = 1.0f;
	UsingTexture = nullptr;
	Visible = false;
}

bool Object3d::LoadFromPIE(std::string filepath) {
	PIEmodel* model = GetModel(filepath);
	if(model == nullptr) {
		return false;
	}
	if(model->levelscount <= 0) {
		log_warn("Model have negative levels?!");
		return false;
	}
	this->TexturePath = model->texturename;
	// std::vector<glm::vec3> points;
	// struct PIEpolygon {
	// 	int flags;
	// 	int pcount;
	// 	int porder[6];
	// 	float texcoords[12];
	// };
	// std::vector<PIEpolygon> polygons;
	float TexCoordFix = 1.0f;
	if(model->ver != 3) {
		TexCoordFix = 4.0f;
	}
	GLvertexesCount = 0;
	for(long unsigned int i = 0; i < model->levels[0].polygonscount; i++) {
		GLvertexesCount += model->levels[0].polygons[i].pcount * (3 + 2);
	}
	GLvertexes = (float*)malloc(GLvertexesCount*sizeof(float));
	size_t filled = 0;
	for(unsigned int i = 0; i < model->levels[0].polygonscount; i++) {
		if(model->levels[0].polygons[i].pcount != 3) {
			log_fatal("Object3d does not support pcount other than 3, filename [%s]", filepath.c_str());
			free(GLvertexes);
			return false;
		}
		for(int j = 0; j < model->levels[0].polygons[i].pcount; j++) {
			GLvertexes[filled+0] = model->levels[0].points[model->levels[0].polygons[i].porder[j]].x;
			GLvertexes[filled+1] = model->levels[0].points[model->levels[0].polygons[i].porder[j]].y;
			GLvertexes[filled+2] = model->levels[0].points[model->levels[0].polygons[i].porder[j]].z;
			GLvertexes[filled+3] = model->levels[0].polygons[i].texcoords[j*2+0]*TexCoordFix;
			GLvertexes[filled+4] = model->levels[0].polygons[i].texcoords[j*2+1]*TexCoordFix;
			filled += 5;
		}
	}
	return true;
}

// Convert texture w/h coords into 0.0f .. 1.0f coords
void Object3d::PrepareTextureCoords() {
	for(unsigned int i=0; i<GLvertexesCount/5; i++) {
		GLvertexes[i*5+3] /= this->UsingTexture->w;
		GLvertexes[i*5+4] /= this->UsingTexture->h;
	}
}

// Makes up buffers and stores arrays
void Object3d::BufferData(unsigned int shader) {
	glGenVertexArrays(1, &VAOv);
	glGenBuffers(1, &VBOv);
	BindVAO();
	BindVBO();
	// for(int i=0; i<GLvertexesCount; i+=5) {
	// 	printf("%f %f %f %f %f\n", GLvertexes[i], GLvertexes[i+1], GLvertexes[i+2], GLvertexes[i+3], GLvertexes[i+4]);
	// }
	glBufferData(GL_ARRAY_BUFFER, GLvertexesCount*sizeof(float), GLvertexes, GL_STATIC_DRAW);
	glVertexAttribPointer(glGetAttribLocation(shader, "VertexCoordinates"), 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(glGetAttribLocation(shader, "VertexCoordinates"));
	glVertexAttribPointer(glGetAttribLocation(shader, "TextureCoordinates"), 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(glGetAttribLocation(shader, "TextureCoordinates"));
}

void Object3d::BindVAO() {
	glBindVertexArray(VAOv);
}
void Object3d::BindVBO() {
	glBindBuffer(GL_ARRAY_BUFFER, VBOv);
}

glm::mat4 Object3d::GetMatrix() {
	return glm::translate(glm::mat4(1), -GLpos) *
		glm::rotate(glm::mat4(1), glm::radians(-GLrot[0]), glm::vec3(1, 0, 0)) *
		glm::rotate(glm::mat4(1), glm::radians(-GLrot[1]), glm::vec3(0, 1, 0)) *
		glm::rotate(glm::mat4(1), glm::radians(-GLrot[2]), glm::vec3(0, 0, 1)) *
		glm::mat4(1);
}

void Object3d::Render(unsigned int shader) {
	if(UsingTexture != nullptr) {
		UsingTexture->Bind(UsingTexture->id);
		glUniform1i(glGetUniformLocation(shader, "Texture"), UsingTexture->id);
	}
	glUniformMatrix4fv(glGetUniformLocation(shader, "Model"), 1, GL_FALSE, glm::value_ptr(GetMatrix()));
	BindVAO();
	BindVBO();
	if(FillTextures) {
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	} else {
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	}
	glDrawArrays(RenderingMode, 0, GLvertexesCount);
	glFlush();
	if(UsingTexture != nullptr) {
		UsingTexture->Unbind();
	}
}

void Object3d::RenderColorPick(unsigned int shader) {
	uint8_t cr = this->pickid & 0x000000ff;
	uint8_t cg = (this->pickid & 0x0000ff00) >> 8;
	uint8_t cb = (this->pickid & 0x00ff0000) >> 16;
	uint8_t ca = (this->pickid & 0xff000000) >> 24;
	glUniform4ui(glGetUniformLocation(shader, "Color"), cr, cg, cb, 128);
	glUniformMatrix4fv(glGetUniformLocation(shader, "Model"), 1, GL_FALSE, glm::value_ptr(GetMatrix()));
	BindVAO();
	BindVBO();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(RenderingMode, 0, GLvertexesCount);
	glFlush();
}

void Object3d::Free() {
	if(GLvertexes) {
		free(GLvertexes);
	}
}
