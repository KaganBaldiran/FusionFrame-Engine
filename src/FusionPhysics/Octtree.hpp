#pragma once
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "../FusionCore/Object.hpp"
#include "../FusionCore/Model.hpp"
#include "Physics.hpp"
#include "unordered_map"
#include "../FusionUtility/FusionDLLExport.h"

namespace FUSIONPHYSICS
{
	inline int NodeIDiterator = 0;

	struct FUSIONFRAME_EXPORT Ivec3Hash
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

	class FUSIONFRAME_EXPORT ObjectBoundingBox
	{
	public:
		ObjectBoundingBox();
		void CompareVec3MinMax(glm::vec3 &v);

		glm::vec3 Min;
		glm::vec3 Max;
		FUSIONCORE::Object* Object;
	};

	class FUSIONFRAME_EXPORT QuadNode
	{
	public:
		std::vector<ObjectBoundingBox*> Objects;
		std::vector<std::shared_ptr<QuadNode>> ChildrenNode;
		glm::vec3 Center;
		glm::vec3 Size;
		int NodeID;
		unsigned int SubdivisionCount;

		QuadNode();
		QuadNode(int ID) : NodeID(ID)
		{};
	};

	FUSIONFRAME_EXPORT_FUNCTION void UpdateQuadTreeWorldPartitioning(FUSIONPHYSICS::QuadNode& HeadNode , std::vector<FUSIONCORE::Object*> &ObjectInstances, unsigned int SingleQuadObjectCountLimit = 2 , unsigned int SubdivisionLimit = 5);
	//Internal use
	FUSIONFRAME_EXPORT_FUNCTION void DisposeQuadNodes(FUSIONPHYSICS::QuadNode& HeadNode);
	//Internal use
	FUSIONFRAME_EXPORT_FUNCTION void SubdivideQuadNode(FUSIONPHYSICS::QuadNode& Node, std::deque<QuadNode*>& NodesToProcess);
	//Internal use
	FUSIONFRAME_EXPORT_FUNCTION void CalculateInitialQuadTreeGridSize(std::vector<ObjectBoundingBox> &BoundingBoxes , std::vector<FUSIONCORE::Object*> &ObjectInstances,glm::vec3& WorldSizeMin,glm::vec3& WorldSizeMax);
	FUSIONFRAME_EXPORT_FUNCTION FUSIONCORE::WorldTransform NodeToWorldTransform(FUSIONPHYSICS::QuadNode& Node);
	//Be aware that visualizing quad trees can be quite costly and demanding since the tree is dynamically changing.
	FUSIONFRAME_EXPORT_FUNCTION void VisualizeQuadTree(FUSIONPHYSICS::QuadNode& HeadNode, FUSIONCORE::Camera3D& Camera, FUSIONCORE::Shader& Shader, glm::vec3 NodeColor);
}