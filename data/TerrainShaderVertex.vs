#version 330 core

attribute vec4 VertexCoordinates;
attribute vec3 TextureCoordinates;
attribute vec3 TextureGroundCoordinates;

uniform mat4 ViewProjection;
uniform mat4 Model;
uniform int Pass;

varying vec3 VaryingTextureCoordinates;

void main()
{
    gl_Position = ViewProjection * Model * VertexCoordinates;
	if(Pass == 0) {
		VaryingTextureCoordinates = TextureCoordinates;
	} else {
		VaryingTextureCoordinates = TextureCoordinates;

	}
}
