#version 460 core
#extension GL_ARB_gpu_shader_int64 : enable

#define VNDF_SAMPLING

layout(local_size_x=32,local_size_y=32,local_size_z=1) in;
layout(rgba32f,binding=0) uniform image2D image;

//BVH data
layout (location = 1) uniform samplerBuffer MinBounds;
layout (location = 2) uniform samplerBuffer MaxBounds;
layout (location = 3) uniform samplerBuffer ChildIndicies;
layout (location = 4) uniform samplerBuffer TriangleIndicies;
layout (location = 5) uniform samplerBuffer TriangleCounts;

//Utility uniforms
layout (location = 6) uniform vec2 WindowSize;
layout (location = 7) uniform vec3 CameraPosition;
layout (location = 8) uniform mat4 ProjectionViewMat;
layout (location = 9) uniform float Time;

//Material data
layout (location = 10) uniform samplerBuffer ModelAlbedos;
layout (location = 11) uniform samplerBuffer ModelRoughness;
layout (location = 12) uniform samplerBuffer ModelMetallic;
layout (location = 13) uniform samplerBuffer ModelAlphas;
layout (location = 14) uniform samplerBuffer ModelEmissives;

readonly layout(std430,binding=9) restrict buffer ModelTextureHandles
{
   uint64_t TexturesHandles[];
};

//Geometry data
layout (location = 15) uniform samplerBuffer TriangleUVS;
layout (location = 16) uniform samplerBuffer TriangleNormals;
layout (location = 17) uniform samplerBuffer TrianglePositions;
layout (location = 18) uniform samplerBuffer TriangleTangentsBitangents;

layout (location = 19) uniform int ModelCount;
layout (location = 20) uniform samplerCube EnvironmentCubeMap;
layout (location = 21) uniform int ProgressiveRenderedFrameCount;

layout (location = 22) uniform int LightCount;
layout (location = 23) uniform float CameraPlaneDistance;
layout (location = 24) uniform float RandomSeed;

layout (location = 25) uniform samplerBuffer EmissiveObjects;
layout (location = 26) uniform float TotalLightIntensity;

readonly layout(std430,binding=8) restrict buffer ModelMatricesData
{
  mat4 ModelMatrices[]; 
};

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

layout(std430 , binding = 4) restrict buffer LightsDatas
{
    Light Lights[];
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
};

const float InvLightCount = 1.0f / max(1.0f,float(LightCount));
uint seed;
int NodesToProcess[27];

float Hash(inout int x)
{
	x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return float(x & 0xFFFFFF) / float(0x1000000); 
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


RayData TraverseBVH(in vec3 rayOrigin,in vec3 rayDirection,in vec3 InvRayDirection,inout float ClosestDistance,in bool SampleMaterial)
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

     while(StackIndex > 0)
     {
        StackIndex--;
        CurrentIndex = NodesToProcess[StackIndex];
        
        ChildIndex = int(texelFetch(ChildIndicies,CurrentIndex).r);
        if(ChildIndex == -1)
        {
            TriangleIndex = int(texelFetch(TriangleIndicies,CurrentIndex).r);
	        TriCount = int(texelFetch(TriangleCounts,CurrentIndex).r);
            for(int i = TriangleIndex;i < TriangleIndex + TriCount;i++)
            {
                v0 = texelFetch(TrianglePositions,i * 3).xyzw;
                v1 = texelFetch(TrianglePositions,i * 3 + 1).xyz;
                v2 = texelFetch(TrianglePositions,i * 3 + 2).xyz;

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
                            n0 = texelFetch(TriangleNormals,iTrip).xyz;
                            n1 = texelFetch(TriangleNormals,iTrip + 1).xyz;
                            n2 = texelFetch(TriangleNormals,iTrip + 2).xyz;
                            Normal = normalize(OneMinusUV * n0 + TriangleUV.x * n1 + TriangleUV.y * n2);

                            TextureHandle = TexturesHandles[CurrentModelID * 6 + 1];
                            if (TextureHandle != 0) 
                            {
                                iTrip *= 2;

                                n0 = texelFetch(TriangleTangentsBitangents,iTrip).xyz;
                                n1 = texelFetch(TriangleTangentsBitangents,iTrip + 2).xyz;
                                n2 = texelFetch(TriangleTangentsBitangents,iTrip + 4).xyz;
                                Tangent = normalize(OneMinusUV * n0 + TriangleUV.x * n1 + TriangleUV.y * n2);

                                n0 = texelFetch(TriangleTangentsBitangents,iTrip + 1).xyz;
                                n1 = texelFetch(TriangleTangentsBitangents,iTrip + 3).xyz;
                                n2 = texelFetch(TriangleTangentsBitangents,iTrip + 5).xyz;
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

                            TextureHandle = TexturesHandles[CurrentModelID * 6 + 2];
                            if (TextureHandle != 0) {
                                data.Roughness = texture(sampler2D(TextureHandle),TriangleTextureUV).x;
                            }
                            else
                            {
                                data.Roughness = texelFetch(ModelRoughness,CurrentModelID).x;
                            } 

                            TextureHandle = TexturesHandles[CurrentModelID * 6 + 3];
                            if (TextureHandle != 0) {
                                data.Metallic = texture(sampler2D(TextureHandle),TriangleTextureUV).x;
                            }
                            else
                            {
                                data.Metallic = texelFetch(ModelMetallic,CurrentModelID).x;
                            }
                          

                            data.uv = TriangleTextureUV;
                        }  
                        TextureHandle = TexturesHandles[CurrentModelID * 6];
                        if (TextureHandle != 0) {
                            data.Albedo = vec4(texture(sampler2D(TextureHandle),TriangleTextureUV).xyz,1.0f);
                        }
                        else
                        {
                            data.Albedo = texelFetch(ModelAlbedos,CurrentModelID);
                        } 

                        data.Position = rayOrigin + rayDirection * result.t;
                        TextureHandle = TexturesHandles[CurrentModelID * 6 + 4];
                        if (TextureHandle != 0) {
                            data.Alpha = texture(sampler2D(TextureHandle),TriangleTextureUV).x;
                        }
                        else
                        {
                            data.Alpha = texelFetch(ModelAlphas,CurrentModelID).x;
                        }

                        TextureHandle = TexturesHandles[CurrentModelID * 6 + 5];
                        if (TextureHandle != 0) {
                            data.Emissive.xyzw = texture(sampler2D(TextureHandle),TriangleTextureUV).xyzw;
                        }
                        else
                        {
                            data.Emissive.xyzw = texelFetch(ModelEmissives,CurrentModelID).xyzw;
                        }
                        ClosestDistance = result.t;
                    }  
                }
            }
        }
        else
        {
               DistanceChild0 = RayAABBIntersection(rayOrigin,InvRayDirection,texelFetch(MinBounds,ChildIndex).xyz,texelFetch(MaxBounds,ChildIndex).xyz);
               DistanceChild1 = RayAABBIntersection(rayOrigin,InvRayDirection,texelFetch(MinBounds,ChildIndex + 1).xyz,texelFetch(MaxBounds,ChildIndex + 1).xyz);
           
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

float DistributionGGX(vec3 N , vec3 H, float roughness)
{
    roughness = clamp(roughness,0.09f,1.0f);
    float a = roughness * roughness;
    float a2 = a*a;
    float NdotH = max(dot(N,H),0.0f);
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

float RayTraceShadows(in vec3 CurrentPosition,in vec3 LightDirection,in float LightDiameter)
{
     vec3 SampleLightDirection = vec3(0.0f);
     float Shadow = 0.0f;
     float ClosestDistance = 0.0f;
     RayData data;
     float HitAlpha = 0.0f;
     vec3 Position = CurrentPosition;
     vec3 InvRayDirection;
     int iterator = 0;
     while(iterator < 3 && HitAlpha < 1.0f && Shadow < 1.0f)
     {
         ClosestDistance = pos_infinity;
         SampleLightDirection = mix(LightDirection,SampleSemiSphere(vec2(Hash(seed),Hash(seed)),LightDirection),LightDiameter);
         InvRayDirection = 1.0f / (SampleLightDirection);
         data = TraverseBVH(Position + Epsilon * distance(CameraPosition,Position) * SampleLightDirection , SampleLightDirection,InvRayDirection,ClosestDistance,false);
         HitAlpha = data.Alpha;
         if(ClosestDistance != pos_infinity)
         {
            Shadow += HitAlpha;
            Position = data.Position;
         }
         else break;
         iterator++;
     }
   
     return min(1.0f,Shadow);
}

vec3 RayTracePositionalShadows(in vec3 CurrentPosition,in vec3 LightDirection,in vec3 LightPosition,inout vec4 HitEmissive)
{
     vec3 SampleLightDirection = vec3(0.0f);
     vec3 Shadow = vec3(0.0f);
     float ClosestDistance = 0.0f;
     RayData data;
     float HitAlpha = 1.0f;
     vec3 Position = CurrentPosition;
     vec3 InvRayDirection;
     float DistancePositions;
     int iterator = 0;
     do
     {
         ClosestDistance = pos_infinity;
         SampleLightDirection = LightDirection;
         InvRayDirection = 1.0f / (SampleLightDirection);
         data = TraverseBVH(Position + Epsilon * SampleLightDirection , SampleLightDirection,InvRayDirection,ClosestDistance,false);
         if(ClosestDistance != pos_infinity)
         {
            if(length(data.Emissive.xyz) > 0.0f) 
            {
               HitEmissive = data.Emissive;
               return min(vec3(1.0f),Shadow);
            }
            if(distance(CurrentPosition,LightPosition) <= distance(CurrentPosition,data.Position)) return vec3(0.0f);
            
            Shadow += data.Alpha; 
            HitAlpha = data.Alpha;
            Position = data.Position;
         }
         else break;
         iterator++;
     }
     while(iterator < 3 && HitAlpha < 1.0f && length(Shadow) < 1.0f);
   
     return min(vec3(1.0f),Shadow);
}

vec3 CookTorranceBRDFspec(vec3 Normal, vec3 ViewDirection, vec3 LightDirection,float Roughness,float Metallic,vec3 Albedo)
{
    vec3 F0 = mix(vec3(0.04), Albedo, Metallic);
    vec3 H = normalize(ViewDirection + LightDirection);
    vec3 F = fresnelSchlickRoughness(max(dot(Normal, H), 0.0), F0, Roughness);
    float NDF = DistributionGGX(Normal, H, Roughness);
    float G = GeometrySmith(Normal, ViewDirection, LightDirection, Roughness);

    float NdotL = max(dot(Normal, LightDirection), 0.0);
    float NdotV = max(dot(Normal, ViewDirection), 0.0);
    return (NDF * G * F) / max(4.0 * NdotL * NdotV, 1e-4);
}

vec3 CookTorranceBRDF(vec3 Normal, vec3 ViewDirection, vec3 LightDirection,vec3 LightColor,float Roughness,float Metallic,vec3 Albedo)
{
    vec3 F0 = mix(vec3(0.04),Albedo, Metallic);
    vec3 H = normalize(ViewDirection + LightDirection);

    vec3 F = fresnelSchlickRoughness(max(dot(Normal, H), 0.0), F0, Roughness);
    float NDF = DistributionGGX(Normal, H, Roughness);
    float G = GeometrySmith(Normal, ViewDirection, LightDirection, Roughness);

    float NdotL = max(dot(Normal, LightDirection), 0.0);
    float NdotV = max(dot(Normal, ViewDirection), 0.0);
    vec3 kD = (1.0 - F) * (1.0 - Metallic);

    vec3 specular = (NDF * G * F) / max(4.0 * NdotL * NdotV, 1e-4);
    return (kD * Albedo * Inv_PI + specular) * LightColor;
}

vec3 MicroFacetBTDF(vec3 Normal, vec3 IncidentDirection,vec3 OutgoingDirection,float eta_i, float eta_t,float Roughness,float Metallic,vec3 Albedo)
{
    vec3 F0 = mix(vec3(0.04),Albedo, Metallic);
    vec3 H = normalize(eta_i * IncidentDirection + eta_t * OutgoingDirection);

    vec3 F = fresnelSchlickRoughness(max(dot(Normal, H), 0.0), F0, Roughness);
    float NDF = DistributionGGX(Normal, H, Roughness);
    float G = GeometrySmith(Normal, IncidentDirection, OutgoingDirection, Roughness);

    float NdotL = abs(dot(Normal, IncidentDirection));
    float NdotV = abs(dot(Normal, OutgoingDirection));
    float dotHL = dot(H, IncidentDirection);
    float dotHV = dot(H, OutgoingDirection);
    float HdotL = abs(dotHL);
    float HdotV = abs(dotHV);

    float EnergyScale = (eta_t * eta_t) / (eta_i * eta_i);
    float c = (HdotL * HdotV) / (NdotL * NdotV);
    float t = (EnergyScale / pow(dotHL + EnergyScale * dotHV,2));

    return t * (NDF * G * (1.0f - F)) * c;
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

float Luminance(vec3 c)
{
    return 0.212671 * c.x + 0.715160 * c.y + 0.072169 * c.z;
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
    float HdotL = dot(IncidentDirection,OutgoingDirection);
    vec3 tint = Tint(Albedo);
    return Sheen * mix(vec3(1.0f),tint,SheenTint) * SchlickWeight(HdotL);
}

void SampleBSDF(inout SampleData Sample,in vec3 Direction,in vec3 Normal,in vec3 Albedo,in vec3 Position,in float Roughness,in float Metallic,in float Alpha,in float Anisotropy)
{
    float pdf;
    vec3 SampleVector; 
    float AveragedFresnel = Sample.Fresnel;
    vec3 H = vec3(0.0f);
    
    float CoinFlip = Hash(seed);
   
    float dielectricWt = (1.0 - Metallic) * Alpha;
    float metalWt = Metallic;
    float glassWt = (1.0 - Metallic) * (1.0f - Alpha);

    float diffPr = dielectricWt * Luminance(Albedo);
    float dielectricPr = dielectricWt * AveragedFresnel;
    float metalPr = metalWt * Luminance(mix(Albedo, vec3(1.0), AveragedFresnel));
    float glassPr = glassWt;
    float clearCtPr = 0.25 * 0.0f;

    float invTotalWt = 1.0 / (diffPr + dielectricPr + metalPr + glassPr + clearCtPr);
    diffPr *= invTotalWt;
    dielectricPr *= invTotalWt;
    metalPr *= invTotalWt;
    glassPr *= invTotalWt;
    clearCtPr *= invTotalWt;

    float cdf[5];
    cdf[0] = diffPr;
    cdf[1] = cdf[0] + dielectricPr;
    cdf[2] = cdf[1] + metalPr;
    cdf[3] = cdf[2] + glassPr;
    cdf[4] = cdf[3] + clearCtPr;

    if(CoinFlip < cdf[0])
    {
        Sample.lobe = 0;
        vec3 LambertianSample = SampleSemiSphere(vec2(Hash(seed),Hash(seed)),Normal);
        SampleVector = LambertianSample; 

        float pdfDiffuse = max(dot(Normal, LambertianSample),0.0f) / PI;
        pdf = pdfDiffuse * cdf[0] * (1.0f - AveragedFresnel);
    }
    else if(CoinFlip < cdf[2] || CoinFlip < cdf[1])
    {
        if(CoinFlip < cdf[2])
        {
           Sample.lobe = 2;
           pdf = cdf[2];   
        }
        else
        {
           Sample.lobe = 1;
           pdf = cdf[1];   
        }
        
        if(Roughness < 1e-4)
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
            pdf *= pdfGGX;
#endif
        }
    }
    else if(CoinFlip < cdf[3])
    {    
        if(Roughness < 1e-4)
        {
            H = Normal;
        }
        else
        {
            if(length(H) <= 0.0f)
            {
                float a = max(Roughness * Roughness,1e-4);
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
        }

        float VdotH = dot(Direction,H);
        bool IsReflect = VdotH <= 0.0f;
        float Eta_i = IsReflect ? 1.5f : 1.0f;
        float Eta_t = IsReflect ? 1.0f : 1.5f;
        float RelativeIOR = Eta_t/Eta_i;

        vec3 refracted = refract(Direction,IsReflect ? H : -H,RelativeIOR);
        Sample.Eta_i = Eta_i;
        Sample.Eta_t = Eta_t;

        float FresnelCoinShot = Hash(seed);
        if (length(refracted) == 0.0 || FresnelCoinShot < AveragedFresnel) 
        {
            SampleVector = reflect(Direction,H);
            float NDF = DistributionGGX(Normal, H, Roughness);
            float pdfGGX = (NDF * max(dot(Normal, H), 0.0f)) / max(4.0f * max(dot(H, -Direction), 0.0f),0.0001); 
            pdf = pdfGGX * cdf[2] * AveragedFresnel;
            Sample.lobe = 2;
        }
        else
        {
            SampleVector = refracted;
            Sample.lobe = 3;
            float LdotH = abs(dot(SampleVector, H));
            float jacobian = LdotH / (pow(LdotH + RelativeIOR * VdotH,2));
            pdf = (1.0f - AveragedFresnel) * cdf[3] / jacobian;
        }
    }
    
    pdf = max(pdf, 1e-4);
    Sample.Direction = normalize(SampleVector);
    Sample.pdf = pdf;
}

void CoinFlipBSDFlobe(inout SampleData Sample,in vec3 Albedo,in float Roughness,in float Metallic,in float Alpha)
{
    float AveragedFresnel = Sample.Fresnel;

    float CoinFlip = Hash(seed);
   
    float dielectricWt = (1.0 - Metallic) * Alpha;
    float metalWt = Metallic;
    float glassWt = (1.0 - Metallic) * (1.0f - Alpha);

    float diffPr = dielectricWt * Roughness * Luminance(Albedo);
    float dielectricPr = dielectricWt * (1.0f - Roughness) * AveragedFresnel;
    float metalPr = metalWt * Luminance(mix(Albedo, vec3(1.0), AveragedFresnel));
    float glassPr = glassWt;
    float clearCtPr = 0.25 * 0.0f;

    float invTotalWt = 1.0 / (diffPr + dielectricPr + metalPr + glassPr + clearCtPr);
    diffPr *= invTotalWt;
    dielectricPr *= invTotalWt;
    metalPr *= invTotalWt;
    glassPr *= invTotalWt;
    clearCtPr *= invTotalWt;

    float cdf[5];
    cdf[0] = diffPr;
    cdf[1] = cdf[0] + dielectricPr; 
    cdf[2] = cdf[1] + metalPr;
    cdf[3] = cdf[2] + glassPr;
    cdf[4] = cdf[3] + clearCtPr;

    if(CoinFlip < cdf[0]) Sample.lobe = 0;
    else if(CoinFlip < cdf[1]) Sample.lobe = 1;
    else if(CoinFlip < cdf[2]) Sample.lobe = 2;
    else if(CoinFlip < cdf[3]) Sample.lobe = 3;
    //else Sample.lobe = 4;
}

vec3 Eval(in SampleData Sample,in vec3 Radiance,in vec3 Direction,in vec3 LightDirection,in vec3 Normal,in float Roughness,in float Metallic,in vec3 Albedo,in float Alpha)
{
    if (Sample.lobe == 0)
    {
        return Radiance * (Albedo * Inv_PI + EvalSheen(Sample,-Direction,LightDirection,0.4f,0.2f,Albedo));
    }
    else if (Sample.lobe == 1) 
    {
        return CookTorranceBRDF(Normal, -Direction, LightDirection, Radiance, Roughness, Metallic, Albedo);
    }
    else if (Sample.lobe == 2)
    {   
        return CookTorranceBRDFspec(Normal, -Direction, LightDirection, Roughness, Metallic, Albedo) * Radiance;  
    }
    else
    {   
        return Radiance * Albedo * MicroFacetBTDF(Normal, -Direction,LightDirection, Sample.Eta_i, Sample.Eta_t, Roughness, Metallic, Albedo);  
    }
    
    
    /*
    if (Sample.lobe == 0)
    {
        return vec3(1.0f,0.0f,0.0f);
    }
    else if(Sample.lobe == 1)
    {
        return vec3(1.0f,1.0f,1.0f);
    }
    else if (Sample.lobe == 2)
    {   
        return vec3(0.0f,1.0f,0.0f);  
    }
    else if (Sample.lobe == 3)
    {   
        return vec3(1.0f,0.0f,1.0f); 
    }
    else
    {
        return vec3(0.0f);
    }
   */
}

vec3 SampleLights(in SampleData Sample,vec3 Direction,vec3 N, vec3 Albedo, float Roughness, float Metallic,vec3 Position,in float Alpha) {
    if(LightCount <= 0) return vec3(0.0f);
    float isShadow = 0.0f;
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
    
    if(IsPointLight) isShadow = Luminance(RayTracePositionalShadows(Position,L,CurrentLight.Position.xyz,Emissive));
    else if(IsDirectionalLight) isShadow = RayTraceShadows(Position,L,Epsilon * abs(ndotl));
    
    if(isShadow >= 1.0f) return vec3(0.0f,0.f,0.0f);
    return NdotL * (1.0f - isShadow) * Eval(Sample,Radiance,Direction,L,N,Roughness,Metallic,Albedo,Alpha);
}

vec3 SampleEmissiveObjects(in SampleData Sample,in vec3 Direction,in vec3 Position,in vec3 Normal,in float Roughness,in float Metallic,in vec3 Albedo,in float Alpha)
{
     int EmissiveTriangleCount = textureSize(EmissiveObjects);
     if(EmissiveTriangleCount <= 0) return vec3(0.0f);
     float RandomValue = Hash(seed);
     int SampledIndex = int(texelFetch(EmissiveObjects,int(RandomValue * EmissiveTriangleCount)).x) * 3;

     vec2 uv = vec2(RandomValue,Hash(seed));

     vec4 v0 = texelFetch(TrianglePositions,SampledIndex).xyzw;
     vec3 v1 = texelFetch(TrianglePositions,SampledIndex + 1).xyz;
     vec3 v2 = texelFetch(TrianglePositions,SampledIndex + 2).xyz;
     vec3 SampledPosition = (1.0f - uv.x - uv.y) * v0.xyz + uv.x * v1 + uv.y * v2;
     
     vec3 L = normalize(SampledPosition - Position);

     float NdotL = max(dot(L,Normal),0.0f);
     if(NdotL <= 0.0f) return vec3(0.0f);

     float Distance = length(L);

     vec4 Emissive = vec4(0.0f);
     vec3 Shadow = RayTracePositionalShadows(Position,L,SampledPosition,Emissive);
     float IsVisible = Luminance(Shadow);
     if((1.0f - IsVisible) <= 0.0f) return vec3(0.0f);

     vec3 n0 = texelFetch(TriangleNormals,SampledIndex).xyz;
     vec3 n1 = texelFetch(TriangleNormals,SampledIndex + 1).xyz;
     vec3 n2 = texelFetch(TriangleNormals,SampledIndex + 2).xyz;
     vec3 LightNormal = normalize((1.0f - uv.x - uv.y) * n0 + uv.x * n1 + uv.y * n2);
     
     float LdotLnormal = max(0.0f,dot(-L,LightNormal));
     IsVisible = LdotLnormal * NdotL * (1.0f - IsVisible);
     if(IsVisible <= 0.0f) return vec3(0.0f);

     float Attenuation = 1.0 / (Distance * Distance);
     vec3 Radiance = 10.4f * Emissive.xyz * Attenuation;

     return IsVisible * Eval(Sample,Radiance,Direction,L,Normal,Roughness,Metallic,Albedo,Alpha);
}

vec4 TraceRay(in vec3 rayOrigin,in vec3 rayDirection,in int SampleCount,inout vec3 FirstHitPosition)
{
    RayData data;
    SampleData Sample;
    Sample.lobe = -1;
    Sample.Fresnel = 1.0f;
    float ClosestDistance = pos_infinity;
    vec3 InvRayDirection = 1.0f / rayDirection;
    
    vec3 SampleVector = vec3(0.0f);
    
    vec4 Albedo = vec4(vec3(0.0f),1.0f);

    vec3 Origin = rayOrigin;
    vec3 Direction = rayDirection;
    float EnvironmentIntensity = 1.0f;
    vec3 Throughput = vec3(1.0f);
   
    vec3 AnalyticLightContribution;
    vec3 EmissiveLightContribution;
    bool IsEmissive;

    float lightPdf = 1.0f; 
    float BSDFpdf = 1.0f;
    vec3 ThroughputBRDF;

    bool IsFirstHit;
    for(int i = 0;i < SampleCount;i++)
    { 
        IsFirstHit = (i == 0);
        data = TraverseBVH(Origin + clamp(Epsilon * distance(CameraPosition,Origin),0.001f, 0.1f) * Direction,Direction,InvRayDirection,ClosestDistance,true);
        if(ClosestDistance == pos_infinity)
        {
            Albedo.xyz += Throughput * (IsFirstHit ? 0.0f : EnvironmentIntensity) * texture(EnvironmentCubeMap,Direction).xyz;
            //Albedo.xyz += Throughput * (i == 0 ? 1.0f : EnvironmentIntensity) * vec3(0.8f);
            break;
        }

        if(length(data.Emissive.xyz) > 0.0f)
        {
            if(IsFirstHit) 
            {
                return vec4(data.Emissive.xyz,1.0f); 
            }

            Albedo.xyz += Throughput * (data.Emissive.xyz);  
            break;
        }

        vec3 F0 = mix(vec3(0.04), data.Albedo.xyz, data.Metallic);
        vec3 F = FresnelSchlick(max(dot(data.Normal,-Direction), 0.0), F0);
        Sample.Fresnel = Luminance(F);

        if(IsFirstHit)
        {
          CoinFlipBSDFlobe(Sample,data.Albedo.xyz,data.Roughness,data.Metallic,data.Alpha);
          FirstHitPosition = data.Position;
        }
        
        AnalyticLightContribution = SampleLights(Sample,Direction,data.Normal, data.Albedo.xyz, data.Roughness, data.Metallic, data.Position,data.Alpha);
        if(length(AnalyticLightContribution) > 0.0f) Albedo.xyz += Throughput * AnalyticLightContribution;
   
        EmissiveLightContribution = SampleEmissiveObjects(Sample,Direction,data.Position,data.Normal,data.Roughness,data.Metallic,data.Albedo.xyz,data.Alpha);
        if(length(EmissiveLightContribution) > 0.0f) Albedo.xyz += Throughput * EmissiveLightContribution;

        SampleBSDF(Sample,Direction,data.Normal,data.Albedo.xyz,data.Position,data.Roughness,data.Metallic,data.Alpha,0.0f);
        BSDFpdf = Sample.pdf;

        ThroughputBRDF = Eval(Sample,vec3(1.0f),Direction,Sample.Direction,data.Normal,data.Roughness,data.Metallic,data.Albedo.xyz,data.Alpha);

        Throughput *= mix(data.Albedo.xyz,ThroughputBRDF,data.Alpha) / BSDFpdf;
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
        lightPdf = 1.0f; 
    }
    return vec4(Albedo.xyz,1.0f);
}

void main()
{
    ivec2 pos = ivec2( gl_GlobalInvocationID.xy );
    seed = uint((pos.x + 1) * (pos.y + 1) * (RandomSeed * RandomSeed + 1));
    seed *= uint(Time * Time + ProgressiveRenderedFrameCount * ProgressiveRenderedFrameCount);
    vec2 uv = (vec2(pos)+vec2(Hash(seed),Hash(seed)))/WindowSize;

    vec2 uvND = uv * 2.0f - 1.0f;

    vec3 rayOrigin = CameraPosition;
    vec4 rayDir4 = inverse(ProjectionViewMat) * vec4(uvND, 1.0, 1.0);
    rayDir4 = vec4(rayDir4.xyz / rayDir4.w,1.0f);
    vec3 rayDirection = normalize(rayDir4.xyz - rayOrigin.xyz);

    vec3 FirstHitPosition;
    vec3 result = TraceRay(rayOrigin,rayDirection.xyz,min(10,max(2,int(0.7f * ProgressiveRenderedFrameCount))),FirstHitPosition).xyz;
    
    /*
    if(distance(FirstHitPosition,CameraPosition) > 5.0f)
    {
        vec3 Jitter;
        for(int i = 0;i < 3;i++)
        {
           Jitter = 0.01f * (2.0f * vec3(Hash(seed),Hash(seed),Hash(seed)) - 1.0f);
           result += TraceRay(rayOrigin + Jitter,rayDirection.xyz,6,FirstHitPosition).xyz;
        }
        result /= 4;    
    }
    */
    
    if(ProgressiveRenderedFrameCount >= 0)
    {
       float weight = 1.0 / float(ProgressiveRenderedFrameCount + 1);
       vec4 PreviousFrame = imageLoad( image, pos ).rgba;
       result = mix(PreviousFrame.xyz,result,weight);
    }

    imageStore( image, pos,vec4(result,1.0f));
}