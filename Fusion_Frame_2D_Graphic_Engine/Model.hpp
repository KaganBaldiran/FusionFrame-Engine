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

namespace FUSIONOPENGL
{

    class Model
    {
    public:
        Model(std::string const& filePath)
        {
            this->loadModel(filePath);
            static unsigned int counter = 0;
            this->ModelID = counter;
            counter++;
        }

        unsigned int GetModelID() { return this->ModelID; };
        void Draw(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations);
        WorldTransform& GetTransformation() { return this->transformation; };
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
        WorldTransform transformation;
        unsigned int ModelID;
        std::string directory;
        
        inline void loadModel(std::string const& path)
        {
            
            Assimp::Importer importer;
            const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
            
            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
            {
                std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
                return;
            }
            
            directory = path.substr(0, path.find_last_of('/'));

            
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
                    vertex.TexCoords = glm::vec2(0.0f, 0.0f);

                vertices.push_back(vertex);
            }

            for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                
                for (unsigned int j = 0; j < face.mNumIndices; j++)
                    indices.push_back(face.mIndices[j]);
            }
            
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

    };
}


