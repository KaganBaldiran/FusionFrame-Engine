#pragma once
#include <glew.h>
#include <glfw3.h>
#include "Log.h"
#include "VectorMath.h"
#include "Buffer.h"
#include "Mesh.h"
#include <memory>
#include "Model.hpp"
#define MAX_LIGHT_COUNT 100

namespace FUSIONOPENGL
{
	extern std::unique_ptr<Model> LightIcon;
	extern std::vector<glm::vec3> LightPositions;
	extern std::vector<glm::vec3> LightColors;
	extern std::vector<float> LightIntensity;
	extern int LightCount;

	void SendLightsShader(Shader& shader);

	class Light : public Object
	{
	public:
		Light();
		Light(glm::vec3 Position, glm::vec3 Color, float intensity);
		void SetAttrib(glm::vec3 Position, glm::vec3 Color, float intensity);
		WorldTransform* GetTransformation() { return &this->transformation; };
		void Draw(Camera3D& camera, Shader& shader);
	private:
		WorldTransform transformation;
		int LightID;
	};
}