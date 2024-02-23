#pragma once
#include "Animation.hpp"

#define MAX_BONE_COUNT 100

namespace FUSIONOPENGL
{
	class Animator
	{
	public: 
		Animator(Animation* Animation);
		void UpdateAnimation(float dt);
		void PlayAnimation(Animation* Animation);
		void CalculateBoneTransform(const BoneNodeData* node, glm::mat4 parentTransform);
		std::vector<glm::mat4> GetFinalBoneMatrices();
	private:
		std::vector<glm::mat4> FinalBoneMatrices;
		std::vector<glm::mat4> GlobalBoneMatrices;
		Animation* CurrentAnimation;
		float CurrentTime;
		float DeltaTime;
	};

}