#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 t;

void main()
{
    gl_Position = t * vec4(aPos, 1.0f);
	// gl_Position = mat4(1) * vec4(aPos, 1.0f);
}
