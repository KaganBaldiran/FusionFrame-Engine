#version 460 core

layout(location = 0) in vec3 vertexdata;

out vec3 CurrentPos;
out vec3 UniqueColor;

layout (location = 1) uniform samplerBuffer MinBounds;
layout (location = 2) uniform samplerBuffer MaxBounds;
layout (location = 3) uniform samplerBuffer TriangleCounts;

uniform mat4 ProjView;

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
   vec3 TriangleCount = texelFetch(TriangleCounts,gl_InstanceID).xyz;
   UniqueColor = vec3(Hash(seed),Hash(seed),Hash(seed));

   vec3 Min = texelFetch(MinBounds,gl_InstanceID).xyz;
   vec3 Max = texelFetch(MaxBounds,gl_InstanceID).xyz;

   vec3 BoxOrigin = (Min + Max) * 0.5f;
   vec3 BoxSize = Max - Min;

   CurrentPos = BoxOrigin + BoxSize * vertexdata;
   gl_Position = ProjView * vec4(CurrentPos,1.0f);
 }