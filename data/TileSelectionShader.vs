#version 330 core

attribute vec3 VertexCoordinates;

void main()
{
    gl_Position = vec4(VertexCoordinates, 1);
}
