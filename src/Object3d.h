#ifndef OBJECT3D_H_DEFINED
#define OBJECT3D_H_DEFINED

#include <string>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
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
