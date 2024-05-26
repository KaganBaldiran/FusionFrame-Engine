#version 460 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 VertexColor;
layout (location = 2) in vec2 TextureCoord;

out mat4 ModelMat;
out mat4 ViewMat;
out vec4 PositionClipSpace;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main()
{
   ModelMat = ModelMatrix;
   ViewMat = ViewMatrix;

   vec3 CurrentPos = vec3(ModelMatrix * vec4(Position,1.0f));
   PositionClipSpace = ProjectionMatrix * ViewMatrix * vec4(CurrentPos,1.0f);  
   gl_Position = PositionClipSpace;
}