#pragma once
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "Object.hpp"
#include "Buffer.h"
#include "Camera.h"
#include "Texture.h"
#include "../FusionUtility/FusionDLLExport.h"

namespace FUSIONUTIL
{
	class DefaultShaders;
}

namespace FUSIONCORE
{
	namespace SHAPES
	{
		//Internally called. No need to call.
		FUSIONFRAME_EXPORT_FUNCTION void InitializeShapeBuffers();
		FUSIONFRAME_EXPORT_FUNCTION void DrawRectangle(glm::vec4 Color, glm::vec2 Position , glm::vec2 Scale , float RotationInAngles , Camera2D& camera, FUSIONUTIL::DefaultShaders& shaders);
		FUSIONFRAME_EXPORT_FUNCTION void DrawRectangleTextured(Texture2D& Texture, glm::vec2 Position, glm::vec2 Scale, float RotationInAngles, Camera2D& camera, FUSIONUTIL::DefaultShaders& shaders);
		FUSIONFRAME_EXPORT_FUNCTION void DrawRectangleInstanced(glm::vec4 Color, glm::vec2 Position, glm::vec2 Scale, float RotationInAngles, Camera2D& camera, FUSIONUTIL::DefaultShaders& shaders);

		FUSIONFRAME_EXPORT_FUNCTION void DrawTriangle(glm::vec4 Color, glm::vec2 Position, glm::vec2 Scale, float RotationInAngles, Camera2D& camera, FUSIONUTIL::DefaultShaders& shaders);
		FUSIONFRAME_EXPORT_FUNCTION void DrawTriangleTextured(Texture2D& Texture, glm::vec2 Position, glm::vec2 Scale, float RotationInAngles, Camera2D& camera, FUSIONUTIL::DefaultShaders& shaders);

		FUSIONFRAME_EXPORT_FUNCTION void DrawHexagon(glm::vec4 Color, glm::vec2 Position, glm::vec2 Scale, float RotationInAngles, Camera2D& camera, FUSIONUTIL::DefaultShaders& shaders);
		FUSIONFRAME_EXPORT_FUNCTION void DrawHexagonTextured(Texture2D& Texture, glm::vec2 Position, glm::vec2 Scale, float RotationInAngles, Camera2D& camera, FUSIONUTIL::DefaultShaders& shaders);
	}
}
