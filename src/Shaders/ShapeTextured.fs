#version 460 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D AlbedoTexture;

void main()
{
  FragColor = textureLod(AlbedoTexture,TexCoords,0.0f);
}