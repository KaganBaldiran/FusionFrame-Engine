#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "Object.hpp"
#include "Buffer.h"
#include "Camera.h"
#include "Texture.h"
#include "../FusionUtility/Initialize.h"

namespace FUSIONCORE
{
	namespace SHAPES
	{
		//Internally called. No need to call.
		void InitializeShapeBuffers();
		void DrawRectangle(glm::vec4 Color, glm::vec2 Position , glm::vec2 Scale , float RotationInAngles , Camera2D& camera, FUSIONUTIL::DefaultShaders& shaders);
		void DrawRectangleTextured(Texture2D& Texture, glm::vec2 Position, glm::vec2 Scale, float RotationInAngles, Camera2D& camera, FUSIONUTIL::DefaultShaders& shaders);

		void DrawTriangle(glm::vec4 Color, glm::vec2 Position, glm::vec2 Scale, float RotationInAngles, Camera2D& camera, FUSIONUTIL::DefaultShaders& shaders);
		void DrawTriangleTextured(Texture2D& Texture, glm::vec2 Position, glm::vec2 Scale, float RotationInAngles, Camera2D& camera, FUSIONUTIL::DefaultShaders& shaders);

		void DrawHexagon(glm::vec4 Color, glm::vec2 Position, glm::vec2 Scale, float RotationInAngles, Camera2D& camera, FUSIONUTIL::DefaultShaders& shaders);
		void DrawHexagonTextured(Texture2D& Texture, glm::vec2 Position, glm::vec2 Scale, float RotationInAngles, Camera2D& camera, FUSIONUTIL::DefaultShaders& shaders);
	}
}
