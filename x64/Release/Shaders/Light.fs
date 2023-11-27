#version 330 core

layout (location = 0) out vec4 OutColor;
layout (location = 1) out vec4 Depth;

in vec3 Normal;
in vec2 FinalTexCoord;
in mat3 TBN;
in vec3 CurrentPos;

uniform vec3 CameraPos;
uniform float FarPlane;
uniform float NearPlane;

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_normal0;
uniform sampler2D texture_specular0;
uniform sampler2D texture_metalic0;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_metalic1;

uniform vec4 albedo;
uniform vec3 LightColor;

uniform float FogIntesityUniform;
uniform vec3 FogColor;

uniform int disableclaymaterial[4];

void main()
{
   OutColor = vec4(LightColor,1.0f);
   Depth = vec4(CurrentPos,1.0f);
}