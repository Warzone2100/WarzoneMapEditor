#version 330 core

varying vec2 VaryingTextureCoordinates;
uniform sampler2D Texture;

uniform vec3 ColorMix;
uniform int DoColorMix;

layout(location = 0) out vec4 ColorOut;

void main()
{
	// ColorOut = vec4(1.0);
	if(DoColorMix == true) {
		vec4 TextureColor = texture(Texture, VaryingTextureCoordinates)*vec4(0.3);
		ColorOut.rgb = ColorMix.xyz*vec3(0.7) + TextureColor.rgb;
		ColorOut.a = texture(Texture, VaryingTextureCoordinates).a;
	} else {
		ColorOut = texture(Texture, VaryingTextureCoordinates);
	}
}
