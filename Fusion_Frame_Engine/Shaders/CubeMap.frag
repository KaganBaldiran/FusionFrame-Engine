#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 Depth;


in vec3 TexCoords;
uniform samplerCube skybox;

void main()
{
   FragColor = texture(skybox,TexCoords);
   Depth = vec4(0.0f);
   //FragColor = textureLod(skybox, TexCoords, 0); 
}