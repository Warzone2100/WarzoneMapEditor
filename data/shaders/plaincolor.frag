#version 330 core

uniform vec4 ColorIn;

layout(location = 0) out vec4 ColorOut;

void main()
{
	ColorOut = ColorIn;
	// ColorOut = vec4(1.0, 1.0, 1.0, 1.0);
}
