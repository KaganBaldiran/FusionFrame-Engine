#version 330 core

out vec4 OutColor;

in vec3 Normal;
in vec2 FinalTexCoord;
in mat3 TBN;
in vec3 CurrentPos;

void main()
{
   OutColor = vec4(1.0f,1.0f,0.0f,1.0f);
}