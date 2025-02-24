#version 460 core
#extension GL_ARB_gpu_shader_int64 : enable

#define VNDF_SAMPLING

layout(local_size_x=32,local_size_y=32) in;
layout(rgba32f,binding=0) uniform image2D image;
layout(rgba32f,binding=1) uniform image2D NormalImage;
layout(rgba32f,binding=2) uniform image2D AlbedoImage;

//Utility uniforms
layout (location = 0) uniform vec2 WindowSize;
layout (location = 1) uniform vec3 CameraPosition;
layout (location = 2) uniform mat4 ProjectionViewMat;
layout (location = 3) uniform float Time;

//Material data
layout (location = 4) uniform samplerBuffer ModelAlbedos;
layout (location = 5) uniform samplerBuffer ModelRoughness;
layout (location = 6) uniform samplerBuffer ModelMetallic;
layout (location = 7) uniform samplerBuffer ModelAlphas;
layout (location = 8) uniform samplerBuffer ModelEmissives;
layout (location = 9) uniform samplerBuffer ModelClearCoats;

readonly layout(std430,binding=9) readonly restrict buffer ModelTextureHandles
{
   uint64_t TexturesHandles[];
};

//Geometry data
layout (location = 10) uniform samplerBuffer TriangleUVS;

layout (location = 11) uniform int ModelCount;
layout (location = 12) uniform samplerCube EnvironmentCubeMap;
layout (location = 13) uniform samplerCube ConvolutedEnvironmentCubeMap;
layout (location = 14) uniform int ProgressiveRenderedFrameCount;

layout (location = 15) uniform int LightCount;
layout (location = 16) uniform float CameraPlaneDistance;
layout (location = 17) uniform float RandomSeed;

layout (location = 18) uniform samplerBuffer EmissiveObjects;
layout (location = 19) uniform float TotalLightIntensity;

layout (location = 20) uniform int TargetSampleCount;
layout (location = 21) uniform vec2 EnvMapBinSize;
layout (location = 22) uniform int AlbedoCount;
layout (location = 23) uniform int NodeCount;
layout (location = 24) uniform int TriangleCount;

layout (location = 25) uniform bool ShouldDisplayTheEnv;
layout (location = 26) uniform int BounceCount;
layout (location = 27) uniform float EnvironmentLightIntensity;
layout (location = 28) uniform vec4 DoFattributes;
layout (location = 29) uniform float TotalEmissiveArea;
struct Light
{
  vec4 Position;
  vec4 Color;
  int Type;
  float Intensity;
  float Radius;
  int ShadowMapIndex;
};

#define POINT_LIGHT 0x56400
#define DIRECTIONAL_LIGHT 0x56401
#define SPOT_LIGHT 0x56402

layout(std430 , binding = 4) readonly restrict buffer LightsDatas
{
    Light Lights[];
};

layout(std430 , binding = 20) readonly restrict buffer BVHvec4Buffer
{
    vec4 Bounds[];
};

layout(std430 , binding = 21) readonly restrict buffer BVHfloatBuffer
{
    int BVHfloatData[];
};

layout(std430 , binding = 18) readonly restrict buffer MeshBuffer
{
    vec4 MeshData[];
};

#define BIN_COUNT 64 * 64 * 6

layout(std430,binding=12) readonly restrict buffer RadianceBins
{
   float Radiances[BIN_COUNT];
   int Face[BIN_COUNT];
   vec4 Bins[BIN_COUNT];
};

#define BACKGROUND_COLOR vec4(1.0f)
const float pos_infinity = uintBitsToFloat(0x7F800000);
const float neg_infinity = uintBitsToFloat(0xFF800000);
const float PI = 3.14159265359;
const float Inv_PI = 1.0f / PI;
const vec3 UP_VECTOR = vec3(0.0f,1.0f,0.0f);
const float Epsilon = 1e-4;

struct IntersectionData
{
   float t;
   vec2 uv;
};

struct RayData
{
   vec3 Position;
   vec3 Normal;
   vec4 Albedo;
   vec4 Emissive;
   float Metallic;
   float Roughness;
   float Alpha;
   float IOR;
   float ClearCoat;
   float ClearCoatRoughness;
   float Sheen;
   float SheenTint;
   float Anisotropy;
   vec2 uv;
   vec3 Light;
};

struct SampleData
{
   vec3 Direction;
   vec3 MicroFacetNormal;
   float pdf;
   float Fresnel;
   float Eta_i;
   float Eta_t;
   int lobe;
   float Roughness;
};

const float InvLightCount = 1.0f / max(1.0f,float(LightCount));
uint seed;
int NodesToProcess[27];
float cdf[10];

float EmissiveBoost = 04.0f;
float InvEmissiveLightArea = 1.0f / TotalEmissiveArea;
float RootNodeDistance = distance(Bounds[0],Bounds[NodeCount]);

float Hash(inout int x)
{
	x ^= x >> 16;
    x *= 0x85ebca6bU; 
    x ^= x >> 13;
    x *= 0xc2b2ae35U; 
    x ^= x >> 16;
    return float(x) / float(0xFFFFFFFFU); 
}

double HashD(inout int x)
{
    x ^= x >> 16;
    x *= 0x85ebca6bU; 
    x ^= x >> 13;
    x *= 0xc2b2ae35U; 
    x ^= x >> 16;
    return double(x) / double(0xFFFFFFFFU); 
}

IntersectionData RayTriangleIntersection(vec3 rayOrigin,vec3 rayDirection,vec3 vertex0,vec3 vertex1,vec3 vertex2)
{
   IntersectionData result;
   result.t = -1.0f;
   result.uv = vec2(0.0f);

   const float EPSILON = 1e-8;
   vec3 e1 = vertex1 - vertex0;
   vec3 e2 = vertex2 - vertex0;

   vec3 h = cross(rayDirection,e2);
   float determinant = dot(h,e1);

   if(determinant > -EPSILON && determinant < EPSILON)
   {
     return result;
   }

   float invDeterminant = 1.0f / determinant;
  
   vec3 T = rayOrigin - vertex0;
   vec3 Q = cross(T,e1);

   float u = dot(h,T) * invDeterminant;
   if(u < 0.0f || u > 1.0f)
   {
     return result;
   }

   float v = dot(Q,rayDirection) * invDeterminant;
   if(v < 0.0f || (u + v) > 1.0f)
   {
     return result;
   }
   
   float t = dot(Q,e2) * invDeterminant;
   if(t > EPSILON)
   {
     result.t = t;
     result.uv = vec2(u,v);
   }
   return result;
}

float RayAABBIntersection(in vec3 rayOrigin,in vec3 rayDirectionInv,in vec3 BoxMin,in vec3 BoxMax)
{
   vec3 tMin = (BoxMin.xyz - rayOrigin) * rayDirectionInv;
   vec3 tMax = (BoxMax.xyz - rayOrigin) * rayDirectionInv;

   vec3 t1 = min(tMin,tMax);
   vec3 t2 = max(tMin,tMax);
  
   float NearDistance = max(max(t1.x,t1.y),t1.z);
   float FarDistance = min(min(t2.x,t2.y),t2.z);

   return (FarDistance >= NearDistance && FarDistance > 0) ? NearDistance : pos_infinity;
}

vec4 RayTraceSphere(in vec3 ro,in vec3 rd,float sphereRadius)
{
    vec3 rayOrigin = ro;
    vec3 rayDirection = rd;
    float SphereRadius = sphereRadius;

    float a = dot(rayDirection, rayDirection);
    float b = dot(rayDirection.xyz,rayOrigin);
    float c = dot(rayOrigin,rayOrigin) - (SphereRadius*SphereRadius);

    float discr = (b * b) - (a*c); 

    if(discr >= 0.0f)
    {
      float t0 = (-b - sqrt(discr)) / 2 * a;
      float t1 = (-b + sqrt(discr)) / 2 * a;

      vec3 HitCoordinate0 = ro + rd * t0;
      vec3 HitCoordinate1 = ro + rd * t1;

      vec3 ViewDirection = normalize(HitCoordinate1 - ro);
      vec3 LightDirection = vec3(sin(Time),0.6f,0.5f);
      vec3 Normal = normalize(-HitCoordinate0);
      float Ambient = 0.01f;

      vec3 H = normalize(ViewDirection + LightDirection);
      float LdotN = dot(LightDirection,Normal);
      float HdotN = pow(max(0.0f,dot(H,Normal)),32.0f);
      
      //return vec4(vec3(Ambient + HdotN + LdotN),1.0f);
      return vec4(1.0f,0.0f,0.0,1.0f);
    }
    return BACKGROUND_COLOR;
}


RayData TraverseBVH(in vec3 rayOrigin,in vec3 rayDirection,in vec3 InvRayDirection,inout float ClosestDistance,in bool SampleMaterial,in int LODlevel)
{
     int StackIndex = 0; 
     NodesToProcess[StackIndex] = 0;
     StackIndex++;

     int CurrentIndex;
     
     RayData data;
     data.Normal = vec3(0.0f);
     data.Position = vec3(0.0f);
     data.Albedo = vec4(0.0f,0.0f,0.0f,1.0f);
     data.Emissive = vec4(0.0f);
     data.Roughness = 0.0f;
     data.Metallic = 0.0f;
     data.Alpha = 1.0f;
     data.uv = vec2(0.0f);
     data.Light = vec3(0.0f);

     vec4 v0 = vec4(0.0f);
     vec3 v1 = vec3(0.0f);
     vec3 v2 = vec3(0.0f);

     vec3 n0 = vec3(0.0f);
     vec3 n1 = vec3(0.0f);
     vec3 n2 = vec3(0.0f);

     vec3 Normal = vec3(0.0f);
     vec3 TextureNormal = vec3(0.0f);
     vec3 Tangent;
     vec3 Bitangent;
     vec3 up;
     vec2 TriangleUV = vec2(0.0f);
     vec2 TriangleTextureUV = vec2(0.0f);

     mat3 TBNmat = mat3(1.0);

     uint64_t TextureHandle;

     float DistanceChild0 = 0.0f;
     float DistanceChild1 = 0.0f;  
     
     int CurrentModelID = -1;
     int ChildIndex = -1;

     int NearChildIndex = -1;
     int FarChildIndex = -1;

     int iTrip;
     int TriangleIndex;
     int TriCount;

     vec2 TempVec2;

     while(StackIndex > 0)
     {
        StackIndex--;
        CurrentIndex = NodesToProcess[StackIndex];
        if(CurrentIndex >= NodeCount) break;

        ChildIndex = int(BVHfloatData[CurrentIndex]);
        
        if(ChildIndex == -1)
        {
            TriangleIndex = BVHfloatData[CurrentIndex + NodeCount];
            TriCount = BVHfloatData[CurrentIndex + 2 * NodeCount];
            
            for(int i = TriangleIndex;i < TriangleIndex + TriCount;i++)
            {
                v0 = MeshData[TriangleCount * 3 + i * 3].xyzw;
                v1 = MeshData[TriangleCount * 3 + i * 3 + 1].xyz;
                v2 = MeshData[TriangleCount * 3 + i * 3 + 2].xyz;

                CurrentModelID = int(v0.w);                
                IntersectionData result = RayTriangleIntersection(rayOrigin,rayDirection.xyz,v0.xyz,v1,v2);

                if(result.t > 0.0f)
                {
                    if(result.t < ClosestDistance)
                    {
                        TriangleUV = result.uv;
                        float OneMinusUV = (1.0f - TriangleUV.x - TriangleUV.y);
                        iTrip = i * 3;

                        n0.xy = texelFetch(TriangleUVS,iTrip).xy;
                        n1.xy = texelFetch(TriangleUVS,iTrip + 1).xy;
                        n2.xy = texelFetch(TriangleUVS,iTrip + 2).xy;

                        TriangleTextureUV = OneMinusUV * n0.xy + TriangleUV.x * n1.xy + TriangleUV.y * n2.xy;

                        if(SampleMaterial)
                        {
                            TextureHandle = TexturesHandles[CurrentModelID * 6 + 2];
                            data.Roughness = clamp(TextureHandle != 0 ? textureLod(sampler2D(TextureHandle),TriangleTextureUV,float(LODlevel)).x :
                                                                                   texelFetch(ModelRoughness,CurrentModelID).x,0.001f,0.999f);
                         

                            TextureHandle = TexturesHandles[CurrentModelID * 6 + 3];
                            data.Metallic = TextureHandle != 0 ? textureLod(sampler2D(TextureHandle),TriangleTextureUV,float(LODlevel)).x :
                                                                 texelFetch(ModelMetallic,CurrentModelID).x;
                            
                            //Specular to metallic
                            //data.Metallic = clamp((data.Roughness - 0.04f) / 0.91f,0.0f,1.0f); 

                            //Specular to roughness
                            //data.Roughness = 1.0f - sqrt(data.Roughness); 

                            TempVec2 = texelFetch(ModelClearCoats,CurrentModelID).xy;
                            data.ClearCoat = TempVec2.x;
                            data.ClearCoatRoughness = TempVec2.y;
                           
                            data.uv = TriangleTextureUV;
                        }
                        
                        TextureHandle = TexturesHandles[CurrentModelID * 6 + 4];
                        if (TextureHandle != 0) {
                            data.Alpha = textureLod(sampler2D(TextureHandle),TriangleTextureUV,float(LODlevel)).x;
                            data.IOR = texelFetch(ModelAlphas,CurrentModelID).y;         
                        }
                        else
                        {
                            TempVec2 = texelFetch(ModelAlphas,CurrentModelID).xy;
                            data.Alpha = TempVec2.x;
                            data.IOR = TempVec2.y;
                        }

                        if(data.Alpha < 1.0f || SampleMaterial)
                        {
                            n0 = MeshData[iTrip].xyz;
                            n1 = MeshData[iTrip + 1].xyz;
                            n2 = MeshData[iTrip + 2].xyz;

                            Normal = normalize(OneMinusUV * n0 + TriangleUV.x * n1 + TriangleUV.y * n2);

                            TextureHandle = TexturesHandles[CurrentModelID * 6 + 1];
                            if (TextureHandle != 0) 
                            {
                                iTrip *= 2;

                                n0 = MeshData[TriangleCount + iTrip].xyz;
                                n1 = MeshData[TriangleCount + iTrip + 2].xyz;
                                n2 = MeshData[TriangleCount + iTrip + 4].xyz;

                                Tangent = normalize(OneMinusUV * n0 + TriangleUV.x * n1 + TriangleUV.y * n2);

                                n0 = MeshData[TriangleCount + iTrip + 1].xyz;
                                n1 = MeshData[TriangleCount + iTrip + 3].xyz;
                                n2 = MeshData[TriangleCount + iTrip + 5].xyz;

                                Bitangent = normalize(OneMinusUV * n0 + TriangleUV.x * n1 + TriangleUV.y * n2);

                                TBNmat = mat3(Tangent,Bitangent,Normal);
                                Normal = texture(sampler2D(TextureHandle),TriangleTextureUV).xyz * 2.0f - 1.0f;
                                Normal = normalize(TBNmat * Normal);
                                data.Normal = Normal;
                            }
                            else
                            {
                                data.Normal = Normal;
                            }

                            TextureHandle = TexturesHandles[CurrentModelID * 6];
                            if (TextureHandle != 0) {
                                data.Albedo = textureLod(sampler2D(TextureHandle),TriangleTextureUV,float(LODlevel)).xyzw;                      
                            }
                            else
                            {
                                data.Albedo = texelFetch(ModelAlbedos,CurrentModelID);
                            } 
                           
                        }
                        data.Position = rayOrigin + rayDirection * result.t;
                       
                        TextureHandle = TexturesHandles[CurrentModelID * 6 + 5];
                        data.Emissive.xyzw = TextureHandle != 0 ? textureLod(sampler2D(TextureHandle),TriangleTextureUV,float(LODlevel)).xyzw :
                                                                    texelFetch(ModelEmissives,CurrentModelID).xyzw;
                        ClosestDistance = result.t;
                    }  
                }
            }
        }
        else
        {
               DistanceChild0 = RayAABBIntersection(rayOrigin,InvRayDirection,Bounds[ChildIndex].xyz,Bounds[ChildIndex + NodeCount].xyz);
               DistanceChild1 = RayAABBIntersection(rayOrigin,InvRayDirection,Bounds[ChildIndex + 1].xyz,Bounds[ChildIndex + NodeCount + 1].xyz);

               if (DistanceChild0 < ClosestDistance || DistanceChild1 < ClosestDistance) {
                    if(DistanceChild0 >= DistanceChild1)
                    {
                        if(DistanceChild0 < ClosestDistance)
                        {
                            NodesToProcess[StackIndex] = ChildIndex;
                            StackIndex++;
                        }
                        if(DistanceChild1 < ClosestDistance)
                        {
                            NodesToProcess[StackIndex] = ChildIndex + 1;
                            StackIndex++;
                        }
                    }
                    else
                    {
                        if(DistanceChild1 < ClosestDistance)
                        {
                            NodesToProcess[StackIndex] = ChildIndex + 1;
                            StackIndex++;
                        }
                        if(DistanceChild0 < ClosestDistance)
                        {
                            NodesToProcess[StackIndex] = ChildIndex;
                            StackIndex++;
                        }
                    } 
                
                }
        }
     }
     return data;
}

float Luminance(vec3 c)
{
    return 0.212671 * c.x + 0.715160 * c.y + 0.072169 * c.z;
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
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

    return num / max(denom,1e-5);
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

float FresnelSchlick(float cosTheta , float F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta,0.0,1.0),5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 


vec3 SampleSemiSphere(vec2 InputSeed,vec3 Normal)
{
   float phi = 2.0f * PI * InputSeed.x;
   float cosTheta = sqrt(InputSeed.y);       
   float sinTheta = sqrt(1.0f - InputSeed.y); 

   vec3 SampledVector = vec3(cos(phi) * sinTheta,
                             sin(phi) * sinTheta,
                             cosTheta);

   vec3 Tangent = abs(Normal.z) < 0.9999 ? normalize(cross(vec3(0.0, 0.0, 1.0), Normal)) 
                                          : normalize(cross(vec3(1.0, 0.0, 0.0), Normal));   
   vec3 Bitangent = normalize(cross(Tangent,Normal));
                              
   return normalize(SampledVector.x * Tangent + SampledVector.y * Bitangent + SampledVector.z * Normal);
}

float RayTraceShadows(in vec3 CurrentPosition,in vec3 LightDirection,inout vec3 ShadowColor)
{
     vec3 SampleLightDirection = vec3(0.0f);
     float Shadow = 0.0f;
     float ClosestDistance = 0.0f;
     RayData data;
     float HitAlpha = 0.0f;
     vec3 Position = CurrentPosition;
     vec3 InvRayDirection;
     int iterator = 0;
     do
     {
         ClosestDistance = pos_infinity;
         SampleLightDirection = LightDirection;
         InvRayDirection = 1.0f / (SampleLightDirection);
         data = TraverseBVH(Position + Epsilon * distance(CameraPosition,Position) * SampleLightDirection , SampleLightDirection,InvRayDirection,ClosestDistance,false,0);
         HitAlpha = data.Alpha;
         if(ClosestDistance != pos_infinity)
         {
            ShadowColor *= HitAlpha < 1.0f ? data.Albedo.xyz : vec3(1.0f);
            Shadow += HitAlpha < 1.0f ? mix(Luminance(FresnelSchlick(abs(dot(data.Normal,LightDirection)),vec3(0.04f))),1.0f,HitAlpha) : 1.0f;
            Position = data.Position;
         }
         else break;
         iterator++;
     }
     while(iterator < 5 && HitAlpha < 1.0f && length(Shadow) < 1.0f);
   
     return min(1.0f,Shadow);
}

float RayTracePositionalShadows(in vec3 CurrentPosition,in vec3 LightDirection,in vec3 LightPosition,inout vec4 HitEmissive,inout vec3 ShadowColor)
{
     vec3 SampleLightDirection = vec3(0.0f);
     float Shadow = 0.0f;
     float ClosestDistance = 0.0f;
     RayData data;
     float HitAlpha = 0.0f;
     vec3 Position = CurrentPosition;
     vec3 InvRayDirection;
     float DistancePositions;
     int iterator = 0;
     do
     {
         ClosestDistance = pos_infinity;
         SampleLightDirection = LightDirection;
         InvRayDirection = 1.0f / (SampleLightDirection);
         data = TraverseBVH(Position + Epsilon * SampleLightDirection , SampleLightDirection,InvRayDirection,ClosestDistance,false,0);
         if(ClosestDistance != pos_infinity)
         {
            if(length(data.Emissive.xyz) > 0.0f) 
            {
               HitEmissive = data.Emissive;
               return min(1.0f,Shadow);
            }
            if(distance(CurrentPosition,LightPosition) <= distance(CurrentPosition,data.Position)) return 0.0f;
            
            HitAlpha = data.Alpha;
            ShadowColor *= HitAlpha < 1.0f ? data.Albedo.xyz : vec3(1.0f);
            Shadow += HitAlpha < 1.0f ? mix(Luminance(FresnelSchlick(abs(dot(data.Normal,LightDirection)),vec3(0.04f))),1.0f,HitAlpha) : 1.0f;
            Position = data.Position;
         }
         else break;
         iterator++;
     }
     while(iterator < 5 && HitAlpha < 1.0f && length(Shadow) < 1.0f);
   
     return min(1.0f,Shadow);
}

vec3 CookTorranceBRDFspec(vec3 Normal, vec3 ViewDirection, vec3 LightDirection,float Roughness,float Metallic,vec3 Albedo,in float IOR)
{
    float NdotL = dot(Normal, LightDirection);
    if (NdotL <= 0.0)
        return vec3(0.0);

    float R0 = pow((1.0 - IOR) / (1.0 + IOR), 2.0);
    vec3 F0 = mix(vec3(R0), Albedo, Metallic);
    vec3 H = normalize(ViewDirection + LightDirection);
    vec3 F = fresnelSchlickRoughness(max(dot(Normal, H), 0.0), F0, Roughness);

    float NDF = DistributionGGX(Normal, H, Roughness);
    float G = GeometrySmith(Normal, ViewDirection, LightDirection, Roughness);

    float NdotV = max(dot(Normal, ViewDirection), 0.0);
    return max((NDF * G * F) / max(4.0 * max(NdotL, 0.0) * NdotV, 1e-6),0.0f);
}

vec3 CookTorranceBRDF(vec3 Normal, vec3 ViewDirection, vec3 LightDirection,vec3 LightColor,float Roughness,float Metallic,vec3 Albedo,in float IOR)
{
    float NdotL = dot(Normal, LightDirection);
    if (NdotL <= 0.0)
        return vec3(0.0);

    float R0 = pow((1.0 - IOR) / (1.0 + IOR), 2.0);
    vec3 F0 = mix(vec3(R0), Albedo, Metallic);
    vec3 H = normalize(ViewDirection + LightDirection);

    vec3 F = fresnelSchlickRoughness(max(dot(Normal, H), 0.0), F0, Roughness);
    //vec3 F = FresnelSchlick(max(dot(Normal, H), 0.0), F0);
    float NDF = DistributionGGX(Normal, H, Roughness);
    float G = GeometrySmith(Normal, ViewDirection, LightDirection, Roughness);

    float NdotV = max(dot(Normal, ViewDirection), 0.0);
    vec3 kD = (1.0 - F) * (1.0 - Metallic);

    vec3 specular = (NDF * G * F) / max(4.0 * max(NdotL, 0.0) * NdotV, 1e-4);
    return max((kD * Albedo * Inv_PI + specular) * LightColor,0.0f);
}

//Tutorial https://schuttejoe.github.io/post/disneybsdf/
vec3 MicroFacetBTDF(vec3 Normal,vec3 IncidentDirection,vec3 OutgoingDirection,float eta_i,float eta_t,float Roughness,float Metallic,vec3 Albedo) 
{
    float NdotL = dot(Normal, OutgoingDirection);

    vec3 H = normalize(IncidentDirection + OutgoingDirection);

    vec3 F0 = mix(vec3(0.04), Albedo, Metallic);
    vec3 F = fresnelSchlickRoughness(max(dot(Normal, H), 0.0), F0, Roughness);
    float NDF = DistributionGGX(Normal, H, Roughness);
    float G = GeometrySmith(Normal,IncidentDirection,OutgoingDirection,Roughness);

    float NdotV = dot(Normal,IncidentDirection);
    float LdotH = dot(OutgoingDirection,H);
    float VdotH = dot(IncidentDirection,H);

    float eta = eta_t / eta_i;
    float denom = LdotH + VdotH * eta;
    float jacobian = max(0.0001f,abs(LdotH)) / max(0.0001f,(denom * denom));

    float eta2 = eta * eta;
    vec3 BTDF = (1.0 - F) * NDF * G * max(0.0001f,abs(VdotH)) * jacobian * eta2 / max(0.0001f,abs(NdotL * NdotV));
    return max(BTDF,0.0f);
}

float PowerHeuristic(float pdf0,float pdf1)
{
    float pdf02 = pow(pdf0,2);
    return pdf02 / (pdf02 + pow(pdf1,2));
}

vec3 SampleGGX(vec2 InputSeed,vec3 Normal,float Roughness)
{
   float a = Roughness * Roughness;
   float phi = 2.0f * PI * InputSeed.x;
   float cosTheta = sqrt((1.0 - InputSeed.y) / (1.0 + (a * a - 1.0) * InputSeed.y));
   float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

   vec3 SampledVector = vec3(cos(phi) * sinTheta,
                             sin(phi) * sinTheta,
                             cosTheta);

   vec3 Tangent = abs(Normal.z) < 0.9999 ? normalize(cross(vec3(0.0, 0.0, 1.0), Normal)) 
                                          : normalize(cross(vec3(1.0, 0.0, 0.0), Normal));   
   vec3 Bitangent = normalize(cross(Tangent,Normal));
                              
   return normalize(SampledVector.x * Tangent + SampledVector.y * Bitangent + SampledVector.z * Normal);
}

//VNDF microfacet distribution for sampling 
//Source:https://jcgt.org/published/0007/04/01/paper.pdf
vec3 sampleGGXVNDF(vec3 Ve, float alpha_x, float alpha_y, float U1, float U2)
{
    vec3 Vh = normalize(vec3(alpha_x * Ve.x, alpha_y * Ve.y, Ve.z));
    
    float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
    vec3 T1 = lensq > 0 ? vec3(-Vh.y, Vh.x, 0) * inversesqrt(lensq) : vec3(1,0,0);
    vec3 T2 = cross(Vh, T1);

    float r = sqrt(U1);
    float phi = 2.0 * PI * U2;
    float t1 = r * cos(phi);
    float t2 = r * sin(phi);
    float s = 0.5 * (1.0 + Vh.z);
    t2 = (1.0 - s)*sqrt(1.0 - t1*t1) + s*t2;
    
    vec3 Nh = t1*T1 + t2*T2 + sqrt(max(0.0, 1.0 - t1*t1 - t2*t2))*Vh;
    
    vec3 Ne = normalize(vec3(alpha_x * Nh.x, alpha_y * Nh.y, max(0.0, Nh.z)));
    return Ne;
}

vec3 Tint(vec3 Albedo)
{
  float Luminance = dot(vec3(0.3f, 0.6f, 1.0f), Albedo);
  return (Luminance > 0.0f) ? Albedo * (1.0f / Luminance) : vec3(1.0f);
}

float SchlickWeight(float cosTheta) {
    float m = clamp(1 - cosTheta, 0, 1);
    return (m * m) * (m * m) * m;
}

//Disney BSDFs Sheen lobe
//Source:https://schuttejoe.github.io/post/disneybsdf/ 
vec3 EvalSheen(in SampleData Sample,in vec3 IncidentDirection,in vec3 OutgoingDirection,in float Sheen,in float SheenTint,in vec3 Albedo)
{
    if(Sheen <= 0.0f) {
        return vec3(0.0f);
    }

    float HdotL = dot(IncidentDirection,OutgoingDirection);
    vec3 tint = Tint(Albedo);
    return Sheen * mix(vec3(1.0f),tint,SheenTint) * SchlickWeight(HdotL);
}

void SampleBSDF(inout SampleData Sample,in vec3 Direction,in vec3 Normal,in vec3 Albedo,in vec3 Position,
                in float Roughness,in float Metallic,in float Alpha,float ior,in float Anisotropy,in float ClearCoat)
{
    float pdf;
    vec3 SampleVector; 
    float AveragedFresnel = Sample.Fresnel;
    vec3 H = vec3(0.0f);
    bool IsReflectionLobe = false;

    float CoinFlip = Hash(seed);
   
    float DielectricWeight = (1.0 - Metallic) * Alpha;
    float MetalWeight = Metallic;
    float GlassWeight = (1.0 - Metallic) * (1.0f - Alpha);

    float DiffuseProbability = DielectricWeight * Luminance(Albedo);
    float DielectricProbability = DielectricWeight * AveragedFresnel;

    float DiffuseShadingProbability = DielectricWeight * Roughness * Luminance(Albedo);
    float DielectricShadingProbability = DielectricWeight * mix(0.0f,1.0f,AveragedFresnel);
     
    float MetalProbability = MetalWeight * Luminance(mix(Albedo, vec3(1.0), AveragedFresnel));
    float GlassProbability = GlassWeight;
    float ClearCoatProbability = 0.25 * ClearCoat;

    float InverseTotalWeight = 1.0 / (DiffuseProbability + DielectricProbability + MetalProbability + GlassProbability + ClearCoatProbability);
    float InverseTotalShadingWeight = 1.0 / (DiffuseShadingProbability + DielectricShadingProbability + MetalProbability + GlassProbability + ClearCoatProbability);

    cdf[5] = DiffuseShadingProbability * InverseTotalShadingWeight;
    cdf[6] = DielectricShadingProbability * InverseTotalShadingWeight;
    cdf[7] = MetalProbability * InverseTotalShadingWeight;
    cdf[8] = GlassProbability * InverseTotalShadingWeight;
    cdf[9] = ClearCoatProbability * InverseTotalShadingWeight;

    DiffuseProbability *= InverseTotalWeight;
    DielectricProbability *= InverseTotalWeight;
    MetalProbability *= InverseTotalWeight;
    GlassProbability *= InverseTotalWeight;
    ClearCoatProbability *= InverseTotalWeight;

    cdf[0] = DiffuseProbability;
    cdf[1] = cdf[0] + DielectricProbability;
    cdf[2] = cdf[1] + MetalProbability;
    cdf[3] = cdf[2] + GlassProbability;
    cdf[4] = cdf[3] + ClearCoatProbability;
   

    if(CoinFlip < cdf[0])
    {
        Sample.lobe = 0;
        vec3 LambertianSample = SampleSemiSphere(vec2(Hash(seed),Hash(seed)),Normal);
        SampleVector = LambertianSample; 

        float pdfDiffuse = abs(dot(Normal, LambertianSample)) / PI;
        pdf = pdfDiffuse * DiffuseProbability;
    }
    else if(CoinFlip < cdf[1] || CoinFlip < cdf[2])
    {
        if(CoinFlip < cdf[1])
        {
           Sample.lobe = 1;
           pdf = DielectricShadingProbability;  
        }
        else if(CoinFlip < cdf[2])
        {
           Sample.lobe = 2;
           pdf = MetalProbability;  
        }
         
        if(Roughness <= 1e-3)
        {
           SampleVector = reflect(Direction,Normal);
        }
        else
        {   
#ifdef GGX_SAMPLING
            GGX_H = SampleGGX(vec2(Hash(seed),Hash(seed)),Normal,Roughness);
            vec3 GGXsample = reflect(Direction,GGX_H);
            SampleVector = GGXsample; 
#elif defined(VNDF_SAMPLING)     
            vec3 Tangent = normalize(cross(vec3(0, 1, 0), Normal));
            vec3 Bitangent = cross(Normal, Tangent);
            vec3 TangentSpaceViewVector = vec3(dot(-Direction,Tangent),dot(-Direction,Bitangent),dot(-Direction,Normal));

            float a = max(Roughness * Roughness,1e-3);
            float Aspect = sqrt(1.0f - 0.9f * Anisotropy); 
            float ax = a / Aspect; 
            float ay = a * Aspect; 
            vec3 GGX_H = sampleGGXVNDF(TangentSpaceViewVector,ax,ay,Hash(seed),Hash(seed));
            
            if (GGX_H.z < 0.0) GGX_H = -GGX_H;
            
            vec3 GGXsample = reflect(-TangentSpaceViewVector,GGX_H);
            SampleVector = normalize(GGXsample.x * Tangent + GGXsample.y * Bitangent + GGXsample.z * Normal);
           
            H = normalize(GGX_H.x * Tangent + GGX_H.y * Bitangent + GGX_H.z * Normal);
            float NDF = DistributionGGX(Normal, H, Roughness);
            float pdfGGX = (NDF * max(dot(Normal, H), 0.0f)) / max(4.0f * max(dot(H, -Direction), 0.0f),0.0001); 
            pdf *= pdfGGX;
#endif
        }
    }
    else if(CoinFlip < cdf[3])
    {    
        if(Roughness <= 1e-3)
        {
            float VdotH = dot(Direction,Normal);
            bool IsReflect = VdotH < 0.0f;
            float Eta_i = IsReflect ? ior : 1.0f;
            float Eta_t = IsReflect ? 1.0f : ior;
            float RelativeIOR = Eta_t / Eta_i;

            vec3 refracted = refract(Direction,IsReflect ? Normal : -Normal,RelativeIOR);
            vec3 reflected = reflect(Direction,Normal);
            Sample.Eta_i = Eta_i;
            Sample.Eta_t = Eta_t;
            float FresnelWeight = SchlickWeight(abs(dot(-Direction,Normal)));

            bool ShouldReflect = length(refracted) == 0.0 || ((CoinFlip - cdf[2]) / (cdf[3] - cdf[2])) < FresnelWeight;

            if (ShouldReflect) SampleVector = reflected;
            else SampleVector = refracted;

            IsReflectionLobe = ShouldReflect && VdotH * dot(SampleVector,Normal) > 0.0f;
           
            if(IsReflectionLobe)
            {
               pdf = GlassProbability;
               Sample.lobe = 1;
            }
            else
            {
               pdf = GlassProbability;
               Sample.lobe = 3;
            }
        }
        else
        {
            if(length(H) <= 0.0f)
            {
                float a = max(Roughness * Roughness,1e-3);
                float Aspect = sqrt(1.0f - 0.9f * Anisotropy); 
                float ax = a / Aspect; 
                float ay = a * Aspect; 
                vec3 Tangent = normalize(cross(vec3(0, 1, 0), Normal));
                vec3 Bitangent = cross(Normal, Tangent);
                vec3 TangentSpaceViewVector = vec3(dot(-Direction,Tangent),dot(-Direction,Bitangent),dot(-Direction,Normal));

                vec3 GGX_H = sampleGGXVNDF(TangentSpaceViewVector,a,a,Hash(seed),Hash(seed));
                if (GGX_H.z < 0.0) GGX_H = -GGX_H;  
                H = normalize(GGX_H.x * Tangent + GGX_H.y * Bitangent + GGX_H.z * Normal);
            }

            float VdotH = dot(Direction,H);
            bool IsReflect = VdotH < 0.0f;
            float Eta_i = IsReflect ? ior : 1.0f;
            float Eta_t = IsReflect ? 1.0f : ior;
            float RelativeIOR = Eta_t / Eta_i;

            vec3 refracted = refract(Direction,IsReflect ? H : -H,RelativeIOR);
            vec3 reflected = reflect(Direction,H);
            Sample.Eta_i = Eta_i;
            Sample.Eta_t = Eta_t;

            float FresnelWeight = SchlickWeight(abs(dot(-Direction,H)));

            bool ShouldReflect = length(refracted) == 0.0 || ((CoinFlip - cdf[2]) / (cdf[3] - cdf[2])) < FresnelWeight;

            if (ShouldReflect) SampleVector = reflected;
            else SampleVector = refracted;
           
            IsReflectionLobe = ShouldReflect && VdotH * dot(SampleVector,Normal) > 0.0f;

            if(IsReflectionLobe)
            {
               float NDF = DistributionGGX(Normal, H, Roughness);
               float pdfGGX = (NDF * max(dot(Normal, H), 0.0f)) / max(4.0f * max(dot(H, -Direction), 0.0f),0.0001); 
               pdf = pdfGGX * GlassProbability;
               Sample.lobe = 1;
            }
            else
            {
                float LdotH = abs(dot(SampleVector, H));
                float denum = LdotH + RelativeIOR * VdotH;
                float jacobian = LdotH / (denum * denum);
                float G1_V = GeometrySchlickGGX(max(dot(Normal, -Direction), 0.0), Roughness);
                float NDF = DistributionGGX(Normal, H, Roughness);
                 
                pdf = G1_V * max(0.0, VdotH) * NDF * jacobian * GlassProbability; 
                Sample.lobe = 3;
            }
            
        }
    }
    else
    {
        Sample.lobe = 4;
        if(Roughness <= 1e-3)
        {
           SampleVector = reflect(Direction,Normal);
           pdf = ClearCoatProbability;
        }
        else
        {   
#ifdef GGX_SAMPLING
            GGX_H = SampleGGX(vec2(Hash(seed),Hash(seed)),Normal,Roughness);
            vec3 GGXsample = reflect(Direction,GGX_H);
            SampleVector = GGXsample; 
#elif defined(VNDF_SAMPLING)     
            vec3 Tangent = normalize(cross(vec3(0, 1, 0), Normal));
            vec3 Bitangent = cross(Normal, Tangent);
            vec3 TangentSpaceViewVector = vec3(dot(-Direction,Tangent),dot(-Direction,Bitangent),dot(-Direction,Normal));

            float a = max(Roughness * Roughness,1e-4);
            float Aspect = sqrt(1.0f - 0.9f * Anisotropy); 
            float ax = a / Aspect; 
            float ay = a * Aspect; 
            vec3 GGX_H = sampleGGXVNDF(TangentSpaceViewVector,ax,ay,Hash(seed),Hash(seed));
            
            if (GGX_H.z < 0.0) GGX_H = -GGX_H;
            
            vec3 GGXsample = reflect(-TangentSpaceViewVector,GGX_H);
            SampleVector = normalize(GGXsample.x * Tangent + GGXsample.y * Bitangent + GGXsample.z * Normal);
           
            H = normalize(GGX_H.x * Tangent + GGX_H.y * Bitangent + GGX_H.z * Normal);
            float NDF = DistributionGGX(Normal, H, Roughness);
            float pdfGGX = (NDF * max(dot(Normal, H), 0.0f)) / max(4.0f * max(dot(H, -Direction), 0.0f),0.0001); 
            pdf = ClearCoatProbability * pdfGGX;
#endif
        }
    }

    pdf = max(pdf, 1e-6);
    Sample.Direction = normalize(SampleVector);
    Sample.pdf = pdf;   
    Sample.Roughness = Roughness;
}

void CoinFlipBSDFlobe(inout SampleData Sample,in vec3 Albedo,in float Roughness,in float Metallic,in float Alpha,in float ClearCoat)
{
    float AveragedFresnel = Sample.Fresnel;
   
    float DielectricWeight = (1.0 - Metallic) * Alpha;
    float MetalWeight = Metallic;
    float GlassWeight = (1.0 - Metallic) * (1.0f - Alpha);

    float DiffuseProbability = DielectricWeight * Roughness * Luminance(Albedo);
    float DielectricProbability = DielectricWeight * (1.0f - Roughness) * mix(0.0f,1.0f,AveragedFresnel);
    float MetalProbability = MetalWeight * Luminance(mix(Albedo, vec3(1.0), AveragedFresnel));
    float GlassProbability = GlassWeight;
    float ClearCoatProbability = ClearCoat * 0.25f;

    float InverseTotalWeight = 1.0 / (DiffuseProbability + DielectricProbability + MetalProbability + GlassProbability + ClearCoatProbability);
    DiffuseProbability *= InverseTotalWeight;
    DielectricProbability *= InverseTotalWeight;
    MetalProbability *= InverseTotalWeight;
    GlassProbability *= InverseTotalWeight;
    ClearCoatProbability *= InverseTotalWeight;

    cdf[0] = DiffuseProbability;
    cdf[1] = cdf[0] + DielectricProbability;
    cdf[2] = cdf[1] + MetalProbability;
    cdf[3] = cdf[2] + GlassProbability;
    cdf[4] = cdf[3] + ClearCoatProbability;
    cdf[5] = DiffuseProbability;
    cdf[6] = DielectricProbability;
    cdf[7] = MetalProbability;
    cdf[8] = GlassProbability;
    cdf[9] = ClearCoatProbability;
}

vec3 Eval(in SampleData Sample,in vec3 Radiance,in vec3 Direction,in vec3 LightDirection,
          in vec3 Normal,in float Roughness,in float Metallic,in vec3 Albedo,in float Alpha,
          in float Sheen,in float SheenTint,in float ClearCoat,in float ClearCoatRoughness,in float IOR,in bool IsThroughPut)
{  
    vec3 result = vec3(0.0f);
    if (cdf[5] > 0.0f)
    {
        result += cdf[5] * Radiance * (Albedo * Inv_PI + EvalSheen(Sample,-Direction,LightDirection,Sheen,SheenTint,Albedo));
    }
    if (cdf[6] > 0.0f)
    {
       result += cdf[6] * CookTorranceBRDF(Normal, -Direction, LightDirection, Radiance, Roughness, Metallic, Albedo,IOR);
    }
    if (cdf[7] > 0.0f)
    {   
        result += cdf[7] * Albedo * CookTorranceBRDFspec(Normal, -Direction, LightDirection, Roughness, Metallic, Albedo,IOR) * Radiance;  
    }
    if (cdf[8] > 0.0f)
    {    
       if(IsThroughPut) result += cdf[8] * Radiance * (Albedo + MicroFacetBTDF(Normal, -Direction,LightDirection, Sample.Eta_i, Sample.Eta_t, Roughness, Metallic, Albedo));  
       else result += cdf[8] * Radiance * Albedo * MicroFacetBTDF(Normal, -Direction,LightDirection, Sample.Eta_i, Sample.Eta_t, Roughness, Metallic, Albedo);  
    }
    if (cdf[9] > 0.0f)
    {
        result += cdf[9] * CookTorranceBRDFspec(Normal, -Direction, LightDirection,ClearCoatRoughness,0.0f,Albedo,IOR) * Radiance;
    }
    return result;
}


vec3 SampleLights(in SampleData Sample,vec3 Direction,vec3 N, vec3 Albedo, float Roughness, float Metallic,
                 vec3 Position,in float Alpha,in float Sheen,in float SheenTint,in float ClearCoat,in float ClearCoatRoughness,in float IOR) {
    if(LightCount <= 0) return vec3(0.0f);
    float isShadow = 0.0f;
    vec3 ShadowColor = vec3(1.0f);
    vec4 Emissive = vec4(0.0f);
    int SampledLightIndex = int(Hash(seed) * LightCount);
    vec3 L;
    vec3 Radiance;
    Light CurrentLight = Lights[SampledLightIndex];

    bool IsPointLight = CurrentLight.Type == POINT_LIGHT;
    bool IsDirectionalLight = CurrentLight.Type == DIRECTIONAL_LIGHT;

    if(IsPointLight)
    {
       vec3 LightVec = CurrentLight.Position.xyz - Position;
       float LightDistance = length(LightVec);
       L = normalize(LightVec);     
       Radiance = CurrentLight.Color.xyz * CurrentLight.Intensity / (LightDistance * LightDistance);
    }
    else if(IsDirectionalLight)
    {
       L = normalize(CurrentLight.Position.xyz);
       Radiance = CurrentLight.Color.xyz * CurrentLight.Intensity;
    }

    float ndotl = dot(N,L);
    float NdotL = max(ndotl, 0.0);
    if(NdotL <= 0.0f) return vec3(0.0f,0.0f,0.0f);
    
    if(IsPointLight) isShadow = RayTracePositionalShadows(Position,L,CurrentLight.Position.xyz,Emissive,ShadowColor);
    else if(IsDirectionalLight) isShadow = RayTraceShadows(Position,L,ShadowColor);
    
    if(isShadow >= 1.0f) return vec3(0.0f,0.f,0.0f);
    return NdotL * ShadowColor * (1.0f - isShadow) * Eval(Sample,Radiance,Direction,L,N,Roughness,Metallic,Albedo,Alpha,Sheen,SheenTint,ClearCoat,ClearCoatRoughness,IOR,false);
}

vec3 SampleEmissiveObjects(in SampleData Sample,in vec3 Direction,in vec3 Position,in vec3 Normal,
                           in float Roughness,in float Metallic,in vec3 Albedo,in float Alpha,
                           in float Sheen,in float SheenTint,in float ClearCoat,in float ClearCoatRoughness,in float IOR,inout float LightPDF)
{
     int EmissiveTriangleCount = textureSize(EmissiveObjects);
     if(EmissiveTriangleCount <= 0) return vec3(0.0f);
     float RandomValue = Hash(seed);
     vec2 EmissiveLightData = texelFetch(EmissiveObjects,int(RandomValue * EmissiveTriangleCount)).xy;
     int SampledIndex = int(EmissiveLightData.x) * 3;

     vec2 uv = vec2(RandomValue,Hash(seed));

     vec4 v0 = MeshData[TriangleCount * 3 + SampledIndex].xyzw;
     vec3 v1 = MeshData[TriangleCount * 3 + SampledIndex + 1].xyz;
     vec3 v2 = MeshData[TriangleCount * 3 + SampledIndex + 2].xyz;
     vec3 SampledPosition = (1.0f - uv.x - uv.y) * v0.xyz + uv.x * v1 + uv.y * v2;
     
     vec3 L = normalize(SampledPosition - Position);

     float NdotL = max(dot(L,Normal),0.0f);
     if(NdotL <= 0.0f) return vec3(0.0f);


     vec4 Emissive = vec4(0.0f);
     vec3 ShadowColor = vec3(1.0f);
     float IsVisible = RayTracePositionalShadows(Position,L,SampledPosition,Emissive,ShadowColor);
     if((1.0f - IsVisible) <= 0.0f) return vec3(0.0f);

     vec3 n0 = MeshData[SampledIndex].xyz;
     vec3 n1 = MeshData[SampledIndex + 1].xyz;
     vec3 n2 = MeshData[SampledIndex + 2].xyz;
     vec3 LightNormal = normalize((1.0f - uv.x - uv.y) * n0 + uv.x * n1 + uv.y * n2);
     
     float LdotLnormal = max(0.0f,dot(-L,LightNormal));
     IsVisible = LdotLnormal * NdotL * (1.0f - IsVisible);
     if(IsVisible <= 0.0f) return vec3(0.0f);

     float Distance = length(SampledPosition - Position);
     vec3 Radiance = EmissiveBoost * (Emissive.xyz);
     LightPDF = (Distance * Distance) / (EmissiveLightData.y);

     return ShadowColor * IsVisible * Eval(Sample,Radiance,Direction,L,Normal,Roughness,Metallic,Albedo,Alpha,Sheen,SheenTint,ClearCoat,ClearCoatRoughness,IOR,false);
}

vec3 GetDirection(in vec2 txc, in int face)
{
  vec3 v;
  switch(face)
  {
    case 0: v = vec3( 1.0, -txc.x, txc.y); break; 
    case 1: v = vec3(-1.0,  txc.x, txc.y); break; 
    case 2: v = vec3( txc.x,  1.0, txc.y); break; 
    case 3: v = vec3(-txc.x, -1.0, txc.y); break; 
    case 4: v = vec3(txc.x, -txc.y,  1.0); break; 
    case 5: v = vec3(txc.x,  txc.y, -1.0); break; 
  }
  return normalize(v);
}

int binarySearchRadiance(int low, int high, double x)
{
    while (low <= high) {
        int mid = low + (high - low) / 2;
        double MidValue = Radiances[mid];
        double OneBeforeMidValue = Radiances[mid - 1];

        if (x > OneBeforeMidValue && x <= MidValue)
            return mid;

        if (MidValue < x)
            low = mid + 1;
        else
            high = mid - 1;
    }

    return -1;
}

vec3 SampleEnvironment(in vec3 SampledVector,in float Intensity,RayData data)
{
    float Radiance;
    int ChosenIndex;
    int TotalBinCount = int(EnvMapBinSize.x * EnvMapBinSize.y) * 6;

    double RandomValue = HashD(seed);
    vec3 Direction;
    int itr = 0;
    do
    {
        if(itr >= 5) return vec3(0.0f);
        ChosenIndex = binarySearchRadiance(0,TotalBinCount,RandomValue);

        Radiance = Radiances[ChosenIndex];
    
        vec4 Bin = Bins[ChosenIndex];
        int face = Face[ChosenIndex];

        vec2 UV = Bin.xy + ((Bin.zw - Bin.xy) * vec2(Hash(seed),Hash(seed)));
        Direction = GetDirection(UV,face);
        itr++;
    }
    while(dot(Direction,SampledVector) <= 0.0f || ChosenIndex == -1);
    Direction = mix(Direction,SampledVector,(1.0f - data.Roughness) * data.Metallic * (1.0f - data.Alpha));

    vec3 ShadowColor;
    float isShadow = RayTraceShadows(data.Position,Direction,ShadowColor);
    float NdotL = max(dot(Direction,data.Normal),0.0f);

    return (1.0f - isShadow) * NdotL * PI * Intensity * clamp(texture(EnvironmentCubeMap,Direction).xyz,vec3(0.0f),vec3(1.0f));
}

vec4 TraceRay(in vec3 rayOrigin,in vec3 rayDirection,in int SampleCount,inout vec3 FirstHitNormal,inout vec3 FirstHitAlbedo,inout vec3 FirstHitPosition)
{
    RayData data;
    SampleData Sample;
    Sample.lobe = -1;
    Sample.Fresnel = 1.0f;
    Sample.pdf = 1.0f;
    float ClosestDistance = pos_infinity;
    vec3 InvRayDirection = 1.0f / rayDirection;
    
    vec3 SampleVector = vec3(0.0f);
    
    vec4 Albedo = vec4(vec3(0.0f),1.0f);

    vec3 Origin = rayOrigin;
    vec3 Direction = rayDirection;
    vec3 Throughput = vec3(1.0f);
   
    vec3 AnalyticLightContribution;
    vec3 EmissiveLightContribution;
    bool IsEmissive;

    float lightPdf = 1.0f; 
    float BSDFpdf = 1.0f;
    vec3 ThroughputBRDF;

    RayData FirstHitData; 
    float FirstHitFresnel; 

    bool IsFirstHit;
    int LODlevel = 0;

    bool IsPathMirror = false;
    for(int i = 0;i < SampleCount;i++)
    { 
        IsFirstHit = (i == 0);
        if(!IsFirstHit && data.Alpha >= 1.0f && Sample.lobe == 0 || data.Roughness > 0.6f) LODlevel = clamp(i / 3,0,5);
        else LODlevel = 0;

        data = TraverseBVH(Origin + clamp(Epsilon * distance(CameraPosition,Origin),0.001f, 0.1f) * Direction,Direction,InvRayDirection,ClosestDistance,true,LODlevel);
        if(ClosestDistance == pos_infinity)
        { 
            if(IsFirstHit)
            {
               Albedo.xyz += float(ShouldDisplayTheEnv) * texture(EnvironmentCubeMap,Direction).xyz;
            }
            else
            {
               if(EnvironmentLightIntensity > 0.0f)
               {
                   vec3 EnvSample;
                   //vec3 DiffuseEnvSample = SampleEnvironment(Direction,EnvironmentIntensity,FirstHitData);
                   if(Sample.lobe == 0) EnvSample = EnvironmentLightIntensity * texture(ConvolutedEnvironmentCubeMap,Direction).xyz;
                   else EnvSample = EnvironmentLightIntensity * texture(EnvironmentCubeMap,Direction).xyz;
                   Albedo.xyz += Throughput * EnvSample;
               }
            }
            break;
        }

        if(length(data.Emissive.xyz) > 0.0f)
        {
            if(IsFirstHit) 
            {
                return vec4(data.Emissive.xyz,1.0f); 
            }

            Albedo.xyz += Throughput * EmissiveBoost * (data.Emissive.xyz);  
            break;
        }


        float R0 = pow((1.0 - data.IOR) / (1.0 + data.IOR), 2.0);
        vec3 F0 = mix(vec3(R0), data.Albedo.xyz, data.Metallic);
        vec3 F = FresnelSchlick(max(dot(data.Normal,-Direction), 0.0), F0);
        Sample.Fresnel = Luminance(F);

        if(IsFirstHit)
        {
          CoinFlipBSDFlobe(Sample,data.Albedo.xyz,data.Roughness,data.Metallic,data.Alpha,data.ClearCoat);
          FirstHitNormal = data.Normal;
          FirstHitAlbedo = data.Albedo.xyz;
          FirstHitPosition = data.Position.xyz;
          FirstHitData = data;
          Sample.Eta_i = 1.0f;
          Sample.Eta_t = data.IOR;
          FirstHitFresnel = Sample.Fresnel;
        }

        AnalyticLightContribution = SampleLights(Sample,Direction,data.Normal, data.Albedo.xyz, data.Roughness, data.Metallic, data.Position,data.Alpha,0.0f,0.0f,data.ClearCoat,data.ClearCoatRoughness,data.IOR);
        if(length(AnalyticLightContribution) > 0.0f) Albedo.xyz += Throughput * AnalyticLightContribution;

        
        EmissiveLightContribution = SampleEmissiveObjects(Sample,Direction,data.Position,data.Normal,data.Roughness,data.Metallic,data.Albedo.xyz,data.Alpha,0.0f,0.0f,data.ClearCoat,data.ClearCoatRoughness,data.IOR,lightPdf);
        if(length(EmissiveLightContribution) > 0.0f) Albedo.xyz += Throughput * vec3(EmissiveLightContribution);
        

        if(data.Roughness < 1e-3) IsPathMirror = true;

        SampleBSDF(Sample,Direction,data.Normal,data.Albedo.xyz,data.Position,data.Roughness,data.Metallic,data.Alpha,data.IOR,0.0f,data.ClearCoat);

        
        ThroughputBRDF = Eval(Sample,vec3(1.0f),Direction,Sample.Direction,data.Normal,
                              data.Roughness,data.Metallic,data.Albedo.xyz,data.Alpha,0.0f,0.0f,
                              data.ClearCoat,data.ClearCoatRoughness,data.IOR,true);

        Throughput *= ThroughputBRDF / Sample.pdf;
        Direction = Sample.Direction;

        float p = max(Throughput.x,max(Throughput.y,Throughput.z));
        if(Hash(seed) > p)
        {
          break;
        }
        Throughput *= 1.0f / p; 

        ClosestDistance = pos_infinity;
        Origin = data.Position;
        InvRayDirection = 1.0 / Direction;
    }
    return vec4(Albedo.xyz,1.0f);
}


void main()
{
    ivec2 pos = ivec2( gl_GlobalInvocationID.xy );
    if(pos.x > WindowSize.x || pos.y > WindowSize.y) return;

    seed = uint((pos.x + 1) * (pos.y + 1) * (RandomSeed * RandomSeed + 1));
    seed *= uint(Time * Time + ProgressiveRenderedFrameCount * ProgressiveRenderedFrameCount);
    vec2 uv = (vec2(pos)+vec2(Hash(seed),Hash(seed)))/WindowSize;

    vec2 uvND = uv * 2.0f - 1.0f;

    vec3 rayOrigin = CameraPosition;
    vec4 rayDir4 = inverse(ProjectionViewMat) * vec4(uvND, 1.0, 1.0);
    rayDir4 = vec4(rayDir4.xyz / rayDir4.w,1.0f);
    vec3 rayDirection = normalize(rayDir4.xyz - rayOrigin.xyz);

    vec3 FirstHitNormal;
    vec3 FirstHitAlbedo;
    vec3 FirstHitPosition;
    vec3 result = TraceRay(rayOrigin,rayDirection.xyz,min(BounceCount,max(2,int(0.7f * ProgressiveRenderedFrameCount))),FirstHitNormal,FirstHitAlbedo,FirstHitPosition).xyz;
    
    if(DoFattributes.x > 0.0f && distance(FirstHitPosition,CameraPosition) > DoFattributes.y)
    {
        vec3 Jitter;
        for(int i = 0;i < 3;i++)
        {
           Jitter = DoFattributes.z * (2.0f * vec3(Hash(seed),Hash(seed),Hash(seed)) - 1.0f);
           result += TraceRay(rayOrigin + Jitter,rayDirection.xyz,int(DoFattributes.w),FirstHitNormal,FirstHitAlbedo,FirstHitPosition).xyz;
        }
        result /= 4;    
    }
    
    if(ProgressiveRenderedFrameCount >= 0)
    {
       float weight = 1.0 / float(ProgressiveRenderedFrameCount + 1);

       vec4 PreviousFrame = imageLoad( image, pos ).rgba;
       vec4 PreviousFrameNormal = imageLoad( NormalImage, pos ).rgba;
       vec4 PreviousFrameAlbedo = imageLoad( AlbedoImage, pos ).rgba;

       result = mix(PreviousFrame.xyz,result,weight);
       FirstHitNormal = mix(PreviousFrameNormal.xyz,FirstHitNormal,weight);
       FirstHitAlbedo = mix(PreviousFrameAlbedo.xyz,FirstHitAlbedo,weight);
    }

    imageStore( image, pos,vec4(result,1.0f));
    imageStore( NormalImage, pos,vec4(FirstHitNormal,1.0f));
    imageStore( AlbedoImage, pos,vec4(FirstHitAlbedo,1.0f));
}