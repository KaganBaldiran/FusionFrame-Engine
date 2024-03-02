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

namespace FUSIONCORE
{
	struct BoneInfo
	{
		int id;
		glm::mat4 OffsetMat;
	};

	struct Vec3Hash {
		size_t operator()(const glm::vec3& v) const
		{
			size_t h1 = std::hash<float>()(v.x);
			size_t h2 = std::hash<float>()(v.y);
			size_t h3 = std::hash<float>()(v.z);

			size_t seed = 0;
			seed ^= h1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);

			return seed;
		}
	};

	struct PairVec3Hash {
		size_t operator()(const std::pair<glm::vec3, glm::vec3>& p) const
		{
			size_t h1 = Vec3Hash()(p.first);
			size_t h2 = Vec3Hash()(p.second);

			size_t seed = 0;
			seed ^= h1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);

			return seed;
		}
	};

	class Mesh
	{
	private:
		Buffer3D ObjectBuffer;
		std::vector<Texture2D> textures;
		std::vector<unsigned int> indices;
		std::vector<std::shared_ptr<Vertex>> vertices;
		std::vector<std::shared_ptr<HalfEdge>> HalfEdges;

		//Internal use
		std::unordered_map<std::pair<glm::vec3, glm::vec3>, int, PairVec3Hash> HalfEdgeMap;

	public:

		Mesh(std::vector<std::shared_ptr<Vertex>>& vertices_i, std::vector<unsigned int>& indices_i , std::vector<Texture2D>& textures_i);
		Mesh(std::vector<std::shared_ptr<Vertex>>& vertices_i, std::vector<unsigned int>& indices_i , std::vector<std::shared_ptr<Face>>& Faces, std::vector<Texture2D>& textures_i);
		void Draw(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations);
		void Draw(Camera3D& camera, Shader& shader , Material material, std::function<void()>& ShaderPreperations);
		void Draw(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations , CubeMap& cubeMap,Material material, float EnvironmentAmbientAmount = 0.2f);
		void DrawDeferred(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, CubeMap& cubeMap, Material material, float EnvironmentAmbientAmount = 0.2f);

		void ConstructHalfEdges();

		void DrawImportedMaterial(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, CubeMap& cubeMap,float EnvironmentAmbientAmount = 0.2f);
		void ConstructMesh();

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

		inline std::vector<std::shared_ptr<HalfEdge>>& GetHalfEdges() { return this->HalfEdges; };
		inline std::vector<std::shared_ptr<Face>>& GetFaces() { return this->Faces; };
		inline std::vector<unsigned int>& GetIndices() { return this->indices; };
		inline std::vector<std::shared_ptr<Vertex>>& GetVertices() { return vertices; };
		inline std::unordered_map<std::pair<glm::vec3, glm::vec3>, int, PairVec3Hash>& GetEdgeHashMap() { return this->HalfEdgeMap; };

		std::vector<std::shared_ptr<Face>> Faces;
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


