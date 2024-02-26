#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 NormalMetalicPass;
layout (location = 2) out vec4 PositionDepthPass;
layout (location = 3) out vec4 MetalicRoughnessPass;

in vec3 TexCoords;
uniform samplerCube skybox;

void main()
{
   FragColor = texture(skybox,TexCoords);
   //FragColor = textureLod(skybox, TexCoords, 0); 
}