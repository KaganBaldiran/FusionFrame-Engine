#include "Octtree.hpp"
#include <map>
#include <deque>

glm::vec3 WorldSizeMin;
glm::vec3 WorldSizeMax;
std::vector<FUSIONPHYSICS::ObjectBoundingBox> BoundingBoxes;

namespace FUSIONPHYSICS
{
	std::vector<FUSIONCORE::Object*> ObjectInstances;
}

void FUSIONPHYSICS::ObjectBoundingBox::CompareVec3MinMax(glm::vec3 v)
{
	Min.x = glm::min(v.x, Min.x);
	Min.y = glm::min(v.y, Min.y);
	Min.z = glm::min(v.z, Min.z);

	Max.x = glm::max(v.x, Max.x);
	Max.y = glm::max(v.y, Max.y);
	Max.z = glm::max(v.z, Max.z);
}

int FastStringToInt(const char* str)
{
	int val = 0;
	while (*str)
	{
		val = val * 10 + (*str++ - '0');
	}
	return val;
}

void FUSIONPHYSICS::CalculateInitialQuadTreeGridSize(std::vector<ObjectBoundingBox>& BoundingBoxes)
{
	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::lowest();
	float minZ = std::numeric_limits<float>::max();
	float maxZ = std::numeric_limits<float>::lowest();

	for (size_t i = 0; i < ObjectInstances.size(); i++)
	{
		ObjectBoundingBox BoundingBox;
		BoundingBox.Object = ObjectInstances[i];
		FUSIONCORE::WorldTransform& objectTransformation = ObjectInstances[i]->GetTransformation();
		auto& ModelScales = objectTransformation.InitialObjectScales;
		auto ModelMatrix = objectTransformation.GetModelMat4();
		auto& ModelPosition = *objectTransformation.OriginPoint;
		auto HalfScales = (ModelScales * 0.5f);
		for (size_t x = 0; x < 2; x++)
		{
			for (size_t y = 0; y < 2; y++)
			{
				for (size_t z = 0; z < 2; z++)
				{
					glm::vec3 Vertex(ModelPosition.x + ((2.0f * x - 1.0f) * HalfScales.x),
						             ModelPosition.y + ((2.0f * y - 1.0f) * HalfScales.y),
						             ModelPosition.z + ((2.0f * z - 1.0f) * HalfScales.z));

					Vertex = glm::vec3(ModelMatrix * glm::vec4(Vertex, 1.0f));
					BoundingBox.CompareVec3MinMax(Vertex);
					BoundingBox.Vertices.push_back(Vertex);
				}
			}
		}
		BoundingBoxes.push_back(BoundingBox);
	}

	for (const auto& BoundingBoxTemp : BoundingBoxes)
	{
		const auto& BoxMin = BoundingBoxTemp.Min;
		const auto& BoxMax = BoundingBoxTemp.Max;

		minX = glm::min(BoxMin.x, minX);
		minY = glm::min(BoxMin.y, minY);
		minZ = glm::min(BoxMin.z, minZ);

		maxX = glm::max(BoxMax.x, maxX);
		maxY = glm::max(BoxMax.y, maxY);
		maxZ = glm::max(BoxMax.z, maxZ);
	}

	WorldSizeMin = glm::vec3(minX, minY, minZ);
	WorldSizeMax = glm::vec3(maxX, maxY, maxZ);
}

void FUSIONPHYSICS::SubdivideQuadNode(FUSIONPHYSICS::QuadNode& Node , std::deque<QuadNode*> &NodesToProcess)
{
	glm::vec3 QuarterSize = Node.Size * 0.25f;
	glm::vec3 HalfSize = Node.Size * 0.5f;
	glm::vec3 ParentCenter = Node.Center;
	for (size_t x = 0; x < 2; x++)
	{
		for (size_t z = 0; z < 2; z++)
		{
			QuadNode newChildNode;
			newChildNode.Center = { ParentCenter.x + (QuarterSize.x * (2.0f * x - 1.0f)),
									ParentCenter.y,
									ParentCenter.z + (QuarterSize.z * (2.0f * z - 1.0f))};
			newChildNode.Size = { HalfSize.x ,Node.Size.y,HalfSize.z};
			Node.ChildrenNode.push_back(std::make_shared<QuadNode>(newChildNode));
			NodesToProcess.push_back(Node.ChildrenNode.back().get());
		}
	}
}

std::pair<glm::vec3, glm::vec3> FUSIONPHYSICS::GetGridSize()
{
	return {WorldSizeMin,WorldSizeMax};
}

/*
bool IsObjectInsideNode(const FUSIONPHYSICS::ObjectBoundingBox& object, const FUSIONPHYSICS::QuadNode* node) {
    const auto& vertices = object.Vertices;
    const auto& nodeCenter = node->Center;
    const auto& nodeHalfSize = node->Size * 0.5f;

    const glm::vec3 nodeMin = nodeCenter - nodeHalfSize;
    const glm::vec3 nodeMax = nodeCenter + nodeHalfSize;

    for (const auto& vertex : vertices) 
	{
        if (vertex.x <= nodeMax.x && vertex.x >= nodeMin.x &&
			vertex.y <= nodeMax.y && vertex.y >= nodeMin.y &&
            vertex.z <= nodeMax.z && vertex.z >= nodeMin.z) {
            return true;
        }
    }
    return false;
}
*/

bool IsObjectInsideQuadNode(const FUSIONPHYSICS::ObjectBoundingBox& object, const FUSIONPHYSICS::QuadNode* node) {
	const auto& nodeCenter = node->Center;
	const auto& nodeHalfSize = node->Size * 0.5f;

	const auto& MinObject = object.Min;
	const auto& MaxObject = object.Max;

	const glm::vec3 nodeMin = nodeCenter - nodeHalfSize;
	const glm::vec3 nodeMax = nodeCenter + nodeHalfSize;

	bool partiallyInside =
		MaxObject.x >= nodeMin.x && MinObject.x <= nodeMax.x &&
		MaxObject.y >= nodeMin.y && MinObject.y <= nodeMax.y &&
		MaxObject.z >= nodeMin.z && MinObject.z <= nodeMax.z;

	return partiallyInside;
}


void FUSIONPHYSICS::UpdateQuadTreeWorldPartitioning(FUSIONPHYSICS::QuadNode& HeadNode)
{
	DisposeQuadNodes(HeadNode);
	BoundingBoxes.clear();
	std::deque<QuadNode*> NodesToProcess;
	CalculateInitialQuadTreeGridSize(BoundingBoxes);

	HeadNode.Center = { (WorldSizeMin.x + WorldSizeMax.x) * 0.5f,
					    (WorldSizeMin.y + WorldSizeMax.y) * 0.5f,
					    (WorldSizeMin.z + WorldSizeMax.z) * 0.5f };
	HeadNode.Size = glm::abs(glm::vec3(WorldSizeMax.x - WorldSizeMin.x ,
					                   WorldSizeMax.y - WorldSizeMin.y ,
					                   WorldSizeMax.z - WorldSizeMin.z));
	HeadNode.Objects.reserve(BoundingBoxes.size());
	for (size_t i = 0; i < BoundingBoxes.size(); i++)
	{
		HeadNode.Objects.push_back(&BoundingBoxes[i]);
	}
	NodesToProcess.push_back(&HeadNode);

	while (!NodesToProcess.empty())
	{
		auto Node = std::move(NodesToProcess.back());
		NodesToProcess.pop_back();
		if (Node->Objects.size() > 2)
		{
			SubdivideQuadNode(*Node, NodesToProcess);
			for (size_t i = 0; i < Node->Objects.size(); i++)
			{
				std::vector<QuadNode*> AssociatedNodes;
				auto& CurrentObject = Node->Objects[i];

				for (size_t y = 0; y < Node->ChildrenNode.size(); y++)
				{
					auto ChildNode = Node->ChildrenNode[y].get();
					if (IsObjectInsideQuadNode(*CurrentObject, ChildNode))
					{
						ChildNode->Objects.push_back(CurrentObject);
						AssociatedNodes.push_back(ChildNode);
					}
				}
				CurrentObject->Object->SetAssociatedQuads(AssociatedNodes);
			}
			Node->Objects.clear();
		}
	}	
}

void FUSIONPHYSICS::DisposeQuadNodes(FUSIONPHYSICS::QuadNode& HeadNode)
{
	HeadNode.ChildrenNode.clear();
	NodeIDiterator = 1;
	HeadNode.Objects.clear();
}


