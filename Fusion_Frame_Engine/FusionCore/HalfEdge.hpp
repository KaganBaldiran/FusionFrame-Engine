#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "Object.hpp"
#define MAX_BONE_INFLUENCE 4

namespace FUSIONCORE
{
	class HalfEdge;
	class Face;
	struct Vertex;

	struct Vertex {
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
		glm::vec3 Tangent;
		glm::vec3 Bitangent;

		int m_BoneIDs[MAX_BONE_INFLUENCE];
		float m_Weights[MAX_BONE_INFLUENCE];
		HalfEdge* halfEdge;

		bool operator==(const Vertex &other) const
		{
			return Position.x == other.Position.x &&
				   Position.y == other.Position.y &&
				   Position.z == other.Position.z;
		}
	};

	struct VertexHash {
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

	inline glm::vec3 TranslateVertex(glm::mat4 Matrix, glm::vec3 VertexPos)
	{
		glm::vec4 transformed = Matrix * glm::vec4(VertexPos, 1.0f);
		return glm::vec3(transformed);
	}

	inline glm::vec3 FindNormal(std::vector<Vertex> Vertices)
	{
		glm::vec3 Normal = glm::cross((Vertices[1].Position - Vertices[0].Position), (Vertices[2].Position - Vertices[0].Position));
		if (glm::length(Normal) < glm::epsilon<float>())
		{
			LOG_ERR("Error: Attempting to normalize a zero-length vector.");
		}
		else {
			Normal = glm::normalize(glm::abs(Normal));
		}
		return Normal;
	}

	inline glm::vec3 FindNormal(glm::mat4 ModelMatrix , std::vector<Vertex> Vertices)
	{
		glm::vec3 Normal = glm::cross((TranslateVertex(ModelMatrix, Vertices[1].Position) - TranslateVertex(ModelMatrix, Vertices[0].Position)), (TranslateVertex(ModelMatrix, Vertices[2].Position) - TranslateVertex(ModelMatrix, Vertices[0].Position)));
		if (glm::length(Normal) < glm::epsilon<float>())
		{
			LOG_ERR("Error: Attempting to normalize a zero-length vector.");
		}
		else {
			Normal = glm::normalize(glm::abs(Normal));
		}
		return Normal;
	}

	class Face
	{
	public:
		inline std::vector<Vertex> GetVertices() { return Vertices; };
		inline glm::vec3 GetNormal() { return Normal; };

		HalfEdge *halfEdge;
		glm::vec3 Normal;
		int FaceVertexCount;
		std::vector<Vertex> Vertices;
		std::vector<unsigned int> Indices;		
	};

	class HalfEdge {
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
		Face* Face;

		glm::vec3 GetMiddlePoint();
		glm::vec3 GetEdgeVector();
		float GetEdgeLength();
	};

	glm::vec3 CalculateAverage(const glm::vec3& v1, const glm::vec3& v2);
	glm::vec2 CalculateAverage(const glm::vec2& uv1, const glm::vec2& uv2);
	glm::vec3 CalculateAverageNormalized(const glm::vec3& v1, const glm::vec3& v2);
	Vertex GetAveragedVertex(Vertex vertex1 , Vertex vertex2);
}