#pragma once
#include <glew.h>
#include <glfw3.h>
#include "Log.h"
#include "VectorMath.h"
#include "Buffer.h"
#include "Camera.h"
#include "Texture.h"
#include <functional>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include "Mesh.h"
#include <functional>
#include <map>
#include "Object.hpp"

namespace FUSIONOPENGL
{
   
    class Model : public Object
    {
    public:
        Model(std::string const& filePath)
        {
            this->loadModel(filePath);
            FindGlobalMeshScales();
            static unsigned int counter = 0;
            this->ModelID = counter;
            counter++;
        }

        unsigned int GetModelID() { return this->ModelID; };
        void Draw(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations);
        void Draw(Camera3D& camera, Shader& shader, Material material, std::function<void()>& ShaderPreperations);
        void FindGlobalMeshScales();
        
        std::vector<Mesh3D> Meshes;
        std::vector<Texture2D> textures_loaded;

        ~Model()
        {
            for (size_t i = 0; i < this->Meshes.size(); i++)
            {
                Meshes[i].Clean();
                LOG_INF("Model" << this->ModelID << " buffers cleaned!");
            }
        }

    private:

        std::string path;
        std::vector<Texture2D> Textures;
        unsigned int ModelID;
        std::string directory;
        glm::vec3 originpoint;
        glm::vec3 dynamic_origin;

        std::map<std::string, BoneInfo> Bones;
        int boneCounter = 0;

        inline void SetVertexBoneDataDefault(Vertex& vertex)
        {
            for (size_t i = 0; i < MAX_BONE_INFLUENCE; i++)
            {
                vertex.m_BoneIDs[i] = -1;
                vertex.m_Weights[i] = 0.0f;
            }
        }
        
        inline void loadModel(std::string const& path)
        {
            
            Assimp::Importer importer;
            const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices);
            
            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
            {
                std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
                return;
            }
            
            directory = path.substr(0, path.find_last_of('/'));

            std::vector<glm::vec3> originPoints;
            for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
                aiMesh* mesh = scene->mMeshes[i];


                float minX = mesh->mVertices[0].x, minY = mesh->mVertices[0].y, minZ = mesh->mVertices[0].z;
                float maxX = mesh->mVertices[0].x, maxY = mesh->mVertices[0].y, maxZ = mesh->mVertices[0].z;
                for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
                    if (mesh->mVertices[j].x < minX) minX = mesh->mVertices[j].x;
                    if (mesh->mVertices[j].y < minY) minY = mesh->mVertices[j].y;
                    if (mesh->mVertices[j].z < minZ) minZ = mesh->mVertices[j].z;
                    if (mesh->mVertices[j].x > maxX) maxX = mesh->mVertices[j].x;
                    if (mesh->mVertices[j].y > maxY) maxY = mesh->mVertices[j].y;
                    if (mesh->mVertices[j].z > maxZ) maxZ = mesh->mVertices[j].z;
                }
                float centerX = (minX + maxX) / 2.0f;
                float centerY = (minY + maxY) / 2.0f;
                float centerZ = (minZ + maxZ) / 2.0f;

                originPoints.push_back(glm::vec3(centerX, centerY, centerZ));
            }

            
            float overallCenterX = 0.0f, overallCenterY = 0.0f, overallCenterZ = 0.0f;
            for (unsigned int i = 0; i < originPoints.size(); i++) {
                overallCenterX += originPoints[i].x;
                overallCenterY += originPoints[i].y;
                overallCenterZ += originPoints[i].z;
            }
            overallCenterX /= originPoints.size();
            overallCenterY /= originPoints.size();
            overallCenterZ /= originPoints.size();

            originpoint = { overallCenterX,overallCenterY,overallCenterZ };
            transformation.OriginPoint = &this->originpoint;
            transformation.Position = glm::vec3(originpoint.x,originpoint.y,originpoint.z);
            dynamic_origin = glm::vec3(overallCenterX, overallCenterY, overallCenterZ);

            std::cout << "Overall origin point: (" << overallCenterX << ", " << overallCenterY << ", " << overallCenterZ << ")" << std::endl;
            
            processNode(scene->mRootNode, scene);
        }

        
        inline void processNode(aiNode* node, const aiScene* scene)
        {
            
            for (unsigned int i = 0; i < node->mNumMeshes; i++)
            {
                
                aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
                Meshes.push_back(processMesh(mesh, scene));
            }
           
            for (unsigned int i = 0; i < node->mNumChildren; i++)
            {
                processNode(node->mChildren[i], scene);
            }

        }

        inline Mesh3D processMesh(aiMesh* mesh, const aiScene* scene)
        {
            
            std::vector<Vertex> vertices;
            std::vector<unsigned int> indices;
            std::vector<Texture2D> textures;

            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
            {
                Vertex vertex;
                glm::vec3 vector; 

                SetVertexBoneDataDefault(vertex);
                
                vector.x = mesh->mVertices[i].x;
                vector.y = mesh->mVertices[i].y;
                vector.z = mesh->mVertices[i].z;
                vertex.Position = vector;
                
                if (mesh->HasNormals())
                {
                    vector.x = mesh->mNormals[i].x;
                    vector.y = mesh->mNormals[i].y;
                    vector.z = mesh->mNormals[i].z;
                    vertex.Normal = vector;
                }
                
                if (mesh->mTextureCoords[0]) 
                {
                    glm::vec2 vec;
                    
                    vec.x = mesh->mTextureCoords[0][i].x;
                    vec.y = mesh->mTextureCoords[0][i].y;
                    vertex.TexCoords = vec;
                    
                    vector.x = mesh->mTangents[i].x;
                    vector.y = mesh->mTangents[i].y;
                    vector.z = mesh->mTangents[i].z;
                    vertex.Tangent = vector;
                   
                    vector.x = mesh->mBitangents[i].x;
                    vector.y = mesh->mBitangents[i].y;
                    vector.z = mesh->mBitangents[i].z;
                    vertex.Bitangent = vector;
                }
                else
                {
                    vertex.TexCoords = glm::vec2(0.0f, 0.0f);
                }

                vertices.push_back(vertex);
            }
            
            for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                
                for (unsigned int j = 0; j < face.mNumIndices; j++)
                    indices.push_back(face.mIndices[j]);
            }
           

            ExtractBones(vertices, mesh, scene);

            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            
           
            std::vector<Texture2D> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            
            std::vector<Texture2D> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
            
            std::vector<Texture2D> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
            textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
            
            std::vector<Texture2D> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
            textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

            
            return Mesh3D(vertices, indices, textures);
        }

        inline void ExtractBones(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
        {
            for (size_t i = 0; i < mesh->mNumBones; i++)
            {
                int boneID = -1;
                std::string boneName = mesh->mBones[i]->mName.C_Str();
                if (Bones.find(boneName) == Bones.end())
                {
                    BoneInfo newBone;
                    newBone.id = boneCounter;
                    newBone.OffsetMat = this->ConvertMatrixToGLMFormat(mesh->mBones[i]->mOffsetMatrix);
                    Bones[boneName] = newBone;
                    boneID = boneCounter;
                    boneCounter++;
                }
                else
                {
                    boneID = Bones[boneName].id;
                }
                assert(boneID != -1);
                auto weights = mesh->mBones[i]->mWeights;
                int NumberWeights = mesh->mBones[i]->mNumWeights;

                for (size_t x = 0; x < NumberWeights; x++)
                {
                    int vertexID = weights[x].mVertexId;
                    float weight = weights[x].mWeight;

                    assert(vertexID <= vertices.size());

                    auto &vertextemp = vertices[vertexID];
                    for (size_t e = 0; e < MAX_BONE_INFLUENCE; e++)
                    {
                        if (vertextemp.m_BoneIDs[e] < 0)
                        {
                            vertextemp.m_Weights[e] = weight;
                            vertextemp.m_BoneIDs[e] = boneID;
                            break;
                        }
                    }
                }
            
            }
        }
        
        inline std::vector<Texture2D> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
        {
            std::vector<Texture2D> textures;
            for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
            {
                aiString str;
                mat->GetTexture(type, i, &str);

                bool skip = false;
                for (unsigned int j = 0; j < textures_loaded.size(); j++)
                {
                    if (std::strcmp(textures_loaded[j].GetFilePath().c_str(), str.C_Str()) == 0)
                    {
                        textures.push_back(textures_loaded[j]);
                        skip = true; 
                        break;
                    }
                }
                if (!skip)
                {   
                    Texture2D texture(str.C_Str(), GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, true);
                    texture.PbrMapType = type;

                    textures.push_back(texture);
                    textures_loaded.push_back(texture);  
                }
            }
            return textures;
        }

        static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
        {
            glm::mat4 to;
            
            to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
            to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
            to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
            to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
            return to;
        }

    };
}


