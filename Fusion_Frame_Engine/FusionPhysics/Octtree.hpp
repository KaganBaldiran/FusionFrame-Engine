#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "../FusionCore/Object.hpp"
#include "../FusionCore/Model.hpp"
#include "Physics.hpp"
#include "unordered_map"

namespace FUSIONPHYSICS
{
	extern std::vector<FUSIONCORE::Object*> ObjectInstances;
	inline int NodeIDiterator = 0;

	struct Ivec3Hash
	{
		size_t operator()(const glm::ivec3& v) const
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

	class ObjectBoundingBox
	{
	public:
		ObjectBoundingBox()
		{
			Vertices.reserve(8);
			Min.x = std::numeric_limits<float>::max();
			Max.x = std::numeric_limits<float>::lowest();
			Min.y = std::numeric_limits<float>::max();
			Max.y = std::numeric_limits<float>::lowest();
			Min.z = std::numeric_limits<float>::max();
			Max.z = std::numeric_limits<float>::lowest();
		};

		void CompareVec3MinMax(glm::vec3 v);

		std::vector<glm::vec3> Vertices;
		glm::vec3 Min;
		glm::vec3 Max;
		FUSIONCORE::Object* Object;
	};

	class QuadNode
	{
	public:
		std::vector<ObjectBoundingBox*> Objects;
		std::vector<std::shared_ptr<QuadNode>> ChildrenNode;
		glm::vec3 Center;
		glm::vec3 Size;
		int NodeID;

		QuadNode()
		{
			NodeID = NodeIDiterator;
			NodeIDiterator++;
		}

		QuadNode(int ID) : NodeID(ID)
		{};
	};

	void UpdateQuadTreeWorldPartitioning(FUSIONPHYSICS::QuadNode& HeadNode);
	//Internal use
	void DisposeQuadNodes(FUSIONPHYSICS::QuadNode& HeadNode);
	//Internal use
	void SubdivideQuadNode(FUSIONPHYSICS::QuadNode& Node, std::deque<QuadNode*>& NodesToProcess);
	std::pair<glm::vec3, glm::vec3> GetGridSize();
	//Internal use
	void CalculateInitialQuadTreeGridSize(std::vector<ObjectBoundingBox> &BoundingBoxes);
}