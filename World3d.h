#ifndef WORLD3D_H_INCLUDED
#define WORLD3D_H_INCLUDED

#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Texture {
public:
	unsigned int GLid;
	std::string path;
	SDL_Texture *tex;
	int w, h;
	bool valid;
	Texture(std::string path, SDL_Renderer* rend);
	void GenerateTexture();
};

class Object3d {
public:
	struct PIEpolygon {
		int flags;
		int pcount;
		int porder[6];
		float texcoords[12];
	};
	std::vector<float> GLvertexes;
	glm::vec3 GLpos;
	glm::vec3 GLrot;
	float GLscale;
	Texture* UsingTexture;
	bool Visible;
	std::string TexturePath;
	Object3d();
	bool LoadFromPIE(std::string filepath);
};

class World3d {
public:
	std::vector<Object3d> Objects;
	std::vector<Texture> Textures;
	SDL_Renderer *Renderer;
	void AddTexture(std::string filepath);
	Texture* GetTexture(std::string filepath);
	void AddObject(std::string filename);
};

#endif /* end of include guard: WORLD3D_H_INCLUDED */
