#version 330 core

varying vec2 VaryingTextureCoordinates;

uniform sampler2D Texture;

void main()
{
	//gl_FragColor = vec4(1); // texture(Texture, VaryingTextureCoordinates);
	gl_FragColor = texture(Texture, VaryingTextureCoordinates);
}
