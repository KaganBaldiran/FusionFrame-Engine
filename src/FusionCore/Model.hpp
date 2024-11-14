#pragma once
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "Buffer.h"
#include "Camera.h"
#include "Texture.h"
#include <functional>
#include <string>
#include "Mesh.h"
#include <map>
#include <set>
#include "Object.hpp"
#include "Cubemap.h"
#include <filesystem>
#include "../FusionUtility/FusionDLLExport.h"

struct aiScene;
struct aiMesh;
struct aiNode;
struct aiMaterial;
enum aiTextureType;

template <typename T>
class aiMatrix4x4t;
typedef float ai_real;
typedef aiMatrix4x4t<ai_real> aiMatrix4x4;

namespace FUSIONCORE
{
    //forward declaration of the omnishadow map class
    class OmniShadowMap;

    class FUSIONFRAME_EXPORT PreMeshData
    {
    public:
        std::vector<std::shared_ptr<Texture2D>> textures;
        std::vector<unsigned int> indices;
        std::vector<std::shared_ptr<Vertex>> vertices;

        PreMeshData(std::vector<std::shared_ptr<Texture2D>>& textures, std::vector<unsigned int>& indices, std::vector<std::shared_ptr<Vertex>>& vertices);
    };

    FUSIONFRAME_EXPORT_FUNCTION glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from);
    
     /*
     Represents a 3D model loaded from a file.

     The Model class represents a 3D model loaded from a file, providing functionality
     for rendering the model with different shaders, materials, and environment maps.
     It supports various rendering techniques such as deferred rendering, instancing,
     and indirect rendering. Additionally, it includes methods for processing model
     data, loading textures, extracting bones for animation, and handling asynchronous
     model loading.

     Example usage:
     // Load a model from a file
     Model myModel("path/to/model.obj");

     // Draw the model with a specific shader and camera
     myModel.Draw(camera, shader, ShaderPreparations);
     */
    class FUSIONFRAME_EXPORT Model : public Object
    {
    public:
        Model();
        Model(std::string const& filePath, bool Async = false, bool AnimationModel = false);
        Model(Model& Other);

        inline unsigned int GetModelID() { return this->ModelID; };
        void Draw(Camera3D& camera, Shader& shader, const std::function<void()>& ShaderPreperations);
        void Draw(Camera3D& camera, Shader& shader, Material material, const std::function<void()>& ShaderPreperations);
        void Draw(Camera3D& camera, Shader& shader, const std::function<void()>& ShaderPreperations, CubeMap& cubemap,Material material,float EnvironmentAmbientAmount = 0.2f);
        void Draw(Camera3D& camera, Shader& shader, const std::function<void()>& ShaderPreperations, CubeMap& cubemap, Material material, std::vector<OmniShadowMap*> ShadowMaps, float EnvironmentAmbientAmount = 0.2f);
        void Draw(Camera3D& camera, Shader& shader, const std::function<void()>& ShaderPreperations, CubeMap& cubemap, Material material, std::vector<OmniShadowMap*> ShadowMaps , std::vector<glm::mat4>& AnimationBoneMatrices, float EnvironmentAmbientAmount = 0.2f);
        void DrawInstanced(Camera3D& camera, Shader& shader, const std::function<void()>& ShaderPreperations, CubeMap& cubemap, Material material,VBO& InstanceDataVBO, size_t InstanceCount,std::vector<OmniShadowMap*> ShadowMaps = std::vector<OmniShadowMap*>(), float EnvironmentAmbientAmount = 0.2f);
        void DrawDeferredInstanced(Camera3D& camera, Shader& shader, const std::function<void()>& ShaderPreperations, Material material, VBO& InstanceDataVBO, size_t InstanceCount);
        void DrawDeferredInstanced(Camera3D& camera, Shader& shader, const std::function<void()>& ShaderPreperations, VBO& InstanceDataVBO, size_t InstanceCount);
        void DrawDeferredInstancedImportedMaterial(Camera3D& camera, Shader& shader, const std::function<void()>& ShaderPreperations, VBO& InstanceDataVBO, size_t InstanceCount);
        void DrawDeferred(Camera3D& camera, Shader& shader, const std::function<void()>& ShaderPreperations, Material material, std::vector<glm::mat4>& AnimationBoneMatrices);
        void DrawDeferred(Camera3D& camera, Shader& shader, const std::function<void()>& ShaderPreperations, Material material);
        void DrawDeferredIndirect(Camera3D& camera, Shader& shader, const std::function<void()>& ShaderPreperations, Material material);
        void DrawDeferredImportedMaterial(Camera3D& camera, Shader& shader, const std::function<void()>& ShaderPreperations);

        void Draw(Camera3D& camera, Shader& shader, std::vector<Material> materials, const std::function<void()>& ShaderPreperations,CubeMap& cubemap, float EnvironmentAmbientAmount = 0.2f);
        void Draw(Camera3D& camera, Shader& shader, std::vector<Material> materials, const std::function<void()>& ShaderPreperations, CubeMap& cubemap , std::vector<OmniShadowMap*> ShadowMaps, float EnvironmentAmbientAmount = 0.2f);
        void Draw(Camera3D& camera, Shader& shader, std::vector<Material> materials, const std::function<void()>& ShaderPreperations, CubeMap& cubemap, std::vector<OmniShadowMap*> ShadowMaps , std::vector<glm::mat4>& AnimationBoneMatrices, float EnvironmentAmbientAmount = 0.2f);
        void DrawImportedMaterial(Camera3D& camera, Shader& shader, const std::function<void()>& ShaderPreperations, CubeMap& cubemap, float EnvironmentAmbientAmount = 0.2f);

        void FindGlobalMeshScales();

        //Sets an indirect command buffer for the model which allows to use indirect rendering for mostly static models in terms of vertices and instance count.
        void SetIndirectCommandBuffer(unsigned int InstanceCount, unsigned int BaseVertex, unsigned int BaseIndex, unsigned int BaseInstance);

        //Binding an internal pointer of the given instance data VBO to render instanced shadows etc.
        //It's crucial to keep the original VBO intact;
        //Not needed to be called unless effects like shadows are desired. 
        void SetInstanced(VBO& InstanceDataVBO,size_t InstanceCount);
        //Binding material pointer to have an alpha map to render shadows
        //It's crucial to keep the original material and it's textures intact
        //No need to call unless material has an alpha map
        void SetAlphaMaterial(Material& material);

        std::vector<Mesh> Meshes;
        std::unordered_map<std::string,std::shared_ptr<Texture2D>> textures_loaded;
        std::vector<PreMeshData> PreMeshDatas;

        ~Model();
        void SetVertexBoneDataDefault(Vertex& vertex);

        void loadModel(std::string const& path , bool Async = false, bool AnimationModel = false);
        void processNode(aiNode* node, const aiScene* scene);
        void processMesh(aiMesh* mesh, const aiScene* scene);
        void ExtractBones(std::vector<std::shared_ptr<Vertex>>& vertices, aiMesh* mesh, const aiScene* scene);
        void LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName,const char* TextureKey, FUSIONCORE::Material& DestinationMaterial);

        PreMeshData processMeshAsync(aiMesh* mesh, const aiScene* scene);
        void processNodeAsync(aiNode* node, const aiScene* scene);
        void ConstructMeshes(std::vector<PreMeshData> PreMeshDatas);
        inline std::vector<glm::mat4>*& GetAnimationMatricesPointer() { return this->FinalAnimationMatrices; };

        inline std::map<std::string, BoneInfo>& GetBones() { return Bones; };
        inline int& GetBoneCounter() { return this->boneCounter; };
        inline const int& GetModelImportStateCode() { return this->ModelImportStateCode; };
        inline bool IsAnimationEnabled() { return this->AnimationEnabled; };
        inline std::string& GetModelName() { return this->ModelName; };
        inline const std::string& GetModelFilePath() { return this->path; };
        inline VBO* GetInstanceDataVBOpointer() { return this->InstanceDataVBO; };
        inline const size_t GetInstanceDataInstanceCount() { return this->InstanceCount; };
        
    private:
        std::string path;
        std::vector<std::shared_ptr<Texture2D>> Textures;
        unsigned int ModelID;
        std::string directory;
        glm::vec3 originpoint;
        glm::vec3 dynamic_origin;
        bool AnimationEnabled;
        std::vector<glm::mat4>* FinalAnimationMatrices;
        Material* AlphaMaterial;

        std::map<std::string, BoneInfo> Bones;
        int boneCounter = 0;

        aiScene* scene;
        std::string ModelName;

        int ModelImportStateCode = FF_INITIAL_CODE;
        VBO* InstanceDataVBO;
        size_t InstanceCount;

        void ProcessMeshMaterial(aiMaterial* material,FUSIONCORE::Material& TempMaterial);
    };

    FUSIONFRAME_EXPORT_FUNCTION std::vector<std::shared_ptr<FUSIONCORE::Model>> ImportMultipleModelsFromDirectory(const char* DirectoryFilePath, bool AnimationModel = false);
    FUSIONFRAME_EXPORT_FUNCTION Model* GetModel(unsigned int ModelID);
}


