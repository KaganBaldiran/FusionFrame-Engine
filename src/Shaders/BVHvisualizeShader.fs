#version 460 core

layout (location = 0) out vec4 OutColor;
layout (location = 1) out vec4 Depth;
layout (location = 2) out vec4 SSLS;

in vec3 CurrentPos;
in vec3 UniqueColor;

void main()
{
   OutColor = vec4(UniqueColor,1.0f);
   SSLS = vec4(UniqueColor,1.0f);
   Depth = vec4(CurrentPos,1.0f);
}