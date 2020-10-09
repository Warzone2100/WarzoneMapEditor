#version 330 core

varying vec2 VaryingTextureCoordinates;

uniform sampler2D Texture;

void main()
{
	//gl_FragColor = vec4(1); // texture(Texture, VaryingTextureCoordinates);
	vec4 texColor = texture(Texture, VaryingTextureCoordinates);
	if(texColor.a < 0.1)
        discard;
	gl_FragColor = texColor;
}
