#include "Octtree.hpp"
#include <map>
#include <deque>

glm::vec3 WorldSizeMin;
glm::vec3 WorldSizeMax;
std::vector<FUSIONPHYSICS::ObjectBoundingBox> BoundingBoxes;

FUSIONPHYSICS::ObjectBoundingBox::ObjectBoundingBox()
{
	Min.x = std::numeric_limits<float>::max();
	Max.x = std::numeric_limits<float>::lowest();
	Min.y = std::numeric_limits<float>::max();
	Max.y = std::numeric_limits<float>::lowest();
	Min.z = std::numeric_limits<float>::max();
	Max.z = std::numeric_limits<float>::lowest();
}

void FUSIONPHYSICS::ObjectBoundingBox::CompareVec3MinMax(glm::vec3 &v)
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

void FUSIONPHYSICS::CalculateInitialQuadTreeGridSize(std::vector<ObjectBoundingBox>& BoundingBoxes, std::vector<FUSIONCORE::Object*> &ObjectInstances)
{
	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::lowest();
	float minZ = std::numeric_limits<float>::max();
	float maxZ = std::numeric_limits<float>::lowest();

	int Recalculated = 0;

	for (size_t i = 0; i < ObjectInstances.size(); i++)
	{
		bool ReCalculateBoundingBox = false;
		ObjectBoundingBox BoundingBox;
		BoundingBox.Object = ObjectInstances[i];
		FUSIONCORE::WorldTransform& objectTransformation = ObjectInstances[i]->GetTransformation();

		if (BoundingBoxes.size() < i + 1 || 
			!BoundingBoxes[i].Object->IsSameObject(BoundingBox.Object) || 
			objectTransformation.IsTransformedQuadTree)
		{
			ReCalculateBoundingBox = true;
		}
		
		if(ReCalculateBoundingBox)
		{
			Recalculated++;
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
					}
				}
			}
			if (BoundingBoxes.size() >= i + 1)
			{
				std::swap(BoundingBox, BoundingBoxes[i]);
			}
			else
			{
				BoundingBoxes.push_back(BoundingBox);
			}
			objectTransformation.IsTransformedQuadTree = false;
		}
	}

	//LOG("Recalculated: " << Recalculated << " ObjectInstances.size(): " << ObjectInstances.size());

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

FUSIONCORE::WorldTransform FUSIONPHYSICS::NodeToWorldTransform(FUSIONPHYSICS::QuadNode& Node)
{
	FUSIONCORE::WorldTransform newTranform;
	newTranform.InitialObjectScales = Node.Size;
	newTranform.OriginPoint = &Node.Center;
	return newTranform;
}

void FUSIONPHYSICS::VisualizeQuadTree(FUSIONPHYSICS::QuadNode& HeadNode,FUSIONCORE::Camera3D& Camera,FUSIONCORE::Shader& Shader, glm::vec3 NodeColor)
{
	std::deque<FUSIONPHYSICS::QuadNode*> nodes;
	nodes.push_back(&HeadNode);
	while (!nodes.empty())
	{
		auto node = nodes.back();
		nodes.pop_back();
		auto NodeTranform = NodeToWorldTransform(*node);
		FUSIONPHYSICS::CollisionBoxAABB HeadNodeBox(NodeTranform, glm::vec3(1.0f));
		HeadNodeBox.SetMeshColor(NodeColor);
		HeadNodeBox.DrawBoxMesh(Camera, Shader);
		if (!node->ChildrenNode.empty())
		{
			for (size_t i = 0; i < node->ChildrenNode.size(); i++)
			{
				nodes.push_back(node->ChildrenNode[i].get());
			}
		}
	}
}

void FUSIONPHYSICS::SubdivideQuadNode(FUSIONPHYSICS::QuadNode& Node , std::deque<QuadNode*> &NodesToProcess)
{
	glm::vec3 QuarterSize = Node.Size * 0.25f;
	glm::vec3 HalfSize = Node.Size * 0.5f;
	glm::vec3 &ParentCenter = Node.Center;
	for (size_t x = 0; x < 2; x++)
	{
		for (size_t z = 0; z < 2; z++)
		{
			QuadNode newChildNode;
			newChildNode.Center = { ParentCenter.x + (QuarterSize.x * (2.0f * x - 1.0f)),
									ParentCenter.y,
									ParentCenter.z + (QuarterSize.z * (2.0f * z - 1.0f))};
			newChildNode.Size = { HalfSize.x ,Node.Size.y,HalfSize.z};
			newChildNode.SubdivisionCount = Node.SubdivisionCount;
			Node.ChildrenNode.push_back(std::make_shared<QuadNode>(newChildNode));
			NodesToProcess.push_back(Node.ChildrenNode.back().get());
		}
	}
}

std::pair<glm::vec3, glm::vec3> FUSIONPHYSICS::GetGridSize()
{
	return {WorldSizeMin,WorldSizeMax};
}


bool IsObjectInsideQuadNode(const FUSIONPHYSICS::ObjectBoundingBox& object, const FUSIONPHYSICS::QuadNode* node) 
{
	const auto& nodeCenter = node->Center;
	const auto& nodeHalfSize = node->Size * 0.5f;

	const auto& MinObject = object.Min;
	const auto& MaxObject = object.Max;

	const glm::vec3 nodeMin = nodeCenter - nodeHalfSize;
	const glm::vec3 nodeMax = nodeCenter + nodeHalfSize;

	return MaxObject.x >= nodeMin.x && MinObject.x <= nodeMax.x &&
		  //MaxObject.y >= nodeMin.y && MinObject.y <= nodeMax.y &&
		   MaxObject.z >= nodeMin.z && MinObject.z <= nodeMax.z;
}

void FUSIONPHYSICS::UpdateQuadTreeWorldPartitioning(FUSIONPHYSICS::QuadNode& HeadNode, std::vector<FUSIONCORE::Object*> &ObjectInstances, unsigned int SingleQuadObjectCountLimit, unsigned int SubdivisionLimit)
{
	DisposeQuadNodes(HeadNode);
	//BoundingBoxes.clear();
	std::deque<QuadNode*> NodesToProcess;
	CalculateInitialQuadTreeGridSize(BoundingBoxes,ObjectInstances);

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
		if (Node->Objects.size() > SingleQuadObjectCountLimit && Node->SubdivisionCount < SubdivisionLimit)
		{
			Node->SubdivisionCount++;
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
	HeadNode.SubdivisionCount = 0;
	HeadNode.Objects.clear();
}


