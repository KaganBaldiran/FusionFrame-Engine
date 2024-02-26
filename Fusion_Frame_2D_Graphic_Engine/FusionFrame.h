#pragma once
#include "FusionUtility/Log.h"
#include "FusionOpengl/Shader.h"
#include "FusionOpengl/Buffer.h"
#include "FusionUtility/VectorMath.h"
#include "FusionOpengl/Texture.h"
#include "FusionUtility/Initialize.h"
#include "FusionOpengl/Camera.h"
#include "FusionOpengl/Mesh.h"
#include "FusionOpengl/Model.hpp"
#include "FusionOpengl/Light.hpp"
#include "FusionOpengl/Framebuffer.hpp"
#include "FusionPhysics/Physics.hpp"
#include "FusionOpengl/Color.hpp"
#include "FusionUtility/StopWatch.h"
#include "FusionOpengl/Cubemap.h"
#include "FusionUtility/Thread.h"
#include "FusionPhysics/Octtree.hpp"
#include "FusionOpengl/ShadowMaps.hpp"
#include "FusionOpengl/Animator.hpp"

namespace FUSIONUTIL
{
	glm::vec2 GetMonitorSize();
	float GetDeltaFrame();
}
