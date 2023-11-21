#pragma once
#include <glew.h>
#include <glfw3.h>
#include "Log.h"
#include "VectorMath.h"
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct KeyPosition
{
	glm::vec3 Position;
	float timeStamp;
};

struct KeyRotation
{
	glm::quat Orientation;
	float timeStamp;
};

struct KeyScale
{
	glm::vec3 Scale;
	float timeStamp;
};

class Bone
{
private:
	std::vector<KeyPosition> Positions;
	std::vector<KeyRotation> Rotations;
	std::vector<KeyScale> Scales;
    
	int NumberOfPositions;
	int NumberOfRotations;
	int NumberOfScalings;

	glm::mat4 LocalTransform;
	std::string Name;
	int ID;

	float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float AnimationTime);
	glm::mat4 InterpolatePosition(float AnimationTime);
	glm::mat4 InterpolateRotation(float AnimationTime);
	glm::mat4 InterpolateScaling(float AnimationTime);


public:

	Bone(const std::string& name, int ID, aiNodeAnim* channel): Name(name),ID(ID),LocalTransform(1.0f)
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

	void Update(float AnimationTime);

	glm::mat4 GetLocalTransformation() { return this->LocalTransform; };
	std::string GetBoneName() { return this->Name; };
	int GetBoneID() { return ID; };

	int GetPositionIndex(float AnimationTime);
	int GetRotationIndex(float AnimationTime);
	int GetScalingIndex(float AnimationTime);

};