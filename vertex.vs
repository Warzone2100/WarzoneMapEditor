#version 330 core

attribute vec4 VertexCoordinates;
attribute vec2 TextureCoordinates;

uniform mat4 Transform;

varying vec2 VaryingTextureCoordinates;

void main()
{
    gl_Position = Transform * VertexCoordinates;
    VaryingTextureCoordinates = TextureCoordinates;
}
