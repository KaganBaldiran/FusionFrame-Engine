#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexColor;
layout (location = 2) in vec2 VertexTexCoord;

out vec2 TexCoord;
out vec3 VColor;

uniform mat4 proj;

void main()
{

   TexCoord = VertexTexCoord;
   VColor = VertexColor;

   gl_Position = proj * vec4(VertexPosition , 1.0f);
}
 
