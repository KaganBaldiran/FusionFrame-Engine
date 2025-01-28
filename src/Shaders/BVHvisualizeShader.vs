#version 460 core

layout(location = 0) in vec3 vertexdata;

out vec3 CurrentPos;
out vec3 UniqueColor;

layout(std430 , binding = 20) readonly restrict buffer BVHBuffer
{
    vec4 Bounds[];
};

uniform mat4 ProjView;
uniform int NodeCount;

float Hash(inout int x)
{
	x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return float(x & 0xFFFFFF) / float(0x1000000); 
}

 void main()
 {
   uint seed = gl_InstanceID + 1;
   UniqueColor = vec3(Hash(seed),Hash(seed),Hash(seed));

   bool IsLessThanNodeCount = gl_InstanceID < NodeCount;
   vec3 Min = IsLessThanNodeCount ? Bounds[gl_InstanceID].xyz : vec3(0.0f);
   vec3 Max = IsLessThanNodeCount ? Bounds[gl_InstanceID + NodeCount].xyz : vec3(0.0f);
  
   vec3 BoxOrigin = (Min + Max) * 0.5f;
   vec3 BoxSize = Max - Min;

   CurrentPos = BoxOrigin + BoxSize * vertexdata;
   gl_Position = ProjView * vec4(CurrentPos,1.0f);
 }