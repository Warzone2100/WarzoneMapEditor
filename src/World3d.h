#ifndef WORLD3D_H_INCLUDED
#define WORLD3D_H_INCLUDED

#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

class World3d {
private:
	int texids = 0;
	int GetNextTextureId();
	Texture* GetTexture(std::string filepath);
public:
	std::vector<Object3d> Objects;
	std::vector<Texture> Textures;
	SDL_Renderer *Renderer;
	void AddObject(std::string filename, unsigned int);
	void RenderAll(unsigned int shader);
};

#endif /* end of include guard: WORLD3D_H_INCLUDED */
