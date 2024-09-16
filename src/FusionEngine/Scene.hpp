#pragma once
#include "../FusionCore/Model.hpp"
#include "../FusionCore/Animation.hpp"
#include "../FusionCore/Animator.hpp"
#include "../FusionCore/ShadowMaps.hpp"
#include "../FusionCore/Framebuffer.hpp"
#include "../../Include/nlohmann/json.hpp"

namespace FUSIONENGINE
{
	class FUSIONFRAME_EXPORT Scene
	{
	public:
		void TranslateScene(glm::vec3 Point);
		void RotateScene(glm::vec3 Axis,float RotationInDegrees);
		void ScaleScene(glm::vec3 ScaleInAxises);
	private:
		std::vector<FUSIONCORE::Object*> SceneObjects;
		std::vector<FUSIONCORE::Animation*> SceneAnimations;
		size_t ObjectCount;
		glm::vec3 Origin;
		glm::vec3 Position;
	};
}
