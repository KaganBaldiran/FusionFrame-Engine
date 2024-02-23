#include "Animator.hpp"

FUSIONOPENGL::Animator::Animator(Animation* Animation)
{
	this->CurrentTime = 0.0f;
	this->CurrentAnimation = Animation;

	FinalBoneMatrices.reserve(MAX_BONE_COUNT);
	Animation->GetModelPointer()->GetAnimationMatricesPointer() = &FinalBoneMatrices;
	GlobalBoneMatrices.reserve(MAX_BONE_COUNT);

	for (size_t i = 0; i < MAX_BONE_COUNT; i++)
	{
		FinalBoneMatrices.push_back(glm::mat4(1.0f));
	}
}

void FUSIONOPENGL::Animator::UpdateAnimation(float dt)
{
	this->DeltaTime = dt;
	if (CurrentAnimation)
	{
		CurrentTime += CurrentAnimation->GetTicksPerSecond() * dt;
		CurrentTime = fmod(CurrentTime, CurrentAnimation->GetDuration());
		CalculateBoneTransform(&CurrentAnimation->GetNodeData() , glm::mat4(1.0f));
	}
}

void FUSIONOPENGL::Animator::PlayAnimation(Animation* Animation)
{
	this->CurrentAnimation = Animation;
	this->CurrentTime = 0.0f;
}

void FUSIONOPENGL::Animator::CalculateBoneTransform(const BoneNodeData* node, glm::mat4 parentTransform)
{
	std::string nodeName = node->name;
	glm::mat4 nodeTransform = node->transformation;

	Bone* Bone = CurrentAnimation->FindBone(nodeName);

	if (Bone)
	{
		Bone->Update(CurrentTime);
		nodeTransform = Bone->GetLocalTransformation();
	}

	glm::mat4 globalTransformation = parentTransform * nodeTransform;

	auto boneInfoMap = CurrentAnimation->GetBoneIDMap();
	if (boneInfoMap.find(nodeName) != boneInfoMap.end())
	{
		int index = boneInfoMap[nodeName].id;
		glm::mat4 offset = boneInfoMap[nodeName].OffsetMat;
		FinalBoneMatrices[index] = globalTransformation * offset;
	}

	for (int i = 0; i < node->childrenCount; i++)
	{
		CalculateBoneTransform(&node->children[i], globalTransformation);
	}
}

std::vector<glm::mat4> FUSIONOPENGL::Animator::GetFinalBoneMatrices()
{
	return FinalBoneMatrices;
}
