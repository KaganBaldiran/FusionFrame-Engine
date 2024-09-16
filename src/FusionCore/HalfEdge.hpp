#pragma once
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "Object.hpp"
#include "../FusionUtility/FusionDLLExport.h"
#define MAX_BONE_INFLUENCE 4

namespace FUSIONCORE
{
	class HalfEdge;
	class Face;
	struct Vertex;

	struct FUSIONFRAME_EXPORT Vertex {
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
		glm::vec3 Tangent;
		glm::vec3 Bitangent;

		int m_BoneIDs[MAX_BONE_INFLUENCE];
		float m_Weights[MAX_BONE_INFLUENCE];
		HalfEdge* halfEdge = nullptr;

		bool operator==(const Vertex& other) const
		{
			return Position.x == other.Position.x &&
				Position.y == other.Position.y &&
				Position.z == other.Position.z;
		}
	};

	struct FUSIONFRAME_EXPORT VertexHash {
		size_t operator()(const Vertex& v) const
		{
			size_t h1 = std::hash<float>()(v.Position.x);
			size_t h2 = std::hash<float>()(v.Position.y);
			size_t h3 = std::hash<float>()(v.Position.z);

			size_t seed = 0;
			seed ^= h1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);

			return seed;
		}
	};

	FUSIONFRAME_EXPORT_FUNCTION glm::vec3 TranslateVertex(glm::mat4 Matrix, glm::vec3 VertexPos);
	glm::vec3 TranslateNormal(glm::mat4 ModelMatrix, glm::vec3 Normal);
	FUSIONFRAME_EXPORT_FUNCTION glm::vec3 FindNormal(std::vector<Vertex> Vertices);
	FUSIONFRAME_EXPORT_FUNCTION glm::vec3 FindNormal(glm::mat4 ModelMatrix, std::vector<Vertex> Vertices);

	class FUSIONFRAME_EXPORT Face
	{
	public:
		inline std::vector<Vertex> GetVertices() { return Vertices; };
		inline glm::vec3 GetNormal() { return Normal; };

		HalfEdge* halfEdge;
		glm::vec3 Normal;
		int FaceVertexCount;
		std::vector<Vertex> Vertices;
		std::vector<unsigned int> Indices;
	};

	class FUSIONFRAME_EXPORT HalfEdge {
	public:
		HalfEdge()
		{
			this->BoundryEdge = false;
		}
		Vertex* StartingVertex;
		Vertex* EndingVertex;
		HalfEdge* NextHalfEdge;
		HalfEdge* PrevHalfEdge;
		HalfEdge* TwinHalfEdge;
		bool BoundryEdge;
		Face* face;

		glm::vec3 GetMiddlePoint();
		glm::vec3 GetEdgeVector();
		float GetEdgeLength();
	};

	FUSIONFRAME_EXPORT_FUNCTION glm::vec3 CalculateAverage(const glm::vec3& v1, const glm::vec3& v2);
	FUSIONFRAME_EXPORT_FUNCTION glm::vec2 CalculateAverage(const glm::vec2& uv1, const glm::vec2& uv2);
	FUSIONFRAME_EXPORT_FUNCTION glm::vec3 CalculateAverageNormalized(const glm::vec3& v1, const glm::vec3& v2);
	FUSIONFRAME_EXPORT_FUNCTION Vertex GetAveragedVertex(Vertex vertex1, Vertex vertex2);
}