#version 330 core

attribute vec4 VertexCoordinates;
attribute vec2 TextureCoordinates;

uniform mat4 Transform;
uniform mat4 Model;

varying vec2 VaryingTextureCoordinates;

void main()
{
    gl_Position = Transform * Model * VertexCoordinates;
    VaryingTextureCoordinates = TextureCoordinates;
}
