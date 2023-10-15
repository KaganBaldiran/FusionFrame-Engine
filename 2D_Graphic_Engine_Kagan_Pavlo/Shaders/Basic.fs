#version 330 core

in vec2 TexCoord;
in vec3 VColor; 

out vec4 FragColor;

void main()
{
	FragColor = vec4(VColor,1.0f);
}