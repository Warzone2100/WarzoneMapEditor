#version 330 core

varying vec2 texCoordinates;

uniform sampler2D Texture;

void main()
{
    gl_FragColor = texture(Texture, texCoordinates);
}
