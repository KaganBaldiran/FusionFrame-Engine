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
#define FF_MAX_CASCADED_SHADOW_MAP_COUNT 10
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
	FUSIONFRAME_EXPORT_FUNCTION void ClearCascadedTextureBuffers();
	FUSIONFRAME_EXPORT_FUNCTION void TerminateCascadedShadowMapTextureArray();
	FUSIONFRAME_EXPORT_FUNCTION GLuint GetCascadedShadowMapTextureArray();
	FUSIONFRAME_EXPORT_FUNCTION float GetCascadedTextureArrayUpperTextureLimit();
	FUSIONFRAME_EXPORT_FUNCTION SSBO* GetCascadedShadowMapMetaDataSSBO();

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
		int GetBoundLightID() { return BoundLightID; };
		void BindShadowMapLight(Light& light);

		~OmniShadowMap();

	private:
		GLuint ShadowMapId, depthMapFBO;
		Vec2<float> ShadowMapSize;
		glm::mat4 shadowProj;
		float far;
		unsigned int ID;
		int BoundLightID;
	};

	struct alignas(16) CascadedMapMetaData
	{
		glm::mat4 LightMatrices[FF_MAX_CASCADES];
		glm::vec4 PositionAndSize[FF_MAX_CASCADES];
		glm::vec4 LightDirection;
		float ShadowCascadeLevels[FF_MAX_CASCADES];
		float Layer[FF_MAX_CASCADES];
		float CascadeCount;
	};

	class FUSIONFRAME_EXPORT CascadedDirectionalShadowMap
	{
	public:

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

	void CalculateLightSpaceMatricesOnGPU(Camera3D& camera, std::vector<CascadedDirectionalShadowMap*>& CascadedDirectionalShadowMaps, Shader& LightSpaceMatrixComputeShader);
	
	//Call before drawing cascaded shadow maps to clear and recalculate data on the buffers.
	//Internally calls "ClearCascadedTextureBuffers()" and "CalculateLightSpaceMatricesOnGPU()" so you don't have to call them. 
	void RefreshCascadedShadowMapBuffers(Camera3D& camera, std::vector<CascadedDirectionalShadowMap*>& CascadedDirectionalShadowMaps, Shader& LightSpaceMatrixComputeShader);
}