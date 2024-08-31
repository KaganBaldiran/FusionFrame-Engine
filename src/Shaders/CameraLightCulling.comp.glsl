#version 460 core

#define MAX_LIGHT_PER_CLUSTER 100
#define MAX_LIGHT_COUNT 100

struct Cluster
{
    vec4 Min;
    vec4 Max;
    uint Count;
    uint LightIndices[MAX_LIGHT_PER_CLUSTER];
};

struct Light
{
    vec4 Position;
    vec4 Color;
    float Type;
    float Intensity;
    float Radius;
};

layout(std430, binding = 0) restrict buffer ClusterData
{
    Cluster Clusters[];
};

layout(std430, binding = 4) restrict buffer LightsDatas
{
    Light Lights[MAX_LIGHT_COUNT];
};
uniform int LightCount;

uniform mat4 viewMat;
uniform vec3 GridSize;
uniform vec2 ScreenSize;

uniform float FarPlane;
uniform float ClosePlane;

float AABBSquaredDistance(in vec3 p, in vec3 AABBmin, in vec3 AABBmax)
{
    float sqDist = 0.0f;
    for (int i = 0; i < 3; i++) 
    {
        float v = p[i];
        if (v < AABBmin[i]) sqDist += (AABBmin[i] - v) * (AABBmin[i] - v);
        if (v > AABBmax[i]) sqDist += (v - AABBmax[i]) * (v - AABBmax[i]);
    }
    return sqDist;
}

bool LightClusterCollision(in Light light , in Cluster cluster)
{
    vec4 ViewSpaceLightCenter = viewMat * light.Position;
    float sqDistance = AABBSquaredDistance(ViewSpaceLightCenter.xyz, cluster.Min.xyz, cluster.Max.xyz);
    return sqDistance <= (light.Radius * light.Radius);
}

bool sphereAABBIntersection(vec3 center, float radius, vec3 aabbMin, vec3 aabbMax)
{
    // closest point on the AABB to the sphere center
    vec3 closestPoint = clamp(center, aabbMin, aabbMax);
    // squared distance between the sphere center and closest point
    float distanceSquared = dot(closestPoint - center, closestPoint - center);
    return distanceSquared <= radius * radius;
}

// this just unpacks data for sphereAABBIntersection
bool testSphereAABB(uint i, Cluster cluster)
{
    vec3 center = vec3(viewMat * Lights[i].Position);
    float radius = Lights[i].Radius;

    vec3 aabbMin = cluster.Min.xyz;
    vec3 aabbMax = cluster.Max.xyz;

    return sphereAABBIntersection(center, radius, aabbMin, aabbMax);
}

#define LOCAL_SIZE 128
layout(local_size_x = LOCAL_SIZE, local_size_y = 1, local_size_z = 1) in;
void main()
{
    uint TileIndex = gl_WorkGroupID.x * LOCAL_SIZE + gl_LocalInvocationID.x;
    //uint TileIndex = gl_WorkGroupID.x;
    Cluster cluster = Clusters[TileIndex];

    cluster.Count = 0;
    for (int lightIndex = 0; lightIndex < LightCount; lightIndex++)
    {
       if(LightClusterCollision(Lights[lightIndex], cluster) && cluster.Count < MAX_LIGHT_PER_CLUSTER)
       {
           cluster.LightIndices[cluster.Count] = lightIndex;
           cluster.Count++;
       }
    }
    Clusters[TileIndex] = cluster;
}
