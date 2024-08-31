#version 460 core

in vec2 TextCoords;
out vec4 FragColor;

uniform sampler2D AlbedoTexture;

void main()
{
  FragColor = texture(AlbedoTexture,TextCoords);
}