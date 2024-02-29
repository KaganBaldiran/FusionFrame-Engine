#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "Buffer.h"
#include "Mesh.h"
#include <memory>
#include "Model.hpp"
#define MAX_LIGHT_COUNT 500

namespace FUSIONOPENGL
{
	extern std::unique_ptr<Model> LightIcon;
	extern std::vector<glm::vec3> LightPositions;
	extern std::vector<glm::vec3> LightColors;
	extern std::vector<int> LightTypes;
	extern std::vector<float> LightIntensity;
	extern int LightCount;

	void SendLightsShader(Shader& shader);

	class Light : public Object
	{
	public:
		Light();
		Light(glm::vec3 Position, glm::vec3 Color, float intensity);
		void SetAttrib(glm::vec3 Color, float intensity);
		WorldTransformForLights* GetTransformation() { return this->transformation.get(); };
		void Draw(Camera3D& camera, Shader& shader);
	private:
		std::unique_ptr<WorldTransformForLights> transformation;
		int LightID;
	};
}