#version 460 core

in vec2 TexCoords;
out vec4 FragColor;

uniform vec4 ShapeColor;

void main()
{
  FragColor = ShapeColor;
}