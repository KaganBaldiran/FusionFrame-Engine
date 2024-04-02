#version 460 core

out vec4 FragColor;

in vec3 CurrentPos;
in vec4 ParticleColor;

void main()
{
   FragColor = ParticleColor;
}