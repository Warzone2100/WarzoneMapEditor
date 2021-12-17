#version 330 core

varying vec2 VaryingTextureCoordinates;

uniform sampler2D Texture;

layout(location = 0) out vec4 Color;

void main()
{
	Color = texture(Texture, VaryingTextureCoordinates);
	//Color = vec4(1.0, 1.0, 1.0, 1.0);
}
