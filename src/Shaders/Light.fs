#version 460 core

layout (location = 0) out vec4 OutColor;
layout (location = 1) out vec4 Depth;
layout (location = 2) out vec4 ID;

in vec3 CurrentPos;
uniform vec3 LightColor;
uniform float ModelID;

void main()
{
   OutColor = vec4(LightColor,1.0f);
   ID = vec4(vec3(ModelID),1.0f);
   Depth = vec4(CurrentPos,1.0f);
}