#version 330 core

attribute vec4 VertexCoordinates;

uniform mat4 ViewProjection;

void main()
{
    gl_Position = ViewProjection * VertexCoordinates;
}
