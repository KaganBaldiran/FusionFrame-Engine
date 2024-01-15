#include "Octtree.hpp"

namespace FUSIONPHYSICS
{
	std::vector<FUSIONOPENGL::WorldTransform*> ObjectInstances;
}

FUSIONPHYSICS::WorldBoundryNode::WorldBoundryNode(glm::vec3 Min, glm::vec3 Max, glm::vec3 Position, std::vector<FUSIONOPENGL::Object*> Objects)
{
	this->Position = Position;
}

void FUSIONPHYSICS::UpdateWorldBoundries()
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
		minX = glm::min(object->ObjectScales.x, minX);
		minY = glm::min(object->ObjectScales.y, minY);
		minZ = glm::min(object->ObjectScales.z, minZ);

		maxX = glm::min(object->ObjectScales.x, maxX);
		maxY = glm::min(object->ObjectScales.y, maxY);
		maxZ = glm::min(object->ObjectScales.z, maxZ);
	}

	glm::vec3 WorldSizeMin = glm::vec3(minX, minY, minZ);
	glm::vec3 WorldSizeMax = glm::vec3(maxX, maxY, maxZ);
}
