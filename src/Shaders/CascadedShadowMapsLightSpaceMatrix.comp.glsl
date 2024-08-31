#version 460 core

layout(location = 0) uniform float CameraFov;
layout(location = 1) uniform float CameraAspectRatio;
layout(location = 2) uniform vec3 CameraUpVector;
layout(location = 3) uniform mat4 ViewMat;
layout(location = 4) uniform float FarPlane;
layout(location = 5) uniform float NearPlane;

#define MAX_CASCADE_PLANE_COUNT 16
#define MAX_CASCADED_SHADOW_MAP_COUNT 12

layout(std430, binding = 10) buffer CascadedMapMetaDatas
{
	mat4 LightMatrices[MAX_CASCADE_PLANE_COUNT * MAX_CASCADED_SHADOW_MAP_COUNT];
	vec4 PositionAndSize[MAX_CASCADE_PLANE_COUNT * MAX_CASCADED_SHADOW_MAP_COUNT];
	vec4 LightDirection[MAX_CASCADED_SHADOW_MAP_COUNT];
	float ShadowCascadeLevels[MAX_CASCADE_PLANE_COUNT * MAX_CASCADED_SHADOW_MAP_COUNT];
	float Layer[MAX_CASCADE_PLANE_COUNT * MAX_CASCADED_SHADOW_MAP_COUNT];
	float CascadeCount[MAX_CASCADED_SHADOW_MAP_COUNT];
};

const uint UINT_MAX = 4294967295u; // 2^32 - 1

mat4 translate(vec3 offset) {
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(offset, 1.0)
    );
}

mat4 ortho(float left, float right, float bottom, float top, float near, float far) {
    float tx = -(right + left) / (right - left);
    float ty = -(top + bottom) / (top - bottom);
    float tz = -(far + near) / (far - near);

    return mat4(
        2.0 / (right - left), 0.0, 0.0, 0.0,
        0.0, 2.0 / (top - bottom), 0.0, 0.0,
        0.0, 0.0, -2.0 / (far - near), 0.0,
        tx, ty, tz, 1.0
    );
}

mat4 perspective(float fov, float aspect, float near, float far) {
    float rad = radians(fov); 
    float tanHalfFovy = tan(rad / 2.0);

    mat4 result = mat4(0.0);
    result[0][0] = 1.0 / (aspect * tanHalfFovy);
    result[1][1] = 1.0 / (tanHalfFovy);
    result[2][2] = -(far + near) / (far - near);
    result[2][3] = -1.0;
    result[3][2] = -(2.0 * far * near) / (far - near);

    return result;
}

mat4 lookAt(vec3 eye, vec3 center, vec3 up) {
    vec3 z = normalize(eye - center); 
    vec3 x = normalize(cross(up, z)); 
    vec3 y = cross(z, x); 

    mat4 result = mat4(1.0); 
    result[0][0] = x.x;
    result[1][0] = x.y;
    result[2][0] = x.z;
    result[0][1] = y.x;
    result[1][1] = y.y;
    result[2][1] = y.z;
    result[0][2] = z.x;
    result[1][2] = z.y;
    result[2][2] = z.z;
    result[3][0] = -dot(x, eye);
    result[3][1] = -dot(y, eye);
    result[3][2] = -dot(z, eye);

    return result;
}


mat4 GetLightSpaceMatrix(float nearPlane,float farPlane,vec3 LightDirection)
{
	const mat4 proj = perspective(CameraFov, CameraAspectRatio, nearPlane, farPlane);
	mat4 inverseMatrix = inverse(proj * ViewMat);

	const int FrustumCornersSize = 8;
	vec4 FrustumCorners[FrustumCornersSize];
	for (int x = 0; x < 2; x++)
	{
		for (int y = 0; y < 2; y++)
		{
			for (int z = 0; z < 2; z++)
			{
				vec4 Point = inverseMatrix * vec4(2.0f * x - 1.0f,
					2.0f * y - 1.0f,
					2.0f * z - 1.0f,
					1.0f);
				FrustumCorners[x + y * 2 + z * 4] = (Point / Point.w);
			}
		}
	}

	vec3 center = vec3(0.0f);
	for (int i = 0; i < FrustumCornersSize; i++)
	{
		center += vec3(FrustumCorners[i]);
	}
	center /= FrustumCornersSize;

	const mat4 LightView = lookAt(center + LightDirection, center, CameraUpVector);

	const float FLT_MAX = 3.402823466e+38;  
	const float FLT_MIN = -3.402823466e+38; 

	float minX = FLT_MAX;
	float maxX = FLT_MIN;
	float minY = FLT_MAX;
	float maxY = FLT_MIN;
	float minZ = FLT_MAX;
	float maxZ = FLT_MIN;

	for (int i = 0; i < FrustumCornersSize; i++)
	{
		const vec4 LightSpaceCorner = LightView * FrustumCorners[i];
		minX = min(minX, LightSpaceCorner.x);
		maxX = max(maxX, LightSpaceCorner.x);
		minY = min(minY, LightSpaceCorner.y);
		maxY = max(maxY, LightSpaceCorner.y);
		minZ = min(minZ, LightSpaceCorner.z);
		maxZ = max(maxZ, LightSpaceCorner.z);
	}

	float temp = -minZ;
    minZ = -maxZ;
    maxZ = temp;

	float mid = (maxZ - minZ) / 2;
    minZ -= mid * 5.0f;
    maxZ += mid * 5.0f;

	const float Zmultiplier = abs(FarPlane - NearPlane) * 0.05f;
	if (minZ < 0.0f)
	{
		minZ *= Zmultiplier;
	}
	else
	{
		minZ /= Zmultiplier;
	}

	if (maxZ < 0.0f)
	{
		maxZ /= Zmultiplier;
	}
	else
	{
		maxZ *= Zmultiplier;
	}

	const mat4 LightProjectionMatrix = ortho(minX, maxX, minY, maxY, minZ, maxZ);
	return LightProjectionMatrix * LightView;
}

void GetLightSpaceMatrices(int MetaDataIndex)
{
	int IndexOffset = MetaDataIndex * MAX_CASCADE_PLANE_COUNT;
	int ShadowCascadeCount = int(CascadeCount[MetaDataIndex]);
	vec3 LightDirection = LightDirection[MetaDataIndex].xyz;
	for (int i = 0; i < ShadowCascadeCount + 1; i++)
	{
		int CurrentIndex = IndexOffset + i;
		if (i == 0)
		{
			LightMatrices[CurrentIndex] = GetLightSpaceMatrix(NearPlane, ShadowCascadeLevels[CurrentIndex], LightDirection);
		}
		else if (i < ShadowCascadeCount)
		{
			LightMatrices[CurrentIndex] = GetLightSpaceMatrix(ShadowCascadeLevels[CurrentIndex - 1], ShadowCascadeLevels[CurrentIndex], LightDirection);
		}
		else
		{
			LightMatrices[CurrentIndex] = GetLightSpaceMatrix(ShadowCascadeLevels[CurrentIndex - 1],FarPlane, LightDirection);
		}
	}
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
    int index = int(gl_GlobalInvocationID.x);
	GetLightSpaceMatrices(index);
}