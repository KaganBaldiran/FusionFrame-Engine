#include "Bone.hpp"

float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float AnimationTime)
{
	return 0.0f;
}

glm::mat4 Bone::InterpolatePosition(float AnimationTime)
{
	return glm::mat4();
}

glm::mat4 Bone::InterpolateRotation(float AnimationTime)
{
	return glm::mat4();
}

glm::mat4 Bone::InterpolateScaling(float AnimationTime)
{
	return glm::mat4();
}

void Bone::Update(float AnimationTime)
{
	glm::mat4 translation = InterpolatePosition(AnimationTime);
	glm::mat4 rotation = InterpolateRotation(AnimationTime);
	glm::mat4 scale = InterpolateScaling(AnimationTime);
	LocalTransform = translation * rotation * scale;
}

int Bone::GetPositionIndex(float AnimationTime)
{
	return 0;
}

int Bone::GetRotationIndex(float AnimationTime)
{
	return 0;
}

int Bone::GetScalingIndex(float AnimationTime)
{
	return 0;
}
