#pragma once
#include "Model.hpp"
#include "Animation.hpp"
#include "Animator.hpp"
#include "ShadowMaps.hpp"
#include "Framebuffer.hpp"
#include "../../Include/nlohmann/json.hpp"

namespace FUSIONCORE
{
	class Scene
	{
	public:
		void TranslateScene(glm::vec3 Point);
		void RotateScene(glm::vec3 Axis,float RotationInDegrees);
		void ScaleScene(glm::vec3 ScaleInAxises);
	private:
		std::vector<Object*> SceneObjects;
		std::vector<Animation*> SceneAnimations;
		std::vector<Object*> SceneObjects;
		size_t ObjectCount;
		glm::vec3 Origin;
		glm::vec3 Position;
	};
}
