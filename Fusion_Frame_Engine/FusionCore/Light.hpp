#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "Buffer.h"
#include "Mesh.h"
#include <memory>
#include "Model.hpp"
#define MAX_LIGHT_COUNT 110

#define FF_POINT_LIGHT 0x56400
#define FF_DIRECTIONAL_LIGHT 0x56401
#define FF_SPOT_LIGHT 0x56402

namespace FUSIONCORE
{
	struct alignas(16) LightData
	{
		glm::vec4 Position;
		glm::vec4 Color;
		int Type;
		float Intensity;
		float Radius;
	};

	extern std::unique_ptr<Model> LightIcon;
	extern std::unordered_map<int, LightData> LightDatas;
	extern int LightCount;
	extern std::unique_ptr<SSBO> LightsShaderStorageBufferObject;

	//Internally called
	void InitializeLightsShaderStorageBufferObject();

	//Uploads all the light datas avaliable to the light SSBO. Might turn to be kind of an expensive operation.
	//Better to operate on lights as a batch before calling this function since lights themselves don't modify shader buffers. 
	void UploadLightsShaderUniformBuffer();
	//Sets light data SSBO in a given shader.
	void SendLightsShader(Shader& shader);
	//Uploads the given light data to the light SSBO.
	//Call it only if you insert lights(not deletation).
	//Better to operate on lights as a batch before calling this function since lights themselves don't modify shader buffers. 
	void UploadSingleLightShaderUniformBuffer(LightData& Light);

	class Light : public Object
	{
	public:
		Light();
		//Depending on the light type position argument can be passed as direction
		Light(glm::vec3 Position_Direction, glm::vec3 Color, float intensity , int LightType = FF_POINT_LIGHT, float Radius = 10.0f);
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
		float LightIntensity , LightRadius;
		glm::vec3 LightColor , LightDirection;
		bool LightState;
	};
}