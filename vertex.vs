#version 330 core

attribute vec4 c;

uniform mat4 t;

void main()
{
    gl_Position = t * c;
}
