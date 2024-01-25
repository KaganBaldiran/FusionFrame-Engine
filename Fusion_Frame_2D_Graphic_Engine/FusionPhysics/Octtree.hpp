#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "../FusionOpengl/Object.hpp"
#include "Physics.hpp"

namespace FUSIONPHYSICS
{
	extern std::vector<CollisionBox*> ObjectInstances;

	class QuadNode
	{
	public:
		std::vector<FUSIONOPENGL::Object*> Objects;
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

}