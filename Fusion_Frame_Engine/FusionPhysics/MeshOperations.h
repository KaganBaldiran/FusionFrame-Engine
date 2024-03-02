#pragma once
#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "../FusionCore/Object.hpp"
#include "../FusionCore/Mesh.h"
#include "../FusionCore/Buffer.h"
#include <memory>

namespace FUSIONPHYSICS
{
	namespace MESHOPERATIONS
	{
	  void LoopSubdivision(FUSIONCORE::Mesh& Mesh, int level);
	  void CollapseDecimation(FUSIONCORE::Mesh& Mesh, int level);
	}
}