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

#include "wmt.hpp"
#include "Object3d.h"
#include "Texture.h"
#include "terrain.h"

class World3d {
private:
	int texids = 0;
	// int GetNextTextureId();
	// Texture* GetTexture(std::string filepath);
	WZmap* map;
	Shader* ObjectsShader = nullptr;
public:
	std::vector<Object3d*> Objects;
	std::vector<Texture*> Textures;
	Terrain Ter;
	SDL_Renderer *Renderer;
	World3d(WZmap *m, SDL_Renderer *r);
	~World3d();
	// void AddObject(std::string filename, unsigned int);
	void RenderScene(glm::mat4 view);
};

#endif /* end of include guard: WORLD3D_H_INCLUDED */
