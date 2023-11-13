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

namespace FUSIONOPENGL
{
	class Model
	{
	public:
		Model(const char* filePath);
		unsigned int GetModelID() { return this->ModelID; };
		~Model();
		WorldTransform* GetTransformation() { return &this->transformation; };

	private:
		void LoadModel(const char* filePath);
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh3D ProcessMesh(aiMesh* mesh, const aiScene* scene);

		std::string path;
		std::vector<Mesh> Meshes;
		std::vector<Texture2D> Textures;
		WorldTransform transformation;

		unsigned int ModelID;
	};
}


