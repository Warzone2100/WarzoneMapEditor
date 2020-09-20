#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture1;

void main()
{
	FragColor = vec4(255, 255, 0, 255); //texture(texture1, TexCoord);
}
