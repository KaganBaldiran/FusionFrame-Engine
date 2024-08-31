#version 460 core

#define MAX_LIGHT_PER_CLUSTER 100

struct Cluster
{
    vec4 Min;
    vec4 Max;
    uint Count;
    uint LightIndices[MAX_LIGHT_PER_CLUSTER];
};

layout(std430, binding = 0) restrict buffer ClusterData
{
    Cluster Clusters[];
};

uniform mat4 InverseProjectionMatrix;
uniform vec3 GridSize;
uniform vec3 CameraOrientation;
uniform vec2 ScreenSize;

uniform float FarPlane;
uniform float ClosePlane;

#define CLOSE_PLANE_DEFAULT -1
#define FAR_PLANE_DEFAULT 1

vec3 ScreenSpaceToViewSpace(in vec2 Point)
{
    vec4 ViewSpacePoint = vec4((Point / ScreenSize) * 2.0f - 1.0f,CLOSE_PLANE_DEFAULT,1.0f);
    ViewSpacePoint = InverseProjectionMatrix * ViewSpacePoint;
    ViewSpacePoint = ViewSpacePoint / ViewSpacePoint.w;
    return ViewSpacePoint.xyz;
}

vec3 lineIntersectionWithZPlane(vec3 startPoint, vec3 endPoint, float zDistance)
{
    vec3 direction = endPoint - startPoint;
    vec3 normal = vec3(0.0, 0.0, -1.0); 
    float t = (zDistance - dot(normal, startPoint)) / dot(normal, direction);
    return startPoint + t * direction; 
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    const vec3 eyePos = vec3(0.0);

    uint TileIndex = uint(gl_WorkGroupID.x * gl_WorkGroupID.y * GridSize.x + gl_WorkGroupID.z * GridSize.x * GridSize.y);
    vec2 tileSize = ScreenSize / GridSize.xy;

    vec2 MinScreenSpace = gl_WorkGroupID.xy * tileSize;
    vec2 MaxScreenSpace = (gl_WorkGroupID.xy + 1) * tileSize;

    vec3 MinViewSpace = ScreenSpaceToViewSpace(MinScreenSpace);
    vec3 MaxViewSpace = ScreenSpaceToViewSpace(MaxScreenSpace);

    float ClusterNear = ClosePlane * pow(FarPlane / ClosePlane, float(gl_WorkGroupID.z) / GridSize.z);
    float ClusterFar = ClosePlane * pow(FarPlane / ClosePlane, float(gl_WorkGroupID.z + 1) / GridSize.z);

    vec3 minPointNear =
        lineIntersectionWithZPlane(eyePos, MinViewSpace, ClusterNear);
    vec3 minPointFar =
        lineIntersectionWithZPlane(eyePos, MinViewSpace, ClusterFar);
    vec3 maxPointNear =
        lineIntersectionWithZPlane(eyePos, MaxViewSpace, ClusterNear);
    vec3 maxPointFar =
        lineIntersectionWithZPlane(eyePos, MaxViewSpace, ClusterFar);

    vec3 minPointAABB = min(minPointNear, minPointFar);
    vec3 maxPointAABB = max(maxPointNear, maxPointFar);

    Clusters[TileIndex].Min = vec4(minPointAABB, 0.0);
    Clusters[TileIndex].Max = vec4(maxPointAABB, 0.0);
}

