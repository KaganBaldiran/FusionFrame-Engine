#include "Bone.hpp"
#include "../FusionUtility/glm/gtx/quaternion.hpp"

float FUSIONOPENGL::Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float AnimationTime)
{
	float scaleFactor = 0.0f;
	float midwayLength = AnimationTime - lastTimeStamp;
	float framesDiff = nextTimeStamp - lastTimeStamp;
	scaleFactor = midwayLength / framesDiff;
	return scaleFactor;
}

glm::mat4 FUSIONOPENGL::Bone::InterpolatePosition(float AnimationTime)
{
	if (1 == NumberOfPositions)
	{
		return glm::translate(glm::mat4(1.0f), Positions[0].Position);
	}

	int pIndex0 = GetPositionIndex(AnimationTime);
	int pIndex1 = pIndex0 + 1;
	float scaleFactor = GetScaleFactor(Positions[pIndex0].timeStamp, Positions[pIndex1].timeStamp, AnimationTime);
	glm::vec3 finalPosition = glm::mix(Positions[pIndex0].Position, Positions[pIndex1].Position, scaleFactor);
	return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 FUSIONOPENGL::Bone::InterpolateRotation(float AnimationTime)
{
	if (1 == NumberOfPositions)
	{
		auto rotation = glm::normalize(Rotations[0].Orientation);
		return glm::toMat4(rotation);
	}

	int pIndex0 = GetRotationIndex(AnimationTime);
	int pIndex1 = pIndex0 + 1;
	float scaleFactor = GetScaleFactor(Rotations[pIndex0].timeStamp, Rotations[pIndex1].timeStamp, AnimationTime);
	glm::quat finalRotation = glm::slerp(Rotations[pIndex0].Orientation, Rotations[pIndex1].Orientation, scaleFactor);
	return glm::toMat4(glm::normalize(finalRotation));
}

glm::mat4 FUSIONOPENGL::Bone::InterpolateScaling(float AnimationTime)
{
	if (NumberOfScalings == 1)
	{
		return glm::scale(glm::mat4(1.0f), Scales[0].Scale);
	}
	int pIndex0 = GetScalingIndex(AnimationTime);
	int pIndex1 = pIndex0 + 1;
	float scaleFactor = GetScaleFactor(Scales[pIndex0].timeStamp, Scales[pIndex1].timeStamp, AnimationTime);
	glm::vec3 FinalScale = glm::mix(Scales[pIndex0].Scale, Scales[pIndex1].Scale, scaleFactor);
	return glm::scale(glm::mat4(1.0f) , FinalScale);
}

void FUSIONOPENGL::Bone::Update(float AnimationTime)
{
	glm::mat4 translation = InterpolatePosition(AnimationTime);
	glm::mat4 rotation = InterpolateRotation(AnimationTime);
	glm::mat4 scale = InterpolateScaling(AnimationTime);
	LocalTransform = translation * rotation * scale;
}

int FUSIONOPENGL::Bone::GetPositionIndex(float AnimationTime)
{
	for (size_t i = 0; i < NumberOfPositions - 1; i++)
	{
		if (AnimationTime < Positions[i + 1].timeStamp)
		{
			return i;
		}
	}
	assert(0);
}

int FUSIONOPENGL::Bone::GetRotationIndex(float AnimationTime)
{
	for (size_t i = 0; i < NumberOfRotations - 1; i++)
	{
		if (AnimationTime < Rotations[i + 1].timeStamp)
		{
			return i;
		}
	}
	assert(0);
}

int FUSIONOPENGL::Bone::GetScalingIndex(float AnimationTime)
{
	for (size_t i = 0; i < NumberOfScalings - 1; i++)
	{
		if (AnimationTime < Scales[i + 1].timeStamp)
		{
			return i;
		}
	}
	assert(0);
}
