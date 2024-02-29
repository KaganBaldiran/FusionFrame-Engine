#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "Buffer.h"
#include "Camera.h"
#include "Texture.h"
#include <functional>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Shader.h"
#include "Material.hpp"
#include <queue>
#include <memory>
#include <array>
#include "Cubemap.h"
#include "Transformation.hpp"
#include "HalfEdge.hpp"
#include "unordered_map"

namespace FUSIONOPENGL
{
	

	struct BoneInfo
	{
		int id;
		glm::mat4 OffsetMat;
	};

	class Mesh3D
	{
	public:
		Mesh3D(std::vector<FUSIONOPENGL::Vertex>& vertices_i, std::vector<unsigned int>& indices_i , std::vector<Texture2D>& textures_i);
		Mesh3D(std::vector<FUSIONOPENGL::Vertex>& vertices_i, std::vector<unsigned int>& indices_i , std::vector<Face>& Faces, std::vector<Texture2D>& textures_i);
		void Draw(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations);
		void Draw(Camera3D& camera, Shader& shader , Material material, std::function<void()>& ShaderPreperations);
		void Draw(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations , CubeMap& cubeMap,Material material, float EnvironmentAmbientAmount = 0.2f);
		void DrawDeferred(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, CubeMap& cubeMap, Material material, float EnvironmentAmbientAmount = 0.2f);

		void ConstructHalfEdges();

		void DrawImportedMaterial(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, CubeMap& cubeMap,float EnvironmentAmbientAmount = 0.2f);
		void ConstructMesh();
		std::vector<Vertex>& GetVertexArray() { return vertices; };

		unsigned int VAO;
		Material ImportedMaterial;
		std::string MeshName;

		void Clean()
		{
			ObjectBuffer.clean();

			for (auto it = ImportedMaterial.GetTextureMaps().begin(); it != ImportedMaterial.GetTextureMaps().end(); ++it)
			{
				it->second->Clear();
			}			
		}

	private:
		unsigned int VBO, EBO;

		Buffer3D ObjectBuffer;

		std::vector<Texture2D> textures;
		std::vector<unsigned int> indices;
		std::vector<Vertex> vertices;
		std::vector<HalfEdge> HalfEdges;
		std::vector<Face> Faces;

		struct PairHash
		{
			size_t operator()(const std::pair<int, int>& p) const
			{
				return std::hash<int>{}(p.second) ^ std::hash<int>{}(p.first);
			}
		};
		std::unordered_map<std::pair<int, int>, int, PairHash> EdgeMap;
	};

	class TextureObj
	{
	public:

		TextureObj();
		~TextureObj();

		void Draw(Camera2D& camera, GLuint shader, Texture2D& texture);
		void Draw(Camera3D& camera, GLuint shader, Texture2D& texture, std::function<void()> ShaderPreperations);

		WorldTransform* GetTransformation() { return &this->transformation; };

	protected:
		Buffer ObjectBuffer;
		WorldTransform transformation;

	};
}


