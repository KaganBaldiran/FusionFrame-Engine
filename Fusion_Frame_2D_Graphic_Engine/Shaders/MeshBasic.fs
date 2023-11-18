#version 330 core

out vec4 OutColor;

in vec3 Normal;
in vec2 FinalTexCoord;
in mat3 TBN;
in vec3 CurrentPos;

void main()
{
   OutColor = vec4(Normal,1.0f);
}