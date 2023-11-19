#version 330 core

out vec4 OutColor;

in vec3 Normal;
in vec2 FinalTexCoord;
in mat3 TBN;
in vec3 CurrentPos;

uniform vec3 CameraPos;

void main()
{
   vec3 LightPosition = vec3(0.0f, 4.0f,0.0f);
   vec3 LightColor = vec3(1.0f,1.0f,1.0f);
   vec3 Ambient = vec3(0.2f,0.2f,0.2f);

   vec3 specularColor = vec3(1.0f,1.0f,1.0f);

   vec3 L = LightPosition - CurrentPos;
   vec3 LDR = normalize(L);

   float LightDistance = length(L);
   float intensity = 1.0f / LightDistance * LightDistance;
   
   vec3 V = CameraPos - CurrentPos;
   vec3 H = normalize(L + V);

   vec3 diffuse = max(dot(Normal,LDR),0.0f) * LightColor;
   vec3 specular = pow(max(dot(Normal,H),0.0f),32.0f) * specularColor;

   OutColor = vec4((diffuse * intensity) + Ambient + (specular * intensity),1.0f);
}