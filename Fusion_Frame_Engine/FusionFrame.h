#pragma once
#include "FusionUtility/Log.h"
#include "FusionCore/Shader.h"
#include "FusionCore/Buffer.h"
#include "FusionUtility/VectorMath.h"
#include "FusionCore/Texture.h"
#include "FusionUtility/Initialize.h"
#include "FusionCore/Camera.h"
#include "FusionCore/Mesh.h"
#include "FusionCore/Model.hpp"
#include "FusionCore/Light.hpp"
#include "FusionCore/Framebuffer.hpp"
#include "FusionPhysics/Physics.hpp"
#include "FusionCore/Color.hpp"
#include "FusionUtility/StopWatch.h"
#include "FusionCore/Cubemap.h"
#include "FusionUtility/Thread.h"
#include "FusionPhysics/Octtree.hpp"
#include "FusionCore/ShadowMaps.hpp"
#include "FusionCore/Animator.hpp"
#include "FusionPhysics/MeshOperations.h"

namespace FUSIONUTIL
{
	glm::vec2 GetMonitorSize();
	float GetDeltaFrame();
}
