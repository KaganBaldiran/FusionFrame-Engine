#pragma once
#include "../FusionUtility/VectorMath.h"
#include "../FusionUtility/Log.h"
#include "Buffer.h"
#include "Texture.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.hpp"
#include "Light.hpp"
#include "../FusionUtility/FusionDLLExport.h"
#define MAX_SHADOW_MAP_COUNT glm::max(int(MAX_LIGHT_COUNT / 5.0f),1)
#define MAX_CASCADES 16

namespace FUSIONUTIL
{
	class DefaultShaders;
}

namespace FUSIONCORE
{
     struct FUSIONFRAME_EXPORT ShadowMapsData
	 {
		glm::vec3 OmnilightPositions[MAX_SHADOW_MAP_COUNT];
		glm::vec3 CascadedlightPositions[MAX_SHADOW_MAP_COUNT];
		GLuint CascadedShadowMaps[MAX_SHADOW_MAP_COUNT];
		float CascadeDistances[MAX_CASCADES];
		int CascadeCount;
		int CascadedShadowMapCount;
		int OmniShadowMapCount;
	};

	//Storing large amounts of data related to shadow maps
	//extern UBO ShadowMapsUniformBufferObject;
	FUSIONFRAME_EXPORT_FUNCTION ShadowMapsData ShadowMapsGlobalUniformData;

	//void InitializeShadowMapsUniformBuffer();

	class FUSIONFRAME_EXPORT OmniShadowMap
	{
	public:
		OmniShadowMap(float width, float height, float FarPlane = 25.0f);
		void LightMatrix(glm::vec3 lightPos, GLuint shader);
		
		void Draw(Shader& shader, Light& BoundLight, std::vector<Model*>& models, Camera3D& camera);
		void Draw(Shader shader, Light& BoundLight, Model& model, Camera3D& camera);
		void Draw(Shader shader, Light& BoundLight, std::vector<std::unique_ptr<Model>*>& models, Camera3D& camera);
		
		GLuint GetShadowMap() { return this->ShadowMapId; };
		Vec2<float> GetShadowMapSize() { return this->ShadowMapSize; };
		float GetFarPlane() { return far; };
		unsigned int GetID() { return ID; };
		unsigned int GetBoundLightID() { return BoundLightID; };
		void BindShadowMapLight(Light& light);

		~OmniShadowMap();
		
	private:
		GLuint ShadowMapId, depthMapFBO;
		Vec2<float> ShadowMapSize;
		glm::mat4 shadowProj;
		float far;
		unsigned int ID , BoundLightID;
	};


    class FUSIONFRAME_EXPORT CascadedDirectionalShadowMap
	{
	public:

		CascadedDirectionalShadowMap(float width, float height , std::vector<float> ShadowCascadeLevels);
		glm::mat4 GetLightSpaceMatrix(Camera3D& camera, const float nearPlane, const float farPlane, const glm::vec3 LightDirection);
		std::vector<glm::mat4> GetLightSpaceMatrices(Camera3D& camera, const glm::vec3 LightDirection);
		void Draw(FUSIONUTIL::DefaultShaders& Shaders, Camera3D& camera, std::vector<Model*> &Models, Light& BoundLight);
		~CascadedDirectionalShadowMap();
		
		inline GLuint GetShadowMap() { return this->ShadowMapId; };
		inline Vec2<float> GetShadowMapSize() { return this->ShadowMapSize; };
		inline unsigned int GetID() { return ID; };
		unsigned int GetBoundLightID() { return BoundLightID; };
		inline const std::vector<float>& GetCascadeLevels() { return this->ShadowCascadeLevels; };

	private:

		GLuint ShadowMapId, depthMapFBO , MatricesUBO;
		Vec2<float> ShadowMapSize;
		glm::mat4 shadowProj;
		unsigned int ID , BoundLightID;
	
		std::vector<float> ShadowCascadeLevels;
	};

}