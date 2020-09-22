#version 330 core

attribute vec4 Coordinates;

uniform mat4 Transform;

void main()
{
    gl_Position = Transform * Coordinates;
}
