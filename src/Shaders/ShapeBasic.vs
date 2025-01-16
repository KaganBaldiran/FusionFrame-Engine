#version 460 core

layout (location = 0) in vec2 VertexPosition;
layout (location = 1) in vec2 VertexTexCoord;

out vec2 TexCoords;

uniform mat4 ModelMatrix;
uniform mat4 ProjMat;

uniform vec2 ShapePosition;
uniform vec2 ShapeScale;

void main()
{
  vec4 CurrentPos = ModelMatrix * vec4(VertexPosition.xy,0.0f,1.0f);
  CurrentPos.xy = CurrentPos.xy * ShapeScale + ShapePosition;

  gl_Position = ProjMat * vec4(CurrentPos.xy,0.0f,1.0f);

  TexCoords = VertexTexCoord;
}