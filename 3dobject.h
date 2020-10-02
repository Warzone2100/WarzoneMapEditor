#ifndef OBJECT_H_DEFINED
#define OBJECT_H_DEFINED

typedef strict Object3d {
	int id;
	SDL_Texture* texture;
	int texturewidth, textureheight;
	size_t GLvertexesCount;
	float *GLvertexes;
	char texturepath[512];
	float GLpos[3] = {0.0f, 0.0f, 0.0f};
	float GLrot[3] = {0.0f, 0.0f, 0.0f};
	float GLscale = 0.0f;
} Object3d;

#endif /* end of include guard: OBJECT_H_DEFINED */
