#version 330 core

attribute vec4 VertexCoordinates;
attribute vec2 TextureCoordinates;

uniform mat4 View;
uniform mat4 Transform;
uniform mat4 Projection;

varying vec2 VaryingTextureCoordinates;

void main()
{
    gl_Position = Transform * VertexCoordinates;
    VaryingTextureCoordinates = TextureCoordinates;
}
