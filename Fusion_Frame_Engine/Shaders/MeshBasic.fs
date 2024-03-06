#version 330 core

layout (location = 0) out vec4 OutColor;
layout (location = 1) out vec4 Depth;
layout (location = 2) out float ID;

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
uniform float metallic;
uniform float roughness;

#define MAX_LIGHT_COUNT 100

uniform vec3 LightPositions[MAX_LIGHT_COUNT];
uniform vec3 LightColors[MAX_LIGHT_COUNT];
uniform float LightIntensities[MAX_LIGHT_COUNT];
uniform int LightCount;

uniform float FogIntesityUniform;
uniform vec3 FogColor;
uniform vec3 EnvironmentRadiance;

uniform int disableclaymaterial[5];

uniform float ModelID;

vec3 PointLight(vec3 texturecolor,float roughnessmap,vec3 resultnormal,vec3 LightColor,vec3 LightPosition , float LightIntensity)
{
   float Ambient = 0.20f;

   vec3 specularColor = LightColor;

   vec3 N = resultnormal;
   vec3 L = LightPosition - CurrentPos;
   vec3 LDR = normalize(L);

   float LightDistance = length(L);
   float a = 0.2f;
   float b = 0.1f;
   float intensity = LightIntensity / (a * LightDistance * LightDistance + b * LightDistance + 1.0f);
   
   vec3 V = CameraPos - CurrentPos;
   vec3 H = normalize(L + V);

   float diffuse = max(dot(N,LDR),0.0f);
   vec3 specular = pow(max(dot(N,H),0.0f),32.0f) * specularColor * roughnessmap;

   return texturecolor * LightColor * ((diffuse  * intensity + Ambient) + roughnessmap * specular  * intensity);
};


void main()
{
    vec3 texturecolor;
     
    if(disableclaymaterial[0] == 1)
    {
      texturecolor = albedo.rgb;
    }
    else
    {
      texturecolor = texture(texture_diffuse0, FinalTexCoord).rgb;
    }

    float roughnessmap;

    if(disableclaymaterial[1] == 1)
    {
      roughnessmap = roughness;
    }
    else
    {
      roughnessmap = texture(texture_specular0, FinalTexCoord).r;
    }

    vec3 resultnormal;

    if(disableclaymaterial[2] == 1)
    {
        resultnormal = normalize(Normal);
    }
    else
    {
        resultnormal = texture(texture_normal0,FinalTexCoord).rgb;
        resultnormal = resultnormal * 2.0f - 1.0f;
        resultnormal = normalize(TBN * resultnormal);
    }


    float metalicmap;

    if(disableclaymaterial[3] == 1)
    {
      metalicmap = metallic;
    }
    else
    {
      metalicmap = texture(texture_metalic0, FinalTexCoord).r;
    }

   vec3 totalRadiance = vec3(0.0f);
   for(int lightID = 0;lightID < LightCount;lightID++)
   {
      totalRadiance += PointLight(texturecolor,roughnessmap,resultnormal,LightColors[lightID],LightPositions[lightID],LightIntensities[lightID]);
   }

   float DeltaPlane = FarPlane - NearPlane;
   float distanceFromCamera = distance(CameraPos,CurrentPos) / DeltaPlane;

   float FogIntensity = distanceFromCamera * distanceFromCamera * FogIntesityUniform;

   vec3 V = CameraPos - CurrentPos;
   vec3 N = resultnormal;

   vec3 F0 = vec3(0.04);
   vec3 fresnel = F0 + (max(vec3(1.0 - roughnessmap), F0) - F0) * pow(clamp(1.0 - max(dot(N,V),0.0f),0.0,1.0),5.0);

   ID = ModelID;
   Depth = vec4(CurrentPos,1.0f);
   float EnvironmentRadianceIntensity = 1.0f / normalize(DeltaPlane) * normalize(DeltaPlane);
   OutColor = vec4(totalRadiance * (EnvironmentRadiance * EnvironmentRadianceIntensity) + fresnel + (FogColor * FogIntensity),1.0f);
}