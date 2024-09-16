#version 460 core

layout(local_size_x=32,local_size_y=32,local_size_z=1) in;
layout(rgba32f,binding=0) uniform image2D image;

layout (location = 1) uniform vec2 WindowSize;
layout (location = 2) uniform vec3 CameraPosition;
layout (location = 3) uniform mat4 ProjectionViewMat;
layout (location = 4) uniform float Time;
layout (location = 5) uniform int TriangleCount;
layout (location = 6) uniform int NodeCount;

layout (location = 7) uniform samplerBuffer MinBounds;
layout (location = 8) uniform samplerBuffer MaxBounds;
layout (location = 9) uniform samplerBuffer ChildIndicies;
layout (location = 10) uniform samplerBuffer TriangleIndicies;
layout (location = 11) uniform samplerBuffer TriangleCounts;

/*
readonly layout(std430, binding=8) restrict buffer GeometryData
{
    vec4 MinBounds[];
    vec4 MaxBounds[];
    int ChildIndicies[];
    int TriangleIndicies[];
    int TriangleCounts[];
};
*/

readonly layout(std430,binding=7) restrict buffer TriangleData
{
  vec4 TrianglePositions[]; 
};

#define BACKGROUND_COLOR vec4(1.0f)
const float pos_infinity = uintBitsToFloat(0x7F800000);
const float neg_infinity = uintBitsToFloat(0xFF800000);

struct IntersectionData
{
   float t;
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

bool RayAABBIntersection(in vec3 rayOrigin,in vec3 rayDirectionInv,in vec3 BoxMin,in vec3 BoxMax)
{
   vec3 tMin = (BoxMin.xyz - rayOrigin) * rayDirectionInv;
   vec3 tMax = (BoxMax.xyz - rayOrigin) * rayDirectionInv;

   vec3 t1 = min(tMin,tMax);
   vec3 t2 = max(tMin,tMax);
  
   float NearDistance = max(max(t1.x,t1.y),t1.z);
   float FarDistance = min(min(t2.x,t2.y),t2.z);

   bool DoesIntersect = FarDistance >= NearDistance && FarDistance > 0;
   return DoesIntersect;
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


vec3 TraverseBVH(in vec3 rayOrigin,in vec3 rayDirection)
{
     int NodesToProcess[32];
     int StackIndex = 0; 
     NodesToProcess[StackIndex] = 0;
     StackIndex++;

     int CurrentIndex;
     vec3 InvRayDirection = 1.0f / (rayDirection + vec3(0.00001f));

     vec3 ClosestTriangleData = vec3(1.0f);
     float ClosestDistance = pos_infinity;
     while(StackIndex > 0)
     {
        StackIndex--;
        CurrentIndex = NodesToProcess[StackIndex];
        
        if(RayAABBIntersection(rayOrigin,InvRayDirection,texelFetch(MinBounds,CurrentIndex).xyz,texelFetch(MaxBounds,CurrentIndex).xyz))
        {
           int ChildIndex = int(texelFetch(ChildIndicies,CurrentIndex).r);
           if(ChildIndex == -1)
           {
                //ClosestTriangleData = vec3(1.0,0.0f,0.0f);
                int TriangleIndex = int(texelFetch(TriangleIndicies,CurrentIndex).r);
	            int TriCount = int(texelFetch(TriangleCounts,CurrentIndex).r);
                for(int i = TriangleIndex;i < TriangleIndex + TriCount;i++)
                {
                    vec3 v0 = TrianglePositions[i * 3].xyz;
                    vec3 v1 = TrianglePositions[i * 3 + 1].xyz;
                    vec3 v2 = TrianglePositions[i * 3 + 2].xyz;

                    IntersectionData result = RayTriangleIntersection(rayOrigin,rayDirection.xyz,v0,v1,v2);

                    if(result.t > 0.0f)
                    {
                        vec3 e1 = v1 - v0;
                        vec3 e2 = v2 - v0;
                        vec3 N = normalize(cross(e1,e2));
                        if(dot(N,rayDirection) < 0.0f)
                        {
                            if(result.t < ClosestDistance)
                            {
                                ClosestDistance = result.t;
                                ClosestTriangleData = N;
                                //StackIndex = -1;
                           }
                        }
                     }
                 }
           }
           else
           {
              NodesToProcess[StackIndex] = ChildIndex + 1;
              StackIndex++;
              NodesToProcess[StackIndex] = ChildIndex;
              StackIndex++;
           }
        }
     }
     return ClosestTriangleData;
}


void main()
{
    ivec2 pos = ivec2( gl_GlobalInvocationID.xy );
    vec2 uv = (vec2(pos)+0.5)/WindowSize;

    //vec2 ImageSize = imageSize(image);
    //uv.x *= (ImageSize.x / ImageSize.y);

    vec2 uvND = uv * 2.0f - 1.0f;

    vec3 rayOrigin = CameraPosition + vec3(0.0f,0.0f,0.0f);
    vec4 rayDir4 = inverse(ProjectionViewMat) * vec4(uvND, 1.0, 1.0);
    vec3 rayDirection = normalize(rayDir4.xyz);

    float SphereRadius = 2.0f;

    //vec4 in_val = imageLoad( image, pos ).rgba;

    /*
    vec3 ClosestTriangleData = vec3(1.0f);
    float ClosestDistance = pos_infinity;
    for(int i = 0;i < TriangleCount - 3;i+=3)
    {
       vec3 v0 = TrianglePositions[i].xyz;
       vec3 v1 = TrianglePositions[i + 1].xyz;
       vec3 v2 = TrianglePositions[i + 2].xyz;

       IntersectionData result = RayTriangleIntersection(rayOrigin,rayDirection.xyz,v0,v1,v2);

       if(result.t > 0.0f)
       {
          vec3 e1 = v1 - v0;
          vec3 e2 = v2 - v0;
          vec3 N = normalize(cross(e1,e2));
          if(dot(N,rayDirection) < 0.0f)
          {
            if(result.t < ClosestDistance)
            {
               ClosestDistance = result.t;
               ClosestTriangleData = N;
            }
          }
       }
    }
    */

  // vec3 result = TraverseBVH(rayOrigin,rayDirection.xyz).xyz;

   vec3 result = vec3(0.0f);

   /*
    for (int i = 0; i < NodeCount; i++)
    {
        int triangleCountInAABB = int(texelFetch(TriangleCounts,i).r);
        int ChildIndexInAABB = int(texelFetch(ChildIndicies,i).r); 
        if (ChildIndexInAABB == -1) continue; 

        vec3 BoxMin = texelFetch(MinBounds,i).xyz;
        vec3 BoxMax = texelFetch(MaxBounds,i).xyz;

        vec3 invRayDir = vec3(1.0) / (rayDirection.xyz + vec3(0.000001)); 

        // Check for intersection
        if (RayAABBIntersection(rayOrigin, invRayDir, BoxMin, BoxMax))
        {
            result = vec3(ChildIndexInAABB); 
            break;
        }
    }
    */



/*
if (pos.x * pos.y < NodeCount)
{
   result = vec3(texelFetch(ChildIndicies,pos.x * pos.y).x); 
}
*/

    //imageStore( image, pos,RayTraceSphere(rayOrigin,rayDirection.xyz,SphereRadius));
    //imageStore( image, pos,vec4(ClosestTriangleData,1.0f));
   //imageStore( image, pos,vec4(result,1.0f));
   imageStore( image, pos,vec4(TraverseBVH(rayOrigin,rayDirection.xyz).xyz,1.0f));
}