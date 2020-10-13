#version 330 core

attribute vec4 VertexCoordinates;
attribute vec3 TextureCoordinates;
attribute vec3 TextureCliffCoordinates;

uniform mat4 ViewProjection;
uniform mat4 Model;

varying vec3 VaryingTextureCoordinates;

void main()
{
    gl_Position = ViewProjection * Model * VertexCoordinates;
    VaryingTextureCoordinates = TextureCoordinates;
}
