#version 330 core

attribute vec4 Coordinates;
attribute vec2 TexCoordinates;

uniform mat4 Transform;

void main()
{
    gl_Position = Transform * Coordinates;
}
