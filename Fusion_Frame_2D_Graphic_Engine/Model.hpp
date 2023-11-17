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
		Model(std::string filePath);
		unsigned int GetModelID() { return this->ModelID; };
		void Draw(Camera3D& camera, GLuint shader, std::function<void()> ShaderPreperations);
		WorldTransform& GetTransformation() { return this->transformation; };
		std::vector<Mesh3D> Meshes;

	private:
		void LoadModel(std::string pathh);
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh3D ProcessMesh(aiMesh* mesh, const aiScene* scene);

		std::string path;
		std::vector<Texture2D> Textures;
		WorldTransform transformation;

		unsigned int ModelID;
	};
}


