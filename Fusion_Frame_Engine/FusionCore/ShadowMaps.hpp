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

#ifndef FF_MAX_CASCADES
#define FF_MAX_CASCADES 16
#endif // !MAX_CASCADES

#ifndef FF_MAX_CASCADED_SHADOW_MAP_COUNT
#define FF_MAX_CASCADED_SHADOW_MAP_COUNT 12
#endif // !FF_MAX_CASCADED_SHADOW_MAP_COUNT

namespace FUSIONUTIL
{
	class DefaultShaders;
}

namespace FUSIONCORE
{
	//Initializes the texture array needed for cascaded directional shadow maps.
	//Representative texture bit count is needed for placing the textures on the layers.
	//More bits will result in better texture alignment in trade-off of speed.
	FUSIONFRAME_EXPORT_FUNCTION void InitializeCascadedShadowMapTextureArray(size_t UpperTextureSizeLimit_i,size_t LayerCount,size_t RepresentativeTextureBitCount_i = 40);
	//Internal
	FUSIONFRAME_EXPORT_FUNCTION void ClearCascadedTextureBuffers();
	//Internal
	FUSIONFRAME_EXPORT_FUNCTION void TerminateCascadedShadowMapTextureArray();
	FUSIONFRAME_EXPORT_FUNCTION GLuint GetCascadedShadowMapTextureArray();
	//Internal
	FUSIONFRAME_EXPORT_FUNCTION float GetCascadedTextureArrayUpperTextureLimit();
	//Internal
	FUSIONFRAME_EXPORT_FUNCTION SSBO* GetCascadedShadowMapMetaDataSSBO();

	 /*
	 Represents an omni-directional shadow map used for shadow generation in a scene.

	 The OmniShadowMap class manages an omni-directional shadow map, which is used to
	 generate shadows cast by point light sources.

	 Example usage:
	 // Create an omni-directional shadow map with a width and height of 1024 pixels
	 OmniShadowMap shadowMap(1024, 1024);

	  // Bind the shadow map to a light source
	 shadowMap.BindShadowMapLight(light);
	 
	 // Draw shadows using the shadow map
	 shadowMap.Draw(shader, light, models, camera);

	 // Get the ID of the shadow map texture
	 GLuint shadowMapTextureID = shadowMap.GetShadowMap();
	 */
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
		float GetFarPlane() { return FarPlane; };
		unsigned int GetID() { return ID; };
		int GetBoundLightID() { return BoundLightID; };
		void BindShadowMapLight(Light& light);

		~OmniShadowMap();

	private:
		GLuint ShadowMapId, depthMapFBO;
		Vec2<float> ShadowMapSize;
		glm::mat4 shadowProj;
		float FarPlane;
		unsigned int ID;
		int BoundLightID;
	};

	struct FUSIONFRAME_EXPORT alignas(16) CascadedMapMetaData
	{
		glm::mat4 LightMatrices[FF_MAX_CASCADES];
		glm::vec4 PositionAndSize[FF_MAX_CASCADES];
		glm::vec4 LightDirection;
		float ShadowCascadeLevels[FF_MAX_CASCADES];
		float Layer[FF_MAX_CASCADES];
		float CascadeCount;
	};

	/*
	 Represents a cascaded directional shadow map used for shadow generation in a scene.

	 The CascadedDirectionalShadowMap class manages a cascaded directional shadow map,
	 which is used to generate shadows cast by directional lights with a large influence
	 area, such as the sun.

	 Example usage:
	 // Define the sizes and levels of shadow cascades
	 std::vector<glm::vec2> shadowCascadeTextureSizes = { {1024, 1024}, {512, 512}, {256, 256} };
	 std::vector<float> shadowCascadeLevels = { 0.1f, 0.3f, 1.0f };

	 // Create a cascaded directional shadow map
	 CascadedDirectionalShadowMap shadowMap(shadowCascadeTextureSizes, shadowCascadeLevels);

	 // Bind the shadow map to a light source
	 shadowMap.BindShadowMapLight(myLight);

	 // Draw shadows using the shadow map
	 shadowMap.Draw(defaultShaders, myCamera, myModels, myLight);
	 */
	class FUSIONFRAME_EXPORT CascadedDirectionalShadowMap
	{
	public:

		//Cascade count rule: if there are n cascade levels then n+1 texture sizes must be provided.
		CascadedDirectionalShadowMap(std::vector<glm::vec2> ShadowCascadeTextureSizes, std::vector<float> ShadowCascadeLevels);
		glm::mat4 GetLightSpaceMatrix(Camera3D& camera, const float nearPlane, const float farPlane, const glm::vec3 LightDirection);
		std::vector<glm::mat4> GetLightSpaceMatrices(Camera3D& camera, const glm::vec3 LightDirection);
		void Draw(FUSIONUTIL::DefaultShaders& Shaders,Camera3D& camera, std::vector<Model*>& Models, Light& BoundLight);
		~CascadedDirectionalShadowMap();

		inline Vec2<float> GetShadowMapSize() { return this->ShadowMapSize; };
		inline unsigned int GetID() { return ID; };
		inline int GetBoundLightID() { return BoundLightID; };
		inline const std::vector<float>& GetCascadeLevels() { return this->ShadowCascadeLevels; };
		void BindShadowMapLight(Light& light);

		int CurrentGlobalArrayIndex;

		inline CascadedMapMetaData& GetMetaData() { return this->MetaData; };
	private:
		Vec2<float> ShadowMapSize;
		glm::mat4 shadowProj;
		unsigned int ID;
		int BoundLightID;

		std::vector<float> ShadowCascadeLevels;
		CascadedMapMetaData MetaData;
	};

	//Internal
	FUSIONFRAME_EXPORT_FUNCTION void CalculateLightSpaceMatricesOnGPU(Camera3D& camera, std::vector<CascadedDirectionalShadowMap*>& CascadedDirectionalShadowMaps, Shader& LightSpaceMatrixComputeShader);
	
	//Call before drawing cascaded shadow maps to clear and recalculate data on the buffers.
	//Internally calls "ClearCascadedTextureBuffers()" and "CalculateLightSpaceMatricesOnGPU()" so you don't have to call them. 
	FUSIONFRAME_EXPORT_FUNCTION void RefreshCascadedShadowMapBuffers(Camera3D& camera, std::vector<CascadedDirectionalShadowMap*>& CascadedDirectionalShadowMaps, Shader& LightSpaceMatrixComputeShader);
	FUSIONFRAME_EXPORT_FUNCTION void SetCascadedShadowSoftness(Shader& TargetShader,float Softness = 1.0f);
	FUSIONFRAME_EXPORT_FUNCTION void SetCascadedShadowBiasMultiplier(Shader& TargetShader,float ShadowBiasMultiplier = 0.005f);
}