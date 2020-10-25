#version 330 core

varying vec3 VaryingTextureCoordinates;

uniform sampler2D Texture;

layout(location = 0) out vec4 Color;

void main()
{
	//Color = texture(Texture, VaryingTextureCoordinates);
	Color.rgb = texture(Texture, VaryingTextureCoordinates.xy).rgb;
	Color.a = VaryingTextureCoordinates.z;
	//Color = vec4(1.0, 1.0, 1.0, 1.0);
}
