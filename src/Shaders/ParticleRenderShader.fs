#version 460 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 Depth;

in vec3 CurrentPos;
in vec4 ParticleColor;

uniform bool EnableTexture;

void main()
{
   FragColor = ParticleColor;
   Depth = vec4(CurrentPos,1.0f);
}