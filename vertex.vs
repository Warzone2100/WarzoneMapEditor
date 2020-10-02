#version 330 core

attribute vec4 Coordinates;
attribute vec2 TexCoordinates;

uniform mat4 View;
uniform mat4 Transform;

varying vec2 texCoordinates;

void main()
{
    gl_Position = Transform * View * Coordinates;
    texCoordinates = TexCoordinates;
}
