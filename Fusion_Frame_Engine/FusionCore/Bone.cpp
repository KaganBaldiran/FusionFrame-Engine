#include "Bone.hpp"
#include "../FusionUtility/glm/gtx/quaternion.hpp"
#include <glew.h>
#include <glfw3.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

float FUSIONCORE::Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float AnimationTime)
{
	float scaleFactor = 0.0f;
	float midwayLength = AnimationTime - lastTimeStamp;
	float framesDiff = nextTimeStamp - lastTimeStamp;
	scaleFactor = midwayLength / framesDiff;
	return scaleFactor;
}

glm::mat4 FUSIONCORE::Bone::InterpolatePosition(float AnimationTime)
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

glm::mat4 FUSIONCORE::Bone::InterpolateRotation(float AnimationTime)
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

glm::mat4 FUSIONCORE::Bone::InterpolateScaling(float AnimationTime)
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

FUSIONCORE::Bone::Bone(const std::string& name, int ID, aiNodeAnim* channel) : Name(name), ID(ID), LocalTransform(1.0f)
{
	NumberOfPositions = channel->mNumPositionKeys;

	for (size_t i = 0; i < NumberOfPositions; i++)
	{
		aiVector3D aiPosition = channel->mPositionKeys[i].mValue;
		float timeStamp = channel->mPositionKeys[i].mTime;
		KeyPosition data;
		data.Position.x = aiPosition.x;
		data.Position.y = aiPosition.y;
		data.Position.z = aiPosition.z;

		data.timeStamp = timeStamp;
		Positions.push_back(data);
	}

	NumberOfRotations = channel->mNumRotationKeys;

	for (size_t i = 0; i < NumberOfRotations; i++)
	{
		aiQuaternion Orientation = channel->mRotationKeys[i].mValue;
		float timeStamp = channel->mRotationKeys[i].mTime;

		KeyRotation data;
		data.Orientation = glm::quat(Orientation.w, Orientation.x, Orientation.y, Orientation.z);
		data.timeStamp = timeStamp;
		Rotations.push_back(data);
	}

	NumberOfScalings = channel->mNumScalingKeys;

	for (size_t i = 0; i < NumberOfScalings; i++)
	{
		aiVector3D scale = channel->mScalingKeys[i].mValue;
		float timeStamp = channel->mScalingKeys[i].mTime;

		KeyScale data;
		data.Scale.x = scale.x;
		data.Scale.y = scale.y;
		data.Scale.z = scale.z;

		data.timeStamp = timeStamp;
		Scales.push_back(data);
	}
}

void FUSIONCORE::Bone::Update(float AnimationTime)
{
	glm::mat4 translation = InterpolatePosition(AnimationTime);
	glm::mat4 rotation = InterpolateRotation(AnimationTime);
	glm::mat4 scale = InterpolateScaling(AnimationTime);
	LocalTransform = translation * rotation * scale;
}

int FUSIONCORE::Bone::GetPositionIndex(float AnimationTime)
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

int FUSIONCORE::Bone::GetRotationIndex(float AnimationTime)
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

int FUSIONCORE::Bone::GetScalingIndex(float AnimationTime)
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
