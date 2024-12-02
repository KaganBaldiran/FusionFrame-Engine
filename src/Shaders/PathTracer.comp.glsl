#version 460 core
#extension GL_ARB_gpu_shader_int64 : enable

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
const float Epsilon = 0.0001f;

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

                            TextureHandle = TexturesHandles[CurrentModelID * 6];
                            if (TextureHandle != 0) {
                                data.Albedo = vec4(texture(sampler2D(TextureHandle),TriangleTextureUV).xyz,1.0f);
                            }
                            else
                            {
                                data.Albedo = texelFetch(ModelAlbedos,CurrentModelID);
                            } 
                        
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
                                //Normal = vec3(Normal.x,-Normal.y,Normal.z);
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
                          
                            TextureHandle = TexturesHandles[CurrentModelID * 6 + 5];
                            if (TextureHandle != 0) {
                                data.Emissive.xyzw = texture(sampler2D(TextureHandle),TriangleTextureUV).xyzw;
                            }
                            else
                            {
                                data.Emissive.xyzw = texelFetch(ModelEmissives,CurrentModelID).xyzw;
                            }
                            
                            /*
                            data.Light = vec3(0.0f);
                            for(int y = 0;y < LightCount;y++)
                            {
                                data.Light += max(0.0f,dot(normalize(Lights[y].Position.xyz),Normal)) * Lights[y].Color.xyz * Lights[y].Intensity;
                            }
                            */

                            data.uv = TriangleTextureUV;
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
     while(HitAlpha < 1.0f && Shadow < 1.0f)
     {
         ClosestDistance = pos_infinity;
         SampleLightDirection = mix(LightDirection,SampleSemiSphere(vec2(Hash(seed),Hash(seed)),LightDirection),LightDiameter);
         InvRayDirection = 1.0f / (SampleLightDirection);
         data = TraverseBVH(Position + Epsilon * distance(CameraPosition,Position) * SampleLightDirection , SampleLightDirection,InvRayDirection,ClosestDistance,false);
         HitAlpha = data.Alpha;
         if(ClosestDistance != pos_infinity)
         {
            Shadow += HitAlpha;
         }
         Position = data.Position;
     }
   
     return min(1.0f,Shadow);
}

vec3 CookTorranceBRDFspec(vec3 Normal, vec3 ViewDirection, vec3 LightDirection,float Roughness,vec3 F0)
{
    vec3 H = normalize(ViewDirection + LightDirection);

    vec3 F = fresnelSchlickRoughness(max(dot(Normal, H), 0.0), F0, Roughness);
    float NDF = DistributionGGX(Normal, H, Roughness);
    float G = GeometrySmith(Normal, ViewDirection, LightDirection, Roughness);

    float NdotL = max(dot(Normal, LightDirection), 0.0);
    float NdotV = max(dot(Normal, ViewDirection), 0.0);
    return (NDF * G * F) / (4.0 * NdotV * NdotL + 0.0001);
}

vec3 CookTorranceBRDF(vec3 Normal, vec3 ViewDirection, vec3 LightDirection,vec3 LightColor,float Roughness,float Metallic,vec3 Albedo)
{
    vec3 F0 = mix(vec3(0.04), Albedo, Metallic);
    vec3 H = normalize(ViewDirection + LightDirection);

    vec3 F = fresnelSchlickRoughness(max(dot(Normal, H), 0.0), F0, Roughness);
    float NDF = DistributionGGX(Normal, H, Roughness);
    float G = GeometrySmith(Normal, ViewDirection, LightDirection, Roughness);

    float NdotL = max(dot(Normal, LightDirection), 0.0);
    float NdotV = max(dot(Normal, ViewDirection), 0.0);
    vec3 kD = (1.0 - F) * (1.0 - Metallic);

    vec3 specular = (NDF * G * F) / (4.0 * NdotV * NdotL + 0.0001);
  
    return (kD * Albedo * Inv_PI + specular) * LightColor;
}

float PowerHeuristic(float pdf0,float pdf1)
{
    float pdf02 = pow(pdf0,2);
    return pdf02 / (pdf02 + pow(pdf1,2));
}

float BSDFpdf(vec3 N,vec3 H,vec3 D,vec3 V,float Roughness,bool IsGGX)
{
   float NDF = DistributionGGX(N, H, Roughness);
   float pdfGGX = (NDF * max(dot(N, H), 0.0f)) / (4.0f * max(dot(normalize(V + H), H), 0.0) + 0.0001); 
   float pdfDiffuse = max(dot(N, D), 0.0) * Inv_PI;
   //float pdfDiffuse = 1.0f / (2.0f * PI);

   return IsGGX ? pdfGGX : pdfDiffuse;
}

vec3 EvaluateBRDF(vec3 N, vec3 Albedo, float roughness, float Metalic, vec3 CameraPos, vec3 Position,inout vec3 brdf) {
    vec3 V = normalize(CameraPos - Position);
    vec3 F0 = mix(vec3(0.04), Albedo, Metalic);
    float NdotV = max(dot(N, V), 0.0);
    
    float pdfDiffuse = 1.0f;
    float pdfGGX = 1.0f;

    vec3 Lo = vec3(0.0f);
    float isShadow = 0.0f;
    for(int i=0;i < LightCount;i++)
    {
        vec3 L = normalize(Lights[i].Position.xyz);
        float ndotl = dot(N, L);
        float NdotL = max(ndotl, 0.0);
        vec3 H = normalize(V + L);
        vec3 F = fresnelSchlickRoughness(max(dot(N, H), 0.0), F0, roughness);

        float NDF = DistributionGGX(N, H, roughness);

        //pdfGGX += (NDF * max(dot(N, H), 0.0)) / (4.0f * max(dot(L, H), 0.0) + 0.0001); 
        //pdfDiffuse += NdotL * Inv_PI;
        float G = GeometrySmith(N, V, L, roughness);
        //brdf += G;
        isShadow = RayTraceShadows(Position,L,Epsilon * abs(ndotl));
        //if(isShadow >= 1.0f) continue;


        vec3 kS = F;
        vec3 kD = (1.0 - kS) * (1.0 - Metalic);

        vec3 specular = (NDF * G * F) / (4.0 * NdotV * NdotL + 0.0001);
        vec3 radiance = Lights[i].Color.xyz;
        Lo += (1.0f - isShadow) * (kD * Albedo * Inv_PI + specular) * radiance * Lights[i].Intensity * NdotL;
        brdf += specular * Lights[i].Intensity * NdotL;
    }
    /*
    pdfGGX *= max(1.0f,InvLightCount);
    pdfDiffuse *= max(1.0f,InvLightCount);
    vec3 AdjustedRoughness = mix(vec3(mix(roughness,0.0f,Metalic)),vec3(0.0f),F);
    pdf = mix(pdfGGX,pdfDiffuse,AdjustedRoughness.x);
    */
    return Lo;
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

vec3 SampleBSDF(in vec3 Direction,in vec3 Normal,in vec3 Albedo,in vec3 Position,in float Roughness,in float Metallic,in float Alpha,in float F0,in vec3 H,in vec3 F,inout float pdf)
{
    float AdjustedRoughness = mix(Roughness,0.0f,Metallic);
    vec3 Reflection = reflect(Direction,Normal);
    vec3 GGXsample = SampleGGX(vec2(Hash(seed),Hash(seed)),Reflection,Roughness);
    vec3 LambertianSample = SampleSemiSphere(vec2(Hash(seed),Hash(seed)),Normal);
    //brdf = CookTorranceBRDFspec(Normal, normalize(CameraPosition - Position), -Direction,Roughness,mix(vec3(0.04), Albedo, Metallic));
    //brdf = CookTorranceBRDF(Normal,normalize(CameraPosition - Position),-Direction,vec3(1.0f),Roughness,Metallic,Albedo.xyz);

    //vec3 F_res = F + (1.0 - F) * pow(1.0 - dot(Normal, -Direction), 5.0);
    bool IsGGX = Hash(seed) > AdjustedRoughness;
    //pdf = BSDFpdf(Normal,GGXsample,LambertianSample,Direction,AdjustedRoughness,IsGGX);
    vec3 SampleVector = IsGGX ? GGXsample : LambertianSample;

    //vec3 SampleVector = mix(LambertianSample,GGXsample,F_res);

    if(Alpha < 1.0f)
    {
      //vec3 F0 = mix(vec3(0.04), Albedo, Metallic);
      //vec3 F = fresnelSchlickRoughness(max(dot(Normal, Direction), 0.0), F0, Roughness);
      vec3 Refraction = refract(normalize(Direction),Normal, 0.67);
      vec3 GGXrefractionSample = SampleGGX(vec2(Hash(seed),Hash(seed)),Direction,Roughness);
      SampleVector = Hash(seed) > Alpha ? Direction : SampleVector;
    }
    SampleVector = normalize(SampleVector);
    return SampleVector;
}

vec4 TraceRay(in vec3 rayOrigin,in vec3 rayDirection,in int SampleCount)
{
    RayData data;
    float ClosestDistance = pos_infinity;
    vec3 InvRayDirection = 1.0f / rayDirection;
    
    vec3 brdf = vec3(0.0f);
    float pdf = 1.0f;
    vec3 F;
    
    /*
    data = TraverseBVH(rayOrigin + Epsilon * distance(CameraPosition,rayOrigin) * rayDirection,rayDirection,InvRayDirection,ClosestDistance,true);
    if(ClosestDistance == pos_infinity)
    {
        return vec4(texture(EnvironmentCubeMap,rayDirection).xyz,1.0f);
    }
    float cosTheta = max(dot(data.Normal, rayDirection), 0.0);

    vec3 H = -normalize(rayDirection + rayDirection);
    //vec3 F0 = mix(vec3(0.04), data.Albedo.xyz, data.Metallic);
    //F = fresnelSchlickRoughness(max(dot(reflect(rayDirection,data.Normal),H), 0.0), F0, data.Roughness);

    //brdf = EvaluateBRDF(data.Normal, data.Albedo.xyz, data.Roughness, data.Metallic,CameraPosition, data.Position,brdf,pdf,F);
   //brdf = SampleBSDF(rayDirection,data.Normal,data.Albedo.xyz,data.Position,data.Roughness,data.Metallic,data.Alpha,1.0f,H,F,pdf);
    //pdf = DistributionGGX(data.Normal, rayDirection, data.Roughness);
    //return vec4((1.0 - F) * (1.0 - data.Metallic) * data.Albedo.xyz,1.0f);
    //vec3 F0 = mix(vec3(0.04), data.Albedo.xyz, data.Metallic);
    
    return vec4(vec3(CookTorranceBRDF(data.Normal,-rayDirection,-rayDirection,vec3(1.0f),0.7,data.Metallic,data.Albedo.xyz)),1.0f);
    //return vec4(vec3(data.Alpha),1.0f);
    */
  
    vec4 Albedo = vec4(vec3(0.0f),1.0f);

    vec3 Origin = rayOrigin;
    vec3 Direction = rayDirection;
    float isShadow = 0.0f;
    float EnvironmentIntensity = 0.7f;
    vec3 SampleVector = vec3(0.0f);

    vec3 Throughput = vec3(1.0f);
    vec3 MaterialEvaluation = vec3(0.0f);
    float NdotV;
    for(int i = 0;i < SampleCount;i++)
    {
        data = TraverseBVH(Origin + Epsilon * distance(CameraPosition,Origin) * Direction,Direction,InvRayDirection,ClosestDistance,true);
        if(ClosestDistance == pos_infinity)
        {
            Albedo.xyz += Throughput * (i == 0 ? 1.0f : EnvironmentIntensity) * texture(EnvironmentCubeMap,Direction).xyz;
            break;
        }
        
        float cosTheta = max(dot(data.Normal, -Direction), 0.0);
        MaterialEvaluation = EvaluateBRDF(data.Normal, data.Albedo.xyz, data.Roughness, data.Metallic,CameraPosition, data.Position,brdf);

        if(length(data.Emissive) > 0.0f)
        {
            if(i == 0) 
            {
                return vec4(data.Emissive.xyz,1.0f);
            }
            else
            {
                Albedo.xyz += Throughput * (data.Emissive.xyz * 30.0f) * cosTheta / pdf;  
                break;
            }
        }

        vec3 H = -normalize(rayDirection + Direction);
        vec3 F0 = mix(vec3(0.04), data.Albedo.xyz, data.Metallic);
        F = fresnelSchlickRoughness(max(dot(data.Normal,Direction), 0.0), F0, data.Roughness);
        Albedo.xyz += Throughput * data.Alpha * (MaterialEvaluation) * cosTheta / pdf;                          
        
        //Throughput *= mix(vec3(1.0f),CookTorranceBRDF(data.Normal,-rayDirection,-Direction,vec3(1.0f),data.Roughness,data.Metallic,data.Albedo.xyz) * cosTheta / pdf,data.Alpha);
        Throughput *= mix(vec3(1.0f),mix(data.Albedo.xyz,F,data.Metallic) / pdf,data.Alpha);
   

        ClosestDistance = pos_infinity;
        Origin = data.Position;
        Direction = SampleBSDF(Direction,data.Normal,data.Albedo.xyz,data.Position,data.Roughness,data.Metallic,data.Alpha,1.0f,H,F,pdf);
        InvRayDirection = 1.0f / Direction;
    }
    return vec4(Albedo.xyz,1.0f);
}

void main()
{
    ivec2 pos = ivec2( gl_GlobalInvocationID.xy );
    seed = uint(Time + ProgressiveRenderedFrameCount);
    seed = pos.x * pos.y * uint(RandomSeed * Time * Time * max(Epsilon,Hash(seed)));
    vec2 uv = (vec2(pos)+vec2(Hash(seed),Hash(seed)))/WindowSize;

    vec2 uvND = uv * 2.0f - 1.0f;

    vec3 rayOrigin = CameraPosition;
    vec4 rayDir4 = inverse(ProjectionViewMat) * vec4(uvND, 1.0, 1.0);
    rayDir4 = vec4(rayDir4.xyz / rayDir4.w,1.0f);
    vec3 rayDirection = normalize(rayDir4.xyz - rayOrigin.xyz);

    vec3 result = TraceRay(rayOrigin,rayDirection.xyz,min(7,max(2,int(0.7f * ProgressiveRenderedFrameCount)))).xyz;
    /*
    for(int i = 0;i < 3;i++)
    {
       result += TraceRay(rayOrigin,rayDirection.xyz,6).xyz;
    }
    result /= 4;
    */
    
    if(ProgressiveRenderedFrameCount >= 0)
    {
       float weight = 1.0 / float(ProgressiveRenderedFrameCount + 1);
       vec4 PreviousFrame = imageLoad( image, pos ).rgba;
       result = mix(PreviousFrame.xyz,result,weight);
    }

    imageStore( image, pos,vec4(result,1.0f));
}