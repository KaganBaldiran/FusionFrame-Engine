#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexColor;
layout (location = 2) in vec2 VertexTexCoord;

out vec2 TexCoord;
out vec3 VColor;

uniform mat4 proj;
uniform mat4 model;
uniform mat4 view;

uniform mat4 ratioMat;

void main()
{

   TexCoord = VertexTexCoord;
   VColor = VertexColor;

   gl_Position = ratioMat * proj * view * model * vec4(VertexPosition , 1.0f);
}
 
