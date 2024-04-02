#version 420 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 Depth;
  
in vec2 TexCoords;
uniform sampler2D AlbedoSpecularPass;
uniform sampler2D NormalMetalicPass;
uniform sampler2D PositionDepthPass;
uniform sampler2D MetalicRoughnessPass;

uniform float FarPlane;
uniform float NearPlane;
uniform vec3 CameraPos;

uniform float DOFdistance;
uniform bool DOFenabled;
uniform float DOFintensity;

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

uniform float ModelID;
uniform float ObjectScale;

uniform float ShadowMapFarPlane[MAX_LIGHT_COUNT / 10];
uniform samplerCube OmniShadowMaps[MAX_LIGHT_COUNT / 10];
uniform int OmniShadowMapsLightIDS[MAX_LIGHT_COUNT];

#define MAX_CASCADE_PLANE_COUNT 16
uniform int CascadeCount;

uniform sampler2DArray SunShadowMap;
uniform float CascadeShadowPlaneDistances[MAX_CASCADE_PLANE_COUNT];
uniform mat4 ViewMatrix;
uniform int CascadeShadowMapLightID;

layout (std140) uniform LightSpaceMatrices
{
    mat4 lightSpaceMatrices[16];
};

uniform int OmniShadowMapCount;
const float PI = 3.14159265359;
   
 float CascadedDirectionalShadowCalculation(vec3 fragPos , sampler2DArray ShadowMapArray , vec3 N, vec3 LightDirection)
 {
    vec4 FragPosView = ViewMatrix * vec4(fragPos,1.0f);
    float Depth = abs(FragPosView.z);

    int Layer = -1;
     for (int i = 0; i < CascadeCount; ++i)
     {
        if(Depth < CascadeShadowPlaneDistances[i])
        {
          Layer = i;
          break;
        }
     }
     if(Layer == -1)
     {
      Layer = CascadeCount;
     }

     vec4 FragPosLightSpace = lightSpaceMatrices[Layer] * vec4(fragPos,1.0f);
     vec3 ProjectedCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
     ProjectedCoords = ProjectedCoords * 0.5f + 0.5f;

     float CurrentDepth = ProjectedCoords.z;
     if(CurrentDepth > 1.0f)
     {
        return 0.0f;
     }

     vec3 normal = N;
     float bias = max(0.05 * (1.0f - dot(normal , LightDirection)) , 0.005);
     const float BiasMultiplier = 0.5f;

     if(Layer == CascadeCount)
     {
        bias *= 1 / (FarPlane * BiasMultiplier);
     }
     else
     {
        bias *= 1 / (CascadeShadowPlaneDistances[Layer] * BiasMultiplier);
     }

     float shadow = 0.0f;
     vec2 TexelSize = 1.0f / vec2(textureSize(ShadowMapArray,0));
     for(int x = -1; x <= 1; ++x)
     {
        for(int y = -1; y <= 1; ++y)
        {
           float FilteredDepth = texture(ShadowMapArray,vec3(ProjectedCoords.xy + vec2(x,y) * TexelSize , Layer)).r;
           shadow += (CurrentDepth - bias) > FilteredDepth ? 1.0f : 0.0f;
        }
     }
     shadow /= 9.0f;
     return shadow;
 }

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
   vec4 AlbedoSpecular = texture(AlbedoSpecularPass, TexCoords);
   vec4 NormalMetalic = texture(NormalMetalicPass, TexCoords);
   vec4 PositionDepth = texture(PositionDepthPass, TexCoords);
   vec4 MetalicRoughness = texture(MetalicRoughnessPass, TexCoords);

   vec3 Albedo = AlbedoSpecular.rgb;
   vec3 Normal = NormalMetalic.rgb;
   float roughness = MetalicRoughness.r;
   float Metalic = MetalicRoughness.g;
   vec3 Position = PositionDepth.rgb;

      float shadow = 0.0f;
      vec3 N = normalize(Normal);
      vec3 V = normalize(CameraPos - Position);

      vec3 F0 = vec3(0.04);
      F0 = mix(F0,Albedo,Metalic);

      vec3 Lo = vec3(0.0);
      for(int i = 0; i < LightCount;++i)
      {
          vec3 L;
          vec3 H;
          vec3 radiance;

          if(LightTypes[i] == POINT_LIGHT)
          {
            L = normalize(LightPositions[i] - Position);
            H = normalize(V + L);
            float distance = length(LightPositions[i] - Position);
            float attenuation = 1.0 / (distance * distance);
            radiance = LightColors[i].xyz * attenuation;
          }
          else if(LightTypes[i] == DIRECTIONAL_LIGHT)
          {
            L = normalize(LightPositions[i]);
            H = normalize(V + L); 
            radiance = LightColors[i].xyz;
          }
          float NDF = DistributionGGX(N,H,roughness);
          float G = GeometrySmith(N,V,L,roughness);
          vec3 F = FresnelSchlick(max(dot(H,V),0.0),F0);
          //vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

          vec3 kS = F;
          vec3 Kd = vec3(1.0) - kS;
          Kd *= 1.0 - Metalic;

          vec3 numerator = NDF * G * F;
          float denominator = 4.0 * max(dot(N,V),0.0) * max(dot(N,L),0.0) + 0.0001;
          vec3 specular = numerator / denominator;

          float NdotL = max(dot(N,L),0.0);

          if(i < OmniShadowMapCount)
          {
             shadow = ShadowCalculationOmni(Position,OmniShadowMaps[i],LightPositions[i] , ShadowMapFarPlane[i]);
          }
          if(i == LightCount - 1)
          {
             shadow = CascadedDirectionalShadowCalculation(Position,SunShadowMap,N , LightPositions[i]);
          }
    
          /*
          else
          {
             Lo += (Kd * Albedo / PI + specular) * radiance * LightIntensities[i] * NdotL;
          }
          */

          Lo += (1.0 - shadow) * (Kd * Albedo / PI + specular) * radiance * LightIntensities[i] * NdotL;

          //Lo += (Kd * texturecolor / PI + specular) * radiance * LightIntensities[i] * NdotL;
      }

      vec3 color;
      vec3 ambient;

      if(EnableIBL)
      {
         vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

         vec3 R = reflect(-V , N);
         const float MAX_REFLECTION_LOD = 4.0f;
         vec3 prefilteredColor = textureLod(prefilteredMap,R,roughness * MAX_REFLECTION_LOD).rgb;
         vec2 EnvLut = texture(LUT,vec2(max(dot(N,V),0.0),roughness)).rg;
         vec3 specular = prefilteredColor * (F * EnvLut.x + EnvLut.y);

         vec3 irradiance = texture(ConvDiffCubeMap, N).rgb;
         vec3 kS = F; 
         vec3 kD = 1.0 - kS;
         vec3 diffuse = irradiance * Albedo;
         ambient = (kD * diffuse + specular) * ao; 
         color = ambient + Lo;
      }
      else
      {
         ambient = vec3(0.03) * Albedo * ao;
         color = ambient + Lo;
      }
      
      color = color / (color + vec3(1.0));
      color = pow(color, vec3(1.0/2.2));  

      bool FogEnabled = false;

      float DeltaPlane = FarPlane - NearPlane;
      float distanceFromCamera = distance(CameraPos,Position) / DeltaPlane;

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

      //float NormalizedDeltaPlane = normalize(DeltaPlane);
      //float EnvironmentRadianceIntensity = 1.0f / NormalizedDeltaPlane * NormalizedDeltaPlane;
      FragColor = vec4(color + (FinalFogColor * FogIntensity), 1.0); 
      Depth = vec4(Position,1.0f);
      //FragColor = vec4(vec3(shadow),1.0f); 

      //FragColor = vec4(vec3(roughness),1.0f);
   //vec4 OutColor = vec4(vec3(Metalic),1.0f);
   //FragColor = vec4(pow(OutColor.xyz.xyz,vec3(0.9)),OutColor.w); 
}