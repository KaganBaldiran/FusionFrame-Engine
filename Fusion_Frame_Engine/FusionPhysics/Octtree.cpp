#include "Octtree.hpp"
#include <map>

glm::vec3 WorldSizeMin;
glm::vec3 WorldSizeMax;

namespace FUSIONPHYSICS
{
	std::vector<CollisionBox*> ObjectInstances;
}

void FUSIONPHYSICS::CalculateGridSize()
{
	float maxX = -std::numeric_limits<float>::infinity();
	float maxY = -std::numeric_limits<float>::infinity();
	float maxZ = -std::numeric_limits<float>::infinity();
	float minX = std::numeric_limits<float>::infinity();
	float minY = std::numeric_limits<float>::infinity();
	float minZ = std::numeric_limits<float>::infinity();

	for (size_t i = 0; i < ObjectInstances.size(); i++)
	{
		auto object = ObjectInstances[i];
		minX = glm::min(object->Min.x, minX);
		minY = glm::min(object->Min.y, minY);
		minZ = glm::min(object->Min.z, minZ);

		maxX = glm::max(object->Max.x, maxX);
		maxY = glm::max(object->Max.y, maxY);
		maxZ = glm::max(object->Max.z, maxZ);
	}

	WorldSizeMin = glm::vec3(minX, minY, minZ);
	WorldSizeMax = glm::vec3(maxX, maxY, maxZ);
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

std::string FUSIONPHYSICS::LinearKeyToRelative(unsigned int LinearKey)
{
	unsigned int PrecisePosition = (LinearKey - 1) % 4;
	unsigned int LargerPosition = ((LinearKey - 1) / 4) + 1;

	return std::to_string(LargerPosition) + ":" + std::to_string(PrecisePosition);
}

int FUSIONPHYSICS::RelativeKeyToLinear(std::string &RelativeKey)
{
	auto seperatorPos = RelativeKey.find(":");
	if (seperatorPos != std::string::npos)
	{
		unsigned int LargerPosition = FastStringToInt(RelativeKey.substr(0,seperatorPos).c_str());
		unsigned int PrecisePosition = FastStringToInt(RelativeKey.substr(seperatorPos + 1, RelativeKey.size() - 1).c_str());

		return ((LargerPosition - 1) * 4) + PrecisePosition + 1;
	}

	return -1;
}


void FUSIONPHYSICS::Subdivide(FUSIONPHYSICS::QuadNode &Node , std::map< std::string, QuadNode*>&SubdividedQuads)
{
	for (size_t i = 0; i < 4; i++)
	{
		QuadNode ChildNode(std::to_string(RelativeKeyToLinear(Node.NodeID)) + ":" + std::to_string(i + 1));
		Node.ChildrenNode.push_back(ChildNode);
		SubdividedQuads[ChildNode.NodeID] = &Node.ChildrenNode.back();
	}
}

std::pair<glm::vec3, glm::vec3> FUSIONPHYSICS::GetGridSize()
{
	return {WorldSizeMin,WorldSizeMax};
}

std::pair<int, int> ReadNodeID(std::string& NodeID)
{
	auto seperatorPos = NodeID.find(":");
	if (seperatorPos != std::string::npos)
	{
		return { FastStringToInt(NodeID.substr(0, seperatorPos).c_str()),FastStringToInt(NodeID.substr(seperatorPos + 1, NodeID.size() - 1).c_str()) };
	}
	return { -1,-1 };
}

std::pair<glm::vec3, glm::vec3> CalculateNodeBoundingBox(std::string &NodeID)
{
	auto NodePosition = ReadNodeID(NodeID);
	glm::vec3 NodeSize = WorldSizeMax - WorldSizeMin;

	for (size_t i = 0; i < NodePosition.first; i++)
	{
		NodeSize /= NodePosition.first / 4.0f;

		//divide by four , thats the count of subdivision 
	}
}

void DisposeNode(FUSIONPHYSICS::QuadNode& HeadNode)
{
	HeadNode.ChildrenNode.clear();
	HeadNode.NodeID = 1;
	HeadNode.Objects.clear();
}

void FUSIONPHYSICS::UpdateWorldBoundries(FUSIONPHYSICS::QuadNode& HeadNode)
{
	std::vector<CollisionBox*> ToProcess = ObjectInstances;
	std::map<std::string, QuadNode*> SubdividedQuads;

	QuadNode* Node = &HeadNode;

	while (!ToProcess.empty())
	{
		if (ToProcess.size() > 2)
		{
			Subdivide(*Node,SubdividedQuads);
			while (!SubdividedQuads.empty())
			{
				/*for (size_t i = 0; i < ToProcess.size(); i++)
				{
					if()
				}*/
			}
		}
	}	
}


