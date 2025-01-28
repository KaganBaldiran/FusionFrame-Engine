#version 460 core
layout(early_fragment_tests) in;
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 Depth;
layout (location = 2) out vec4 ID;

in vec2 TexCoords;
uniform sampler2D AlbedoSpecularPass;
uniform sampler2D NormalPass;
uniform sampler2D PositionDepthPass;
uniform sampler2D MetalicRoughnessModelIDPass;
uniform sampler2D DecalNormalPass;

uniform float FarPlane;
uniform float NearPlane;
uniform vec3 CameraPos;

uniform float DOFdistance;
uniform bool DOFenabled;
uniform float DOFintensity;

#define MAX_LIGHT_COUNT 100
#define MAX_SHADOWMAP_COUNT 5
#define POINT_LIGHT 0x56400
#define DIRECTIONAL_LIGHT 0x56401
#define SPOT_LIGHT 0x56402

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

//uniform float ModelID;

uniform float ShadowMapFarPlane[MAX_SHADOWMAP_COUNT];
uniform samplerCube OmniShadowMaps[MAX_SHADOWMAP_COUNT];

uniform sampler2DArray CascadeShadowMaps;
uniform mat4 ViewMatrix;

#define MAX_CASCADE_PLANE_COUNT 16
#define MAX_CASCADED_SHADOW_MAP_COUNT 12

uniform float CascadedShadowMapSoftness;
uniform float ShadowBiasMultiplier;

layout(std430, binding = 10) readonly buffer CascadedMapMetaDatas
{
	mat4 LightMatrices[MAX_CASCADE_PLANE_COUNT * MAX_CASCADED_SHADOW_MAP_COUNT];
	vec4 PositionAndSize[MAX_CASCADE_PLANE_COUNT * MAX_CASCADED_SHADOW_MAP_COUNT];
	vec4 LightDirection[MAX_CASCADED_SHADOW_MAP_COUNT];
	float ShadowCascadeLevels[MAX_CASCADE_PLANE_COUNT * MAX_CASCADED_SHADOW_MAP_COUNT];
	float Layer[MAX_CASCADE_PLANE_COUNT * MAX_CASCADED_SHADOW_MAP_COUNT];
	float CascadeCount[MAX_CASCADED_SHADOW_MAP_COUNT];
};

uniform vec2 screenSize;

struct Light
{
  vec4 Position;
  vec4 Color;
  int Type;
  float Intensity;
  float Radius;
  int ShadowMapIndex;
};

#define MAX_LIGHT_PER_CLUSTER 100

layout(std430 , binding = 4) restrict buffer LightsDatas
{
    Light Lights[];
};

uniform int OmniShadowMapCount;
const float PI = 3.14159265359;
   
float CascadedDirectionalShadowCalculation(vec3 fragPos,int MetaDataIndex,vec3 N, vec3 LightDirection_i,inout vec3 ColorMap)
{
     int IndexOffset = MAX_CASCADE_PLANE_COUNT * MetaDataIndex;
     vec4 FragPosView = ViewMatrix * vec4(fragPos,1.0f);
     float Depth = abs(FragPosView.z);

     int CascadeCount = int(CascadeCount[MetaDataIndex]);

     int Layeri = -1;
     for (int i = 0; i < CascadeCount; ++i)
     {
        if(Depth < ShadowCascadeLevels[IndexOffset + i])
        {
          if(i == 0)
          {
              ColorMap = vec3(1.0f,0.0f,0.0f);
          }
          else if(i == 1)
          {
              ColorMap = vec3(0.0f,1.0f,0.0f);
          }
          else if(i == 2)
          {
              ColorMap = vec3(0.0f,0.0f,1.0f);
          }
          else if(i == 3)
          {
              ColorMap = vec3(1.0f,0.0f,1.0f);
          }
          Layeri = i;
          //ColorMap = vec3(ShadowCascadeLevels[IndexOffset + i] / (FarPlane - NearPlane));
          break;
        }
     }
     if(Layeri == -1)
     {
       Layeri = CascadeCount;
       ColorMap = vec3(1.0f,1.0f,0.0f);
     }

    
     int OffSetLayerIndex = IndexOffset + Layeri;

     vec4 FragPosLightSpace = LightMatrices[OffSetLayerIndex] * vec4(fragPos,1.0f);
     vec3 ProjectedCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
     ProjectedCoords = ProjectedCoords * 0.5f + 0.5f;

     vec4 PositionAndSize = PositionAndSize[OffSetLayerIndex];
     ProjectedCoords.xy = (ProjectedCoords.xy * PositionAndSize.z) + PositionAndSize.xy;

     float CurrentDepth = ProjectedCoords.z;
     if(CurrentDepth > 1.0f)
     {
        return 0.0f;
     }

     vec3 normal = N;
     float bias = max(ShadowBiasMultiplier * (1.0f - dot(normal , LightDirection_i)) , 0.005);
     //float bias = 0.005;
     //const float BiasMultiplier = 0.5f;
     const float BiasMultiplier = PositionAndSize.z;
     if(Layeri == CascadeCount)
     {
        bias *= 1 / (FarPlane * BiasMultiplier);
     }
     else
     {
        bias *= 1 / (ShadowCascadeLevels[OffSetLayerIndex] * BiasMultiplier);
     }

     Layeri = int(Layer[OffSetLayerIndex]);

     float shadow = 0.0f;
     vec2 TexelSize = CascadedShadowMapSoftness / vec2(textureSize(CascadeShadowMaps,0));
     //vec2 LayerTextureSize = vec2(textureSize(CascadeShadowMaps,0));
     //vec2 TexelSize = 1.0f / (LayerTextureSize * PositionAndSize.zw);
     for(int x = -1; x <= 1; ++x)
     {
        for(int y = -1; y <= 1; ++y)
        {
           float FilteredDepth = texture(CascadeShadowMaps,vec3(ProjectedCoords.xy + vec2(x,y) * TexelSize , Layeri)).r;
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

  float DistributionGGX(vec3 N , vec3 H, float roughness)
  {
    float a = clamp(roughness * roughness, 0.03f, 1.0f);         
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0f);       
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = max(PI * denom * denom, 0.0001f); 

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
   vec4 MetalicRoughness = texture(MetalicRoughnessModelIDPass, TexCoords);
   float SceneAlpha = MetalicRoughness.w;
   float ModelID = MetalicRoughness.z;

   if(SceneAlpha < 1.0f)
   {
      discard;
   }

   vec4 AlbedoSpecular = texture(AlbedoSpecularPass, TexCoords);
   vec3 Normal = texture(NormalPass, TexCoords).rgb;
   vec3 DecalNormal = texture(DecalNormalPass,TexCoords).rgb;
   vec4 PositionDepth = texture(PositionDepthPass, TexCoords);

   vec3 Albedo = AlbedoSpecular.rgb;
   float roughness = MetalicRoughness.r;
   float Metalic = MetalicRoughness.g;
   vec3 Position = PositionDepth.rgb;

   vec3 ColorMap;

   /*
   vec4 FragPosView = ViewMatrix * vec4(Position,1.0f);
   uint zTile = uint((log(abs(FragPosView.z) / NearPlane) * gridSize.z) / log(FarPlane / NearPlane));
   vec2 tileSize = screenSize / gridSize.xy;
   uvec3 tile = uvec3(gl_FragCoord.xy / tileSize, zTile);
   uint tileIndex = uint(tile.x + (tile.y * gridSize.x) + (tile.z * gridSize.x * gridSize.y));

   Cluster cluster = Clusters[tileIndex];
   */

    float shadow = 0.0f;
    vec3 N = normalize(Normal + DecalNormal);
    vec3 V = normalize(CameraPos - Position);

    float DotNV = max(dot(N,V),0.0);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0,Albedo,Metalic);

    vec3 Lo = vec3(0.0);
    for(int i = 0; i < LightCount;++i)
    {
        Light CurrentLight = Lights[i];
        //Light CurrentLight = Lights[cluster.LightIndices[i]];
        vec3 L;
        vec3 H;
        vec3 radiance;

        if(CurrentLight.Type == POINT_LIGHT)
        {
		vec3 CurrentLightPosition = CurrentLight.Position.xyz;
        float distance = length(CurrentLightPosition - Position);
            
        if(distance > CurrentLight.Radius)
        {
            continue;
        }

        L = normalize(CurrentLightPosition - Position);
        H = normalize(V + L);
        float attenuation = 1.0 / (distance * distance);
        radiance = CurrentLight.Color.xyz * attenuation;

        int OmniShadowMapIndex = CurrentLight.ShadowMapIndex;
        if(OmniShadowMapIndex >= 0)
        {
            shadow = ShadowCalculationOmni(Position,OmniShadowMaps[OmniShadowMapIndex],CurrentLight.Position.xyz , ShadowMapFarPlane[OmniShadowMapIndex]);
        }
        }
        else if(CurrentLight.Type == DIRECTIONAL_LIGHT)
        {
        L = normalize(CurrentLight.Position.xyz);
        H = normalize(V + L); 
        radiance = CurrentLight.Color.xyz;

        int DirectionalShadowMapIndex = CurrentLight.ShadowMapIndex;
        if(DirectionalShadowMapIndex >= 0)
        {
            shadow = CascadedDirectionalShadowCalculation(Position,DirectionalShadowMapIndex,N , CurrentLight.Position.xyz,ColorMap);
        }
        }
        float NDF = DistributionGGX(N,H,roughness);
        float G = GeometrySmith(N,V,L,roughness);
        vec3 F = FresnelSchlick(max(dot(H,V),0.0),F0);
        //vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

        float NdotL = max(dot(N,L),0.0);

        vec3 kS = F;
        vec3 Kd = vec3(1.0) - kS;
        Kd *= 1.0 - Metalic;

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * DotNV * NdotL + 0.0001;
        vec3 specular = numerator / denominator;

        Lo += (1.0 - shadow) * (Kd * Albedo / PI + specular) * radiance * CurrentLight.Intensity * NdotL;
    }

    vec3 color;
    vec3 ambient;

    if(EnableIBL)
    {
        vec3 F = fresnelSchlickRoughness(DotNV, F0, roughness);

        vec3 R = reflect(-V , N);
        const float MAX_REFLECTION_LOD = 4.0f;
        vec3 prefilteredColor = textureLod(prefilteredMap,R,roughness * MAX_REFLECTION_LOD).rgb;
        vec2 EnvLut = texture(LUT,vec2(DotNV,roughness)).rg;
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

    FragColor = vec4(color + (FinalFogColor * FogIntensity), 1.0); 
    //FragColor = vec4(Albedo, 1.0); 
    Depth = vec4(Position,1.0f);
    ID = vec4(vec3(ModelID),1.0f);
}