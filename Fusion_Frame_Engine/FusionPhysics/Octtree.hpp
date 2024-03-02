#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "../FusionCore/Object.hpp"
#include "Physics.hpp"
#include "unordered_map"

namespace FUSIONPHYSICS
{
	extern std::vector<CollisionBox*> ObjectInstances;

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

	class QuadNode
	{
	public:
		std::vector<FUSIONCORE::Object*> Objects;
		std::vector<QuadNode> ChildrenNode;
		std::string NodeID;

		QuadNode(std::string ID) : NodeID(ID)
		{};
	};

	std::string LinearKeyToRelative(unsigned int LinearKey);
	int RelativeKeyToLinear(std::string &RelativerKey);
	void CalculateGridSize();
	void UpdateWorldBoundries(FUSIONPHYSICS::QuadNode& HeadNode);
	void DisposeNodes();
	void Subdivide(FUSIONPHYSICS::QuadNode& Node, std::map<std::string, QuadNode*>& SubdividedQuads);

	std::pair<glm::vec3, glm::vec3> GetGridSize();
	//std::unordered_map<glm::ivec3, int, Ivec3Hash> NodeMap;
}