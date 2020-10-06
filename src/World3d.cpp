#include "World3d.h"

#include "log.hpp"

#include <stdio.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <SDL2/SDL_image.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

Object3d::Object3d() {
	GLvertexes = NULL;
	GLpos = {0.0f, 0.0f, 0.0f};
	GLrot = {0.0f, 0.0f, 0.0f};
	GLscale = 1.0f;
	UsingTexture = nullptr;
	Visible = false;
}

bool Object3d::LoadFromPIE(std::string filepath) {
	FILE* f = fopen(filepath.c_str(), "r");
	if(f == NULL) {
		log_error("Error opening file");
		return false;
	}
	int type, pointscount, ret, ver;
	char texturepagepath[512];
	ret = fscanf(f, "PIE %d\nTYPE %d\nTEXTURE %*d %s %*d %*d\nLEVELS %*d\nLEVEL %*d\nPOINTS %d\n", &ver, &type, texturepagepath, &pointscount);
	if(ret!=4) {
		log_error("PIE scanf 1 %d", ret);
		abort();
	}
	this->TexturePath = texturepagepath;
	std::vector<glm::vec3> points;
	for(int i=0; i<pointscount; i++) {
		glm::vec3 newpoint;
		ret = fscanf(f, "\t%f %f %f\n", &newpoint.x, &newpoint.y, &newpoint.z);
		if(ret!=3) {
			log_error("PIE scanf 2 %d", ret);
			abort();
		}
		points.push_back(newpoint);
	}

	int polycount;
	ret = fscanf(f, "POLYGONS %d", &polycount);
	if(ret!=1) {
		log_error("PIE scanf 3 %d", ret);
		abort();
	}
	struct PIEpolygon {
		int flags;
		int pcount;
		int porder[6];
		float texcoords[12];
	};
	std::vector<PIEpolygon> polygons;
	for(int i=0; i<polycount; i++) {
		PIEpolygon newpolygon;
		ret = fscanf(f, "\t%d %d", &newpolygon.flags, &newpolygon.pcount);
		if(ret!=2) {
			log_error("PIE scanf 4 %d (%d)", ret, i);
			abort();
		}
		for(int j=0; j<newpolygon.pcount; j++) {
			ret = fscanf(f, " %d", &newpolygon.porder[j]);
			if(ret!=1) {
				log_error("PIE scanf 5 %d (%d) (%d)", ret, i, j);
				abort();
			}
		}
		if(newpolygon.flags!=200) {
			log_error("Polygons bad flag");
			abort();
		}
		for(int j=0; j<newpolygon.pcount*2; j++) {
			ret = fscanf(f, " %f", &newpolygon.texcoords[j]);
			if(ret!=1) {
				log_error("PIE scanf 6 %d (%d) (%d)\n", ret, i, j);
				abort();
			}
		}
		polygons.push_back(newpolygon);
	}
	float TexCoordFix = 1.0f;
	if(ver != 3) {
		TexCoordFix = 4.0f;
	}
	GLvertexesCount = 0;
	for(int i=0; i<polygons.size(); i++) {
		GLvertexesCount += polygons[i].pcount * (3 + 2);
	}
	GLvertexes = (float*)malloc(GLvertexesCount*sizeof(float));
	size_t filled = 0;
	for(unsigned int i=0; i<polygons.size(); i++) {
		if(polygons[i].pcount != 3) {
			log_fatal("Polygon converter error!");
			abort();
		}
		for(int j=0; j<polygons[i].pcount; j++) {
			GLvertexes[filled+0] = points[polygons[i].porder[j]].x;
			GLvertexes[filled+1] = points[polygons[i].porder[j]].y;
			GLvertexes[filled+2] = points[polygons[i].porder[j]].z;
			GLvertexes[filled+3] = polygons[i].texcoords[j*2+0]*TexCoordFix;
			GLvertexes[filled+4] = polygons[i].texcoords[j*2+1]*TexCoordFix;
			filled += 5;
		}
	}
	fclose(f);
	return true;
}

Object3d::~Object3d() {
	if(GLvertexes)
		free(GLvertexes);
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
		glUniform1i(glGetUniformLocation(shader, "Texture"), UsingTexture->id);
	}
	//printf("%s\n", glm::to_string(GetMatrix()).c_str());
	glUniformMatrix4fv(glGetUniformLocation(shader, "Model"), 1, GL_FALSE, glm::value_ptr(GetMatrix()));
	BindVAO();
	BindVBO();
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	glDrawArrays(GL_TRIANGLES, 0, GLvertexesCount);
	glFlush();
}

void Texture::Load(std::string path, SDL_Renderer *rend) {
	this->path = path;
	log_info("Loading [%s] texture...", this->path.c_str());
	this->tex = IMG_LoadTexture(rend, this->path.c_str());
	SDL_QueryTexture(this->tex, NULL, NULL, &this->w, &this->h);
	glGenTextures(1, &GLid);
	log_info("Loaded [%s] texture.", this->path.c_str());
	return;
}

void Texture::Bind(int texid) {
	this->id = texid;
	glActiveTexture(GL_TEXTURE0+texid);
	float texw, texh;
	SDL_GL_BindTexture(this->tex, &texw, &texh);
	if(texw != 1.0f || texh != 1.0f) {
		log_warn("Texture sizes seems to be wrong: %f %f", texw, texh);
	}
	return;
}

// Search in textures, maybe we already loaded it...
Texture* World3d::GetTexture(std::string filepath) {
	for(int i=0; i<Textures.size(); i++) {
		if(Textures[i].valid && Textures[i].path == filepath) {
			return &Textures[i];
		}
	}
	return nullptr;
}

void World3d::AddObject(std::string filename, unsigned int Shader) {
	Object3d creating;
	creating.LoadFromPIE(filename);
	Texture* found = GetTexture(creating.TexturePath);
	if(found != nullptr) {
		creating.UsingTexture = found;
	} else {
		Texture newtex;
		newtex.Load(creating.TexturePath, Renderer);
		newtex.Bind(GetNextTextureId());
		Textures.push_back(newtex);
		creating.UsingTexture = &Textures[Textures.size()-1];
	}
	creating.PrepareTextureCoords();
	creating.BufferData(Shader);
	Objects.push_back(creating);
}

int World3d::GetNextTextureId() {
	return texids++;
}

void World3d::RenderAll(unsigned int shader) {
	for(auto a : Objects) {
		a.Render(shader);
	}
}
