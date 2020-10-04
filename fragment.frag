#version 330 core

varying vec2 VaryingTextureCoordinates;

uniform sampler2D Texture;

void main()
{
	gl_FragColor = texture(Texture, VaryingTextureCoordinates);
}
