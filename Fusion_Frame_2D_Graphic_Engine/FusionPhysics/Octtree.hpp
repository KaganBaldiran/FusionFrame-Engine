#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "../FusionOpengl/Object.hpp"

namespace FUSIONPHYSICS
{
	extern std::vector<FUSIONOPENGL::WorldTransform*> ObjectInstances;

	class WorldBoundryNode
	{
	public:

		WorldBoundryNode(glm::vec3 Min , glm::vec3 Max, glm::vec3 Position, std::vector<FUSIONOPENGL::Object*> Objects);

	private:
		glm::vec3 Min;
		glm::vec3 Max;
		glm::vec3 Position;
		std::vector<FUSIONOPENGL::Object*> Objects;
	};

	void UpdateWorldBoundries();

}