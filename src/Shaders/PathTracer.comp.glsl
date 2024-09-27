#version 460 core

layout(local_size_x=32,local_size_y=32,local_size_z=1) in;
layout(rgba32f,binding=0) uniform image2D image;

layout (location = 1) uniform samplerBuffer MinBounds;
layout (location = 2) uniform samplerBuffer MaxBounds;
layout (location = 3) uniform samplerBuffer ChildIndicies;
layout (location = 4) uniform samplerBuffer TriangleIndicies;
layout (location = 5) uniform samplerBuffer TriangleCounts;

layout (location = 6) uniform vec2 WindowSize;
layout (location = 7) uniform vec3 CameraPosition;
layout (location = 8) uniform mat4 ProjectionViewMat;
layout (location = 9) uniform float Time;

layout (location = 10) uniform samplerBuffer ModelAlbedos;


readonly layout(std430,binding=6) restrict buffer TriangleDataNormals
{
  vec4 TriangleNormals[]; 
};

readonly layout(std430,binding=7) restrict buffer TriangleDataPositions
{
  vec4 TrianglePositions[]; 
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

layout (location = 14) uniform int LightCount;

layout(std430 , binding = 4) restrict buffer LightsDatas
{
    Light Lights[];
};

#define BACKGROUND_COLOR vec4(1.0f)
const float pos_infinity = uintBitsToFloat(0x7F800000);
const float neg_infinity = uintBitsToFloat(0xFF800000);

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
   vec2 uv;
};

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

   bool DoesIntersect = FarDistance >= NearDistance && FarDistance > 0;
   return DoesIntersect ? NearDistance : pos_infinity;
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


RayData TraverseBVH(in vec3 rayOrigin,in vec3 rayDirection,in vec3 InvRayDirection,inout float ClosestDistance)
{
     int NodesToProcess[50];
     int StackIndex = 0; 
     NodesToProcess[StackIndex] = 0;
     StackIndex++;

     int CurrentIndex;
     
     RayData data;
     data.Normal = vec3(0.0f);
     data.Position = vec3(0.0f);
     data.Albedo = vec4(0.0f,0.0f,0.0f,1.0f);
     data.uv = vec2(0.0f);

     int NearChildIndex = -1;
     int FarChildIndex = -1;
     while(StackIndex > 0)
     {
        StackIndex--;
        CurrentIndex = NodesToProcess[StackIndex];
        
        int ChildIndex = int(texelFetch(ChildIndicies,CurrentIndex).r);
        if(ChildIndex == -1)
        {
            //ClosestTriangleData = vec3(1.0,0.0f,0.0f);
            int TriangleIndex = int(texelFetch(TriangleIndicies,CurrentIndex).r);
	        int TriCount = int(texelFetch(TriangleCounts,CurrentIndex).r);
            for(int i = TriangleIndex;i < TriangleIndex + TriCount;i++)
            {
                vec4 v0 = TrianglePositions[i * 3].xyzw;
                vec3 v1 = TrianglePositions[i * 3 + 1].xyz;
                vec3 v2 = TrianglePositions[i * 3 + 2].xyz;

                IntersectionData result = RayTriangleIntersection(rayOrigin,rayDirection.xyz,v0.xyz,v1,v2);

                if(result.t > 0.0f)
                {
                    if(result.t < ClosestDistance)
                    {
                        vec2 TriangleUV = result.uv;
                            
                        vec3 n0 = TriangleNormals[i * 3].xyz;
                        vec3 n1 = TriangleNormals[i * 3 + 1].xyz;
                        vec3 n2 = TriangleNormals[i * 3 + 2].xyz;
                        vec3 Normal = normalize((1.0f - TriangleUV.x - TriangleUV.y) * n0 + TriangleUV.x * n1 + TriangleUV.y * n2);
                            
                        ClosestDistance = result.t;
                        data.Position = rayOrigin + rayDirection * result.t;
                        data.Normal = Normal;
                        vec4 temp = ProjectionViewMat * vec4(data.Position,1.0f);
                        temp.xyz = temp.xyz / temp.w;
                        data.uv = temp.xy * 0.5f + 0.5f;
                        data.Albedo = texelFetch(ModelAlbedos,int(v0.w));
                    }  
                 }
            }
        }
        else
        {
            float DistanceChild0 = RayAABBIntersection(rayOrigin,InvRayDirection,texelFetch(MinBounds,ChildIndex).xyz,texelFetch(MaxBounds,ChildIndex).xyz);
            float DistanceChild1 = RayAABBIntersection(rayOrigin,InvRayDirection,texelFetch(MinBounds,ChildIndex + 1).xyz,texelFetch(MaxBounds,ChildIndex + 1).xyz);
           
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
     return data;
}

vec3 RayTrace(in vec3 rayOrigin,in vec3 rayDirection)
{
   const float Epsilon = 0.0001f;
   float ClosestDistance = pos_infinity;
   vec3 InvRayDirection = 1.0f / (rayDirection);
   RayData data = TraverseBVH(rayOrigin,rayDirection,InvRayDirection,ClosestDistance);
   
   vec3 Color = vec3(data.Albedo);
   vec3 ReflectedRay;
   vec3 NewRayOrigin;
   int MaxBounces = 2;
   for(int i = 0;i < MaxBounces;i++)
   {
       if(ClosestDistance == pos_infinity)
       {
         break;
       }
       ClosestDistance = pos_infinity;
       ReflectedRay = reflect(rayDirection,data.Normal);
       NewRayOrigin = data.Position + ReflectedRay * Epsilon;
       InvRayDirection = 1.0f / (ReflectedRay);
       data = TraverseBVH(NewRayOrigin,ReflectedRay,InvRayDirection,ClosestDistance);

       Color += vec3(data.Albedo);
   }
   Color = Color / int(MaxBounces);
   return vec3(Color);
   //return vec3(data.Normal);
}

void main()
{
    ivec2 pos = ivec2( gl_GlobalInvocationID.xy );
    vec2 uv = (vec2(pos)+0.5f)/WindowSize;

    vec2 uvND = uv * 2.0f - 1.0f;

    vec3 rayOrigin = CameraPosition;
    vec4 rayDir4 = inverse(ProjectionViewMat) * vec4(uvND, 1.0, 1.0);
    rayDir4 = vec4(rayDir4.xyz / rayDir4.w,1.0f);
    vec3 rayDirection = normalize(rayDir4.xyz);

    vec4 in_val = imageLoad( image, pos ).rgba;

    vec3 result = RayTrace(rayOrigin,rayDirection.xyz);
    //vec3 Color = vec3(1.0f,0.0f,0.0f) * dot(result,vec3(0.5f,0.4f,-0.7f));
    imageStore( image, pos,vec4(result,1.0f));
}