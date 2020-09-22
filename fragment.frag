#version 330 core
in vec2 TexCoordinates;
uniform sampler2D Texture;

void main()
{
    gl_FragColor = texture(Texture, TexCoordinates);
}
