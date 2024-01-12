#include "Octtree.hpp"

namespace FUSIONPHYSICS
{
	std::vector<FUSIONOPENGL::Object*> ObjectInstances;
}

FUSIONPHYSICS::WorldBoundryNode::WorldBoundryNode(glm::vec3 Min, glm::vec3 Max, glm::vec3 Position, std::vector<FUSIONOPENGL::Object*> Objects)
{
	this->Position = Position;
}

void FUSIONPHYSICS::UpdateWorldBoundries()
{


}
