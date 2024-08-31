#pragma once
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include <vector>
#include "../FusionUtility/FusionDLLExport.h"

//Forward declaration
struct aiNodeAnim;

namespace FUSIONCORE
{
	struct FUSIONFRAME_EXPORT KeyPosition
	{
		glm::vec3 Position;
		float timeStamp;
	};

	struct FUSIONFRAME_EXPORT KeyRotation
	{
		glm::quat Orientation;
		float timeStamp;
	};

	struct FUSIONFRAME_EXPORT KeyScale
	{
		glm::vec3 Scale;
		float timeStamp;
	};

	class FUSIONFRAME_EXPORT Bone
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

		Bone(const std::string& name, int ID, aiNodeAnim* channel);
		void Update(float AnimationTime);

		glm::mat4 GetLocalTransformation() { return this->LocalTransform; };
		std::string GetBoneName() { return this->Name; };
		int GetBoneID() { return ID; };

		int GetPositionIndex(float AnimationTime);
		int GetRotationIndex(float AnimationTime);
		int GetScalingIndex(float AnimationTime);
	};
}