#include "Animation.hpp"

FUSIONCORE::Animation::Animation(const char* AnimationPath, Model* model)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(AnimationPath, aiProcess_Triangulate);
    assert(scene && scene->mRootNode);
    if (scene->HasAnimations())
    {
        auto animation = scene->mAnimations[0];
        this->Duration = animation->mDuration;
        this->TicksPerSecond = animation->mTicksPerSecond;
        this->ModelPointer = model;
        ReadHeirarchyData(rootnode , scene->mRootNode);
        ReadMissingBones(animation, *model);
    }
    else
    {
        LOG_ERR("File contains no animation!");
    }
}

FUSIONCORE::Bone* FUSIONCORE::Animation::FindBone(const std::string& name)
{
    auto iter = std::find_if(Bones.begin(), Bones.end(), 
        [&](FUSIONCORE::Bone& Bone)
        {
            return Bone.GetBoneName() == name;
        }
    );
    if (iter == Bones.end())
    {
        return nullptr;
    }
    else
    {
        return &(*iter);
    }
}

void FUSIONCORE::Animation::ReadMissingBones(const aiAnimation* animation, Model& model)
{
    int size = animation->mNumChannels;

    auto& boneInfoMapModel = model.GetBones();
    int& boneCount = model.GetBoneCounter();
    //LOG("ANIMATION CHANNEL SIZE: " << size);
    for (size_t i = 0; i < size; i++)
    {
        auto channel = animation->mChannels[i];
        std::string BoneName = channel->mNodeName.data;

        if (boneInfoMapModel.find(BoneName) == boneInfoMapModel.end())
        {
            boneInfoMapModel[BoneName].id = boneCount;
            boneCount++;
        }
        Bone newBone(channel->mNodeName.data, boneInfoMapModel[channel->mNodeName.data].id, channel);
        Bones.push_back(newBone);
    }

    this->BoneInfoMap = boneInfoMapModel;
}

void FUSIONCORE::Animation::ReadHeirarchyData(BoneNodeData& dest, const aiNode* src)
{
    assert(src);

    dest.name = src->mName.data;
    dest.transformation = ConvertMatrixToGLMFormat(src->mTransformation);
    dest.childrenCount = src->mNumChildren;

    for (int i = 0; i < src->mNumChildren; i++)
    {
        BoneNodeData newData;
        ReadHeirarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}
