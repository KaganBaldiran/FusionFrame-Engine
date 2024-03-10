#include "Animator.hpp"

namespace FUSIONCORE
{
	std::unique_ptr<UBO> AnimationUniformBufferObject;
}

FUSIONCORE::Animator::Animator(Animation* Animation)
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

void FUSIONCORE::Animator::UpdateAnimation(float dt)
{
	this->DeltaTime = dt;
	if (CurrentAnimation)
	{
		CurrentTime += CurrentAnimation->GetTicksPerSecond() * dt;
		CurrentTime = fmod(CurrentTime, CurrentAnimation->GetDuration());
		CalculateBoneTransform(&CurrentAnimation->GetNodeData() , glm::mat4(1.0f));
	}
}

void FUSIONCORE::Animator::UpdateBlendedAnimation(Animation* BaseAnimation, Animation* LayeredAnimation, float blendFactor, float deltaTime)
{
	float a = 1.0f;
	float b = BaseAnimation->GetDuration() / LayeredAnimation->GetDuration();
	const float SpeedMultiplierUp = (1.0f - blendFactor) * a + b * blendFactor;

	a = LayeredAnimation->GetDuration() / BaseAnimation->GetDuration();
	b = 1.0f;
	const float SpeedMultiplierDown = (1.0f - blendFactor) * a + b * blendFactor;

	static float currentTimeBaseAnim = 0.0f;
	currentTimeBaseAnim += BaseAnimation->GetTicksPerSecond() * deltaTime * SpeedMultiplierUp;
	currentTimeBaseAnim = fmod(currentTimeBaseAnim, BaseAnimation->GetDuration());

	static float currentTimeLayeredAnim = 0.0f;
	currentTimeLayeredAnim += LayeredAnimation->GetTicksPerSecond() * deltaTime * SpeedMultiplierDown;
	currentTimeLayeredAnim = fmod(currentTimeLayeredAnim, LayeredAnimation->GetDuration());

	if (blendFactor <= 0.0f)
	{
		CalculateBoneTransform(BaseAnimation, &BaseAnimation->GetNodeData(), currentTimeBaseAnim, glm::mat4(1.0f));
	}
	else if (blendFactor >= 1.0f)
	{
		CalculateBoneTransform(LayeredAnimation, &LayeredAnimation->GetNodeData(), currentTimeLayeredAnim, glm::mat4(1.0f));
	}
	else
	{
		CalculateBlendedBoneTransform(BaseAnimation, &BaseAnimation->GetNodeData(), LayeredAnimation, &LayeredAnimation->GetNodeData(),
			currentTimeBaseAnim, currentTimeLayeredAnim, glm::mat4(1.0f), blendFactor);
	}
}

void FUSIONCORE::Animator::CalculateBlendedBoneTransform(Animation* pAnimationBase, const BoneNodeData* node, 
	Animation* pAnimationLayer, const BoneNodeData* nodeLayered, const float currentTimeBase, const float currentTimeLayered,
	const glm::mat4& parentTransform, const float blendFactor)
{
	const std::string& nodeName = node->name;

	glm::mat4 BaseNodeTransform = node->transformation;
	Bone* bone = pAnimationBase->FindBone(nodeName);
	if (bone)
	{
		bone->Update(currentTimeBase);
		BaseNodeTransform = bone->GetLocalTransformation();
	}

	glm::mat4 LayeredNodeTransform = nodeLayered->transformation;
	bone = pAnimationLayer->FindBone(nodeName);
	if (bone)
	{
		bone->Update(currentTimeLayered);
		LayeredNodeTransform = bone->GetLocalTransformation();
	}

	const glm::quat rot0 = glm::quat_cast(BaseNodeTransform);
	const glm::quat rot1 = glm::quat_cast(LayeredNodeTransform);
	const glm::quat finalRot = glm::slerp(rot0, rot1, blendFactor);
	glm::mat4 blendedMat = glm::mat4_cast(finalRot);
	blendedMat[3] = (1.0f - blendFactor) * BaseNodeTransform[3] + LayeredNodeTransform[3] * blendFactor;

	glm::mat4 globalTransformation = parentTransform * blendedMat;

	const auto& BoneInfoMap = pAnimationBase->GetBoneIDMap();
	if (BoneInfoMap.find(nodeName) != BoneInfoMap.end())
	{
		const int index = BoneInfoMap.at(nodeName).id;
		const glm::mat4& Offset = BoneInfoMap.at(nodeName).OffsetMat;

		FinalBoneMatrices[index] = globalTransformation * Offset;
	}

	for (size_t i = 0; i < node->children.size(); i++)
	{
		CalculateBlendedBoneTransform(pAnimationBase, &node->children[i], pAnimationLayer, &nodeLayered->children[i], currentTimeBase, 
			                           currentTimeLayered, globalTransformation, blendFactor);
	}
}

void FUSIONCORE::Animator::PlayAnimation(Animation* Animation)
{
	this->CurrentAnimation = Animation;
	this->CurrentTime = 0.0f;
}

void FUSIONCORE::Animator::CalculateBoneTransform(const BoneNodeData* node, glm::mat4 parentTransform)
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

void FUSIONCORE::Animator::CalculateBoneTransform(Animation* animation, const BoneNodeData* node, const float currentTime, glm::mat4 parentTransform)
{
	std::string nodeName = node->name;
	glm::mat4 nodeTransform = node->transformation;

	Bone* Bone = animation->FindBone(nodeName);

	if (Bone)
	{
		Bone->Update(currentTime);
		nodeTransform = Bone->GetLocalTransformation();
	}

	glm::mat4 globalTransformation = parentTransform * nodeTransform;

	auto boneInfoMap = animation->GetBoneIDMap();
	if (boneInfoMap.find(nodeName) != boneInfoMap.end())
	{
		int index = boneInfoMap[nodeName].id;
		glm::mat4 offset = boneInfoMap[nodeName].OffsetMat;
		FinalBoneMatrices[index] = globalTransformation * offset;
	}

	for (int i = 0; i < node->childrenCount; i++)
	{
		CalculateBoneTransform(animation,&node->children[i],currentTime, globalTransformation);
	}
}

std::vector<glm::mat4> FUSIONCORE::Animator::GetFinalBoneMatrices()
{
	return FinalBoneMatrices;
}

void FUSIONCORE::InitializeAnimationUniformBuffer()
{
	AnimationUniformBufferObject = std::make_unique<UBO>();
	AnimationUniformBufferObject->Bind();
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * MAX_BONE_COUNT, NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, AnimationUniformBufferObject->GetUBOID());
	BindUBONull();
}
