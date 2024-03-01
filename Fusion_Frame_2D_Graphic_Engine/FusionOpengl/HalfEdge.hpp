#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "Object.hpp"
#define MAX_BONE_INFLUENCE 4

namespace FUSIONOPENGL
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
	};

	inline glm::vec3 TranslateVertex(glm::mat4 Matrix, glm::vec3 VertexPos)
	{
		glm::vec4 transformed = Matrix * glm::vec4(VertexPos, 1.0f);
		return glm::vec3(transformed.x, transformed.y, transformed.z);
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

		std::shared_ptr<HalfEdge> halfEdge;
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
		std::shared_ptr<HalfEdge> NextHalfEdge;
		std::shared_ptr<HalfEdge> PrevHalfEdge;
		std::shared_ptr<HalfEdge> TwinHalfEdge;
		bool BoundryEdge;
		Face* Face;

		glm::vec3 GetMiddlePoint();
		glm::vec3 GetEdgeVector();
		float GetEdgeLength();
	};


}