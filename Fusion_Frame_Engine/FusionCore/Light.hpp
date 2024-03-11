#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "Buffer.h"
#include "Mesh.h"
#include <memory>
#include "Model.hpp"
#define MAX_LIGHT_COUNT 100

#define FF_POINT_LIGHT 0x56400
#define FF_DIRECTIONAL_LIGHT 0x56401
#define FF_SPOT_LIGHT 0x56402

namespace FUSIONCORE
{
	extern std::unique_ptr<Model> LightIcon;
	extern std::vector<glm::vec3> LightPositions;
	extern std::vector<glm::vec3> LightColors;
	extern std::vector<int> LightTypes;
	extern std::vector<float> LightIntensities;
	extern int LightCount;

	void SendLightsShader(Shader& shader);

	class Light : public Object
	{
	public:
		Light();
		//Depending on the light type position argument can be passed as direction
		Light(glm::vec3 Position_Direction, glm::vec3 Color, float intensity , int LightType = FF_POINT_LIGHT);
		void SetAttrib(glm::vec3 Color, float intensity = 1.0f);
		WorldTransformForLights* GetTransformation() { return this->transformation.get(); };
		void Draw(Camera3D& camera, Shader& shader);
		inline int GetLightType() { return this->LightType; };
		inline const glm::vec3& GetLightColor() { return this->LightColor; };
		inline const int GetLightID() { return this->LightID; };

		//Will return position or the direction depending on the light type. Would recommend checking type by calling GetLightType()
        //FF_DIRECTIONAL_LIGHT - Direction
		//FF_POINT_LIGHT - Position
		const glm::vec3 GetLightDirectionPosition();
		inline const void SetLightDirection(glm::vec3 Direction) { this->LightDirection = Direction; };

		//if light is on stack to be used in the shading calculations
		inline bool IsLightBeingUsed() { return this->LightState; };

		//Remove light
		void PopLight();
		//Add the light. Call this function if you exclusively called PopLight() function beforehand
		void PushBackLight();

	private:
		std::unique_ptr<WorldTransformForLights> transformation;
		int LightID;
		int LightType;
		float LightIntensity;
		glm::vec3 LightColor , LightDirection;
		bool LightState;


	};
}