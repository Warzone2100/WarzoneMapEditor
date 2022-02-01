#version 330 core

in vec4 VertexCoordinates;
in vec2 TextureCoordinates;

uniform mat4 ViewProjection;
uniform mat4 Model;

out vec2 VaryingTextureCoordinates;

void main()
{
    gl_Position = ViewProjection * Model * VertexCoordinates;
    VaryingTextureCoordinates = TextureCoordinates;
}
