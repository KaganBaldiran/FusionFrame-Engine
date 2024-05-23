#pragma once
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include <vector>
#include "Bone.hpp"
#include "Model.hpp"
#include "../FusionUtility/FusionDLLExport.h"

struct aiAnimation;
struct aiNode;

namespace FUSIONCORE
{
	struct FUSIONFRAME_EXPORT BoneNodeData
	{
		glm::mat4 transformation;
		std::string name;
		int childrenCount;
		std::vector<BoneNodeData> children;
	};

	/**
	 * Manages animation data for a 3D model.
	 *
	 * The Animation class is responsible for handling animation data associated with a 3D model.
	 * Key functionalities include:
	 * - Loading animation data from a specified file path.
	 * - Providing access to the ticks per second and duration of the animation.
	 * - Managing the hierarchy of bones and their transformations.
	 *
	 * Example usage:
	 * Model* myModel = new Model("path/to/model");
	 * Animation myAnimation("path/to/animation", myModel);
	 *
	 * float duration = myAnimation.GetDuration();
	 * float ticksPerSecond = myAnimation.GetTicksPerSecond();
	 * Bone* bone = myAnimation.FindBone("BoneName");
	 */
	class FUSIONFRAME_EXPORT Animation
	{
	public:
		Animation(const char* AnimationPath, Model* model);
		inline float GetTicksPerSecond() { return TicksPerSecond; };
		inline float GetDuration() { return Duration; };
		inline BoneNodeData& GetNodeData() { return this->rootnode; };
		inline const std::map<std::string, BoneInfo>& GetBoneIDMap()
		{
			return this->BoneInfoMap;
		}

		Bone* FindBone(const std::string& name);
		inline Model*& GetModelPointer() { return this->ModelPointer; };

	private:

		void ReadMissingBones(const aiAnimation* animation, Model& model);
		void ReadHeirarchyData(BoneNodeData& dest, const aiNode* src);

		float Duration;
		int TicksPerSecond;
		Model* ModelPointer;
		std::vector<Bone> Bones;
		BoneNodeData rootnode;
		std::map<std::string, BoneInfo> BoneInfoMap;
	};
}
