#version 420 core

layout (location = 0) out vec4 OutColor;
layout (location = 1) out vec4 Depth;
layout (location = 3) out float ID;

in vec3 Normal;
in vec2 FinalTexCoord;
in mat3 TBN;
in vec3 CurrentPos;

uniform float TilingCoeff;

uniform vec3 CameraPos;
uniform float FarPlane;
uniform float NearPlane;

uniform sampler2D texture_diffuse0;
uniform sampler2D texture_normal0;
uniform sampler2D texture_specular0;
uniform sampler2D texture_metalic0;
uniform sampler2D texture_alpha0;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_metalic1;
uniform sampler2D texture_alpha1;

uniform vec4 albedo;
uniform float metallic;
uniform float roughness;

#define MAX_LIGHT_COUNT 100
#define POINT_LIGHT 0x56400
#define DIRECTIONAL_LIGHT 0x56401
#define SPOT_LIGHT 0x56402

uniform vec3 LightPositions[MAX_LIGHT_COUNT];
uniform vec3 LightColors[MAX_LIGHT_COUNT];
uniform float LightIntensities[MAX_LIGHT_COUNT];
uniform int LightTypes[MAX_LIGHT_COUNT];
uniform int LightCount;

uniform float FogIntesityUniform;
uniform vec3 FogColor;
uniform vec3 EnvironmentRadiance;
uniform bool IBLfog;

uniform samplerCube ConvDiffCubeMap;
uniform samplerCube prefilteredMap;
uniform sampler2D LUT;
uniform bool EnableIBL;
uniform float ao;

uniform int disableclaymaterial[5];

uniform float ModelID;
uniform float ObjectScale;

uniform float ShadowMapFarPlane[MAX_LIGHT_COUNT];
uniform samplerCube OmniShadowMaps[MAX_LIGHT_COUNT];

uniform int OmniShadowMapCount;
const float PI = 3.14159265359;


 float ShadowCalculationOmni(vec3 fragPos , samplerCube OmnishadowMap , vec3 LightPosition , float farplane)
  {
      vec3 gridSamplingDisk[20] = vec3[]
      (
        vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
        vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
        vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
        vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
        vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
      );

      vec3 fragTolight = fragPos - LightPosition;
      float currentDepth = length(fragTolight);

      float shadow = 0.0;
      //float bias   = 0.15;
      float bias   = 0.5;
      //int samples  = int(max(min((20.0f * (ObjectScale / FarPlane)),64.0f) , 1.0f) );
      //int samples  = 20;
      int samples  = 40;
      float viewDistance = length(CameraPos - fragPos);
      float diskRadius = (1.0 + (viewDistance / farplane)) / 25.0;  
      for(int i = 0; i < samples; ++i)
      {
        float closestDepth = texture(OmnishadowMap, fragTolight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= farplane;   
        if(currentDepth - bias > closestDepth)
        {
            shadow += 1.0;
        }
      }
      shadow /= float(samples);   
      return shadow;
  }

  /*
  float ShadowCalculationOmni(vec3 fragPos , samplerCube OmnishadowMap , vec3 LightPosition)
  {
      vec3 fragTolight = fragPos - LightPosition;
      float closestDepth = texture(OmnishadowMap,fragTolight).r;
      closestDepth *= 25.0f;

      float currentDepth = length(fragTolight);

      float bias = 0.05;
      float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
      
      return shadow;
  }
  */
float DistributionGGX(vec3 N , vec3 H, float roughness)
  {
      float a = roughness * roughness;
      float a2 = a*a;
      float NdotH = max(dot(N,H),0.0);
      float NdotH2 = NdotH * NdotH;

      float num = a2;
      float denom = (NdotH2 * (a2 - 1.0) + 1.0);
      denom = PI * denom * denom;

      return num / denom;
  }

  float GeometrySchlickGGX(float NdotV , float roughness)
  {
      float r = (roughness + 1.0);
      float k = (r*r) / 8.0;

      float num = NdotV;
      float denom = NdotV * (1.0 - k) + k;

      return num / denom;
  }

  float GeometrySmith(vec3 N , vec3 V , vec3 L , float roughness)
  {
      float NdotV = max(dot(N,V),0.0);
      float NdotL = max(dot(N,L),0.0);
      float ggx2 = GeometrySchlickGGX(NdotV,roughness);
      float ggx1 = GeometrySchlickGGX(NdotL,roughness);

      return ggx1 * ggx2;
  }

  vec3 FresnelSchlick(float cosTheta , vec3 F0)
  {
      return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta,0.0,1.0),5.0);
  }

  vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
  {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
  } 

void main()
{
    vec3 texturecolor;

    float AlphaMap;
    if(disableclaymaterial[4] == 1)
    {
      AlphaMap = 1.0f;
    }
    else
    {
      AlphaMap = texture(texture_alpha0, FinalTexCoord * TilingCoeff).r;
    }

    if(AlphaMap < 0.5f)
    {
      discard;
    }
     
    if(disableclaymaterial[0] == 1)
    {
      texturecolor = albedo.rgb;
    }
    else
    {
      texturecolor = texture(texture_diffuse0, FinalTexCoord * TilingCoeff).rgb;
    }

    float roughnessmap;

    if(disableclaymaterial[1] == 1)
    {
      roughnessmap = roughness;
    }
    else
    {
      roughnessmap = texture(texture_specular0, FinalTexCoord * TilingCoeff).r;
    }

    vec3 resultnormal;

    if(disableclaymaterial[2] == 1)
    {
        resultnormal = normalize(Normal);
    }
    else
    {
        resultnormal = texture(texture_normal0,FinalTexCoord * TilingCoeff).rgb;
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
      metalicmap = texture(texture_metalic0, FinalTexCoord * TilingCoeff).r;
    }

      float shadow;
      vec3 N = normalize(resultnormal);
      vec3 V = normalize(CameraPos - CurrentPos);

      vec3 F0 = vec3(0.04);
      F0 = mix(F0,texturecolor,metalicmap);

      vec3 Lo = vec3(0.0);
      for(int i = 0; i < LightCount;++i)
      {
          vec3 L;
          vec3 H;
          vec3 radiance;

          if(LightTypes[i] == POINT_LIGHT)
          {
            L = normalize(LightPositions[i] - CurrentPos);
            H = normalize(V + L);
            float distance = length(LightPositions[i] - CurrentPos);
            float attenuation = 1.0 / (distance * distance);
            radiance = LightColors[i].xyz * attenuation;
          }
          else if(LightTypes[i] == DIRECTIONAL_LIGHT)
          {
            L = normalize(LightPositions[i]);
            H = normalize(V + L); 
            radiance = LightColors[i].xyz;
          }

          float NDF = DistributionGGX(N,H,roughnessmap);
          float G = GeometrySmith(N,V,L,roughnessmap);
          vec3 F = FresnelSchlick(max(dot(H,V),0.0),F0);

          vec3 kS = F;
          vec3 Kd = vec3(1.0) - kS;
          Kd *= 1.0 - metalicmap;

          vec3 numerator = NDF * G * F;
          float denominator = 4.0 * max(dot(N,V),0.0) * max(dot(N,L),0.0) + 0.0001;
          vec3 specular = numerator / denominator;

          float NdotL = max(dot(N,L),0.0);

          if(i < OmniShadowMapCount)
          {
             shadow = ShadowCalculationOmni(CurrentPos,OmniShadowMaps[i],LightPositions[i] , ShadowMapFarPlane[i]);
             Lo += (1.0 - shadow) * (Kd * texturecolor / PI + specular) * radiance * LightIntensities[i] * NdotL;
          }
          else
          {
             Lo += (Kd * texturecolor / PI + specular) * radiance * LightIntensities[i] * NdotL;
          }

          //Lo += (Kd * texturecolor / PI + specular) * radiance * LightIntensities[i] * NdotL;
      }

      vec3 color;
      vec3 ambient;

      if(EnableIBL)
      {
         vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughnessmap);

         vec3 R = reflect(-V , N);
         const float MAX_REFLECTION_LOD = 4.0f;
         vec3 prefilteredColor = textureLod(prefilteredMap,R,roughnessmap * MAX_REFLECTION_LOD).rgb;
         vec2 EnvLut = texture(LUT,vec2(max(dot(N,V),0.0),roughnessmap)).rg;
         vec3 specular = prefilteredColor * (F * EnvLut.x + EnvLut.y);

         vec3 irradiance = texture(ConvDiffCubeMap, N).rgb;
         vec3 kS = F; 
         vec3 kD = 1.0 - kS;
         vec3 diffuse = irradiance * texturecolor;
         ambient = (kD * diffuse + specular) * ao; 
         color = ambient + Lo;
      }
      else
      {
         ambient = vec3(0.03) * texturecolor * ao;
         color = ambient + Lo;
      }
      
      color = color / (color + vec3(1.0));
      color = pow(color, vec3(1.0/2.2));  

      bool FogEnabled = false;

      float DeltaPlane = FarPlane - NearPlane;
      float distanceFromCamera = distance(CameraPos,CurrentPos) / DeltaPlane;

      float FogIntensity = distanceFromCamera * distanceFromCamera * FogIntesityUniform;

      vec3 FinalFogColor;
      if(IBLfog)
      {
         FinalFogColor = texture(ConvDiffCubeMap, -N).rgb;
      }
      else
      {
         FinalFogColor = FogColor;
      }

      ID = ModelID;
      Depth = vec4(CurrentPos,1.0f);
      float EnvironmentRadianceIntensity = 1.0f / normalize(DeltaPlane) * normalize(DeltaPlane);
      OutColor = vec4(color + (FinalFogColor * FogIntensity), 1.0);   
}