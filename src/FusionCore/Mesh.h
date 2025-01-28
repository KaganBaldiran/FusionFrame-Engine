#pragma once
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "Buffer.h"
#include "Camera.h"
#include "Texture.h"
#include <functional>
#include "Shader.h"
#include "Material.hpp"
#include <queue>
#include <memory>
#include <array>
#include "Cubemap.h"
#include "Transformation.hpp"
#include "HalfEdge.hpp"
#include "unordered_map"
#include "../FusionUtility/FusionDLLExport.h"

namespace FUSIONCORE
{
	struct FUSIONFRAME_EXPORT BoneInfo
	{
		int id;
		glm::mat4 OffsetMat;
	};

	struct FUSIONFRAME_EXPORT Vec3Hash {
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

	inline void FUSIONFRAME_EXPORT HashCombine(std::size_t& seed, std::size_t value)
	{
		seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	struct FUSIONFRAME_EXPORT PairVec3Hash {
		size_t operator()(const std::pair<glm::vec3, glm::vec3>& p) const
		{
			size_t h1 = Vec3Hash()(p.first);
			size_t h2 = Vec3Hash()(p.second);

			size_t seed = 0;
			HashCombine(seed, h1);
			HashCombine(seed, h2);

			return seed;
		}
	};

	class FUSIONFRAME_EXPORT Mesh
	{
	private:
		IndirectCommandBuffer IndirectCommandDataBuffer;
		bool IndirectBufferFilled;

		Buffer3D ObjectBuffer;
		std::vector<std::shared_ptr<Texture2D>> textures;
		std::vector<unsigned int> indices;
		std::vector<std::shared_ptr<Vertex>> vertices;
		std::vector<std::shared_ptr<HalfEdge>> HalfEdges;

		//Internal use
		std::unordered_map<std::pair<glm::vec3, glm::vec3>, int, PairVec3Hash> HalfEdgeMap;
		std::unordered_map<glm::vec3, std::vector<Vertex*>, Vec3Hash> DuplicateVertexMap;

		glm::vec3 InitialMeshMin;
		glm::vec3 InitialMeshMax;
		glm::vec3 OriginPoint;

		void BookKeepDuplicateVertices(Vertex* vertex);

	public:

		Mesh() = default;
		Mesh(std::vector<std::shared_ptr<Vertex>>& vertices_i, std::vector<unsigned int>& indices_i, std::vector<std::shared_ptr<Texture2D>>& textures_i);
		Mesh(std::vector<std::shared_ptr<Vertex>>& vertices_i, std::vector<unsigned int>& indices_i, std::vector<std::shared_ptr<Face>>& Faces, std::vector<std::shared_ptr<Texture2D>>& textures_i);
		Mesh(std::vector<std::shared_ptr<Vertex>>& vertices_i, std::vector<unsigned int>& indices_i, std::vector<std::shared_ptr<Face>>& Faces, FUSIONCORE::Material& InputMaterial);
		void Draw(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations);
		void Draw(Camera3D& camera, Shader& shader, Material material, std::function<void()>& ShaderPreperations);
		void Draw(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, CubeMap& cubeMap, Material material, float EnvironmentAmbientAmount = 0.2f);
		void DrawInstanced(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, CubeMap& cubeMap, Material material, float EnvironmentAmbientAmount, size_t PrimCount);
		void DrawDeferredInstanced(Camera3D& camera, Shader& shader, std::function<void()> ShaderPreperations, size_t PrimCount);
		void DrawDeferred(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, Material material);
		void DrawDeferredImportedMaterial(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations);

		void DrawDeferredIndirect(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, Material material);

		void SetIndirectCommandBuffer(unsigned int InstanceCount, unsigned int BaseVertex, unsigned int BaseIndex, unsigned int BaseInstance);
		void ConstructHalfEdges();

		void DrawImportedMaterial(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, CubeMap& cubeMap, float EnvironmentAmbientAmount = 0.2f);
		void ConstructMesh();

		Material ImportedMaterial;
		std::string MeshName;

		void Clean();

		inline std::vector<std::shared_ptr<HalfEdge>>& GetHalfEdges() { return this->HalfEdges; };
		inline std::vector<std::shared_ptr<Face>>& GetFaces() { return this->Faces; };
		inline std::vector<unsigned int>& GetIndices() { return this->indices; };
		inline std::vector<std::shared_ptr<Vertex>>& GetVertices() { return vertices; };
		inline std::unordered_map<std::pair<glm::vec3, glm::vec3>, int, PairVec3Hash>& GetEdgeHashMap() { return this->HalfEdgeMap; };
		inline std::unordered_map<glm::vec3, std::vector<Vertex*>, Vec3Hash>& GetDuplicateVertexMap() { return this->DuplicateVertexMap; };
		inline Buffer3D& GetMeshBuffer() { return this->ObjectBuffer; };
		inline glm::vec3 GetInitialMeshMin() { return this->InitialMeshMin; };
		inline glm::vec3 GetInitialMeshMax() { return this->InitialMeshMax; };
		inline void SetInitialMeshMin(glm::vec3 min) { this->InitialMeshMin = min; };
		inline void SetInitialMeshMax(glm::vec3 max) { this->InitialMeshMax = max; };
		inline glm::vec3 GetMeshOriginPoint() { return this->OriginPoint; };
		inline void SetMeshOriginPoint(glm::vec3 Origin) { this->OriginPoint = Origin; };

		std::vector<std::shared_ptr<Face>> Faces;

		//Copy other meshes data and construct the mesh again. Imported material textures are also copied and share different ids.
		void CopyMesh(Mesh& other);
	};
}


