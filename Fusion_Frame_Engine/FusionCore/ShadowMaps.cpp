#include "ShadowMaps.hpp"
#include "Animator.hpp"
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Initialize.h"

namespace FUSIONCORE
{
	int OmniShadowMapBoundLightCount = -1;
	int DirectionalShadowMapBoundLightCount = -1;
}

struct FUSIONFRAME_EXPORT TextureLayer
{
	glm::vec2 Size;
	size_t ID;
	std::vector<std::vector<bool>> layerBitmap;
};

std::map<int, TextureLayer> TextureLayers;
int UpperTextureSizeLimit = -1;
GLuint CascadedShadowMapArray_Texture, CascadedShadowMapArray_FBO;
std::shared_ptr<FUSIONCORE::SSBO> CascadedShadowMapsMetaData;
size_t RepresentativeTextureBitCount = 0;

int LayerIDiterator = 0;
int MinAvailableLayerIndex = 0;

struct alignas(16) CascadedMapMetaDataGPU 
{
	glm::mat4 lightMatrices[FF_MAX_CASCADES * FF_MAX_CASCADED_SHADOW_MAP_COUNT];
	glm::vec4 positionAndSize[FF_MAX_CASCADES * FF_MAX_CASCADED_SHADOW_MAP_COUNT];
	glm::vec4 lightDirection[FF_MAX_CASCADED_SHADOW_MAP_COUNT];
	float shadowCascadeLevels[FF_MAX_CASCADES * FF_MAX_CASCADED_SHADOW_MAP_COUNT];
	float layer[FF_MAX_CASCADES * FF_MAX_CASCADED_SHADOW_MAP_COUNT];
	float cascadeCount[FF_MAX_CASCADED_SHADOW_MAP_COUNT];
};

CascadedMapMetaDataGPU metaDataGPU;

void FUSIONCORE::InitializeCascadedShadowMapTextureArray(size_t UpperTextureSizeLimit_i, size_t LayerCount,size_t RepresentativeTextureBitCount_i)
{
	if (UpperTextureSizeLimit > 0)
	{
		return;
	}

	UpperTextureSizeLimit = UpperTextureSizeLimit_i;

	glGenTextures(1, &CascadedShadowMapArray_Texture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, CascadedShadowMapArray_Texture);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, UpperTextureSizeLimit, UpperTextureSizeLimit, LayerCount, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	//glTexStorage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, UpperTextureSizeLimit, UpperTextureSizeLimit, LayerCount);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	constexpr float BorderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, BorderColor);

	glGenFramebuffers(1, &CascadedShadowMapArray_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, CascadedShadowMapArray_FBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, CascadedShadowMapArray_Texture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOG_ERR("Error Completing the cascaded shadow map array frameBuffer");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	CascadedShadowMapsMetaData = std::make_shared<FUSIONCORE::SSBO>();
	CascadedShadowMapsMetaData->Bind();
	CascadedShadowMapsMetaData->BufferDataFill(GL_SHADER_STORAGE_BUFFER, sizeof(CascadedMapMetaDataGPU), nullptr, GL_STREAM_DRAW);
	CascadedShadowMapsMetaData->BindSSBO(10);
	BindSSBONull(); 
	
	RepresentativeTextureBitCount = RepresentativeTextureBitCount_i;

	TextureLayer Layer;
	Layer.ID = LayerIDiterator;
	Layer.Size = { UpperTextureSizeLimit,UpperTextureSizeLimit };
	Layer.layerBitmap.resize(RepresentativeTextureBitCount, std::vector<bool>(RepresentativeTextureBitCount, false));

	TextureLayers[LayerIDiterator] = Layer;

	LayerIDiterator++;
}

void FUSIONCORE::ClearCascadedTextureBuffers()
{
	glBindFramebuffer(GL_FRAMEBUFFER, CascadedShadowMapArray_FBO);
	glBindTexture(GL_TEXTURE_2D_ARRAY, CascadedShadowMapArray_Texture);
	for (size_t i = 0; i < TextureLayers.size(); i++)
	{
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, CascadedShadowMapArray_Texture, 0, i);
		glClear(GL_DEPTH_BUFFER_BIT);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FUSIONCORE::TerminateCascadedShadowMapTextureArray()
{
	if (UpperTextureSizeLimit < 0)
	{
		return;
	}

	glDeleteTextures(1, &CascadedShadowMapArray_Texture);
	glDeleteFramebuffers(1, &CascadedShadowMapArray_FBO);
}

GLuint FUSIONCORE::GetCascadedShadowMapTextureArray()
{
	return CascadedShadowMapArray_Texture;
}

float FUSIONCORE::GetCascadedTextureArrayUpperTextureLimit()
{
	return UpperTextureSizeLimit;
}


FUSIONCORE::SSBO* FUSIONCORE::GetCascadedShadowMapMetaDataSSBO()
{
	return CascadedShadowMapsMetaData.get();
}

void FUSIONCORE::CalculateLightSpaceMatricesOnGPU(Camera3D& camera, std::vector<CascadedDirectionalShadowMap*>& CascadedDirectionalShadowMaps,Shader& LightSpaceMatrixComputeShader)
{
	LightSpaceMatrixComputeShader.use();
	int CascadedDirectionalShadowMapsCount = CascadedDirectionalShadowMaps.size();

	auto CascadedShadowMapsMetaData = GetCascadedShadowMapMetaDataSSBO();

	static int CascadedShowMapCountPrevious = 0;
	if (CascadedShowMapCountPrevious != CascadedDirectionalShadowMapsCount)
	{
		CascadedShadowMapsMetaData->Bind();

		for (size_t i = 0; i < CascadedDirectionalShadowMapsCount; i++)
		{
			CascadedDirectionalShadowMaps[i]->CurrentGlobalArrayIndex = i;
			CascadedMapMetaData CascadedMetaData = CascadedDirectionalShadowMaps[i]->GetMetaData();
			
			std::copy(std::begin(CascadedMetaData.LightMatrices), std::end(CascadedMetaData.LightMatrices), std::begin(metaDataGPU.lightMatrices) + (i * FF_MAX_CASCADES));
			std::copy(std::begin(CascadedMetaData.PositionAndSize), std::end(CascadedMetaData.PositionAndSize), std::begin(metaDataGPU.positionAndSize) + (i * FF_MAX_CASCADES));
			metaDataGPU.lightDirection[i] = CascadedMetaData.LightDirection;
			std::copy(std::begin(CascadedMetaData.ShadowCascadeLevels), std::end(CascadedMetaData.ShadowCascadeLevels), std::begin(metaDataGPU.shadowCascadeLevels) + (i * FF_MAX_CASCADES));
			std::copy(std::begin(CascadedMetaData.Layer), std::end(CascadedMetaData.Layer), std::begin(metaDataGPU.layer) + (i * FF_MAX_CASCADES));
			metaDataGPU.cascadeCount[i] = CascadedMetaData.CascadeCount;
		}

		const GLsizei dataSize = sizeof(metaDataGPU);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, dataSize, &metaDataGPU);
		
		LightSpaceMatrixComputeShader.setFloat("CameraFov", camera.GetCameraFOV());
		LightSpaceMatrixComputeShader.setFloat("FarPlane", camera.FarPlane);
		LightSpaceMatrixComputeShader.setFloat("NearPlane", camera.NearPlane);
		LightSpaceMatrixComputeShader.setVec3("CameraUpVector", camera.GetUpVector());
	    CascadedShadowMapsMetaData->BindSSBO(10);
	}

	CascadedShowMapCountPrevious = CascadedDirectionalShadowMapsCount;

	LightSpaceMatrixComputeShader.setMat4("ViewMat", camera.viewMat);
	LightSpaceMatrixComputeShader.setFloat("CameraAspectRatio", camera.GetCameraAspectRatio());

	glDispatchCompute(CascadedDirectionalShadowMapsCount,1, 1);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	UseShaderProgram(0);
}

void FUSIONCORE::RefreshCascadedShadowMapBuffers(Camera3D& camera, std::vector<CascadedDirectionalShadowMap*>& CascadedDirectionalShadowMaps, Shader& LightSpaceMatrixComputeShader)
{
	ClearCascadedTextureBuffers();
	CalculateLightSpaceMatricesOnGPU(camera, CascadedDirectionalShadowMaps, LightSpaceMatrixComputeShader);
}

FUSIONCORE::OmniShadowMap::OmniShadowMap(float width, float height, float FarPlane)
{
	static unsigned int IDiterator = 0;
	ID = IDiterator;
	IDiterator++;

	this->far = FarPlane;
	this->ShadowMapSize({ width,height });
	this->shadowProj = glm::mat4(1.0f);

	glGenTextures(1, &this->ShadowMapId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ShadowMapId);

	for (size_t i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		//glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	glGenFramebuffers(1, &depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, this->ShadowMapId, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, this->ShadowMapId, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	BoundLightID = -1;
}

void FUSIONCORE::OmniShadowMap::LightMatrix(glm::vec3 lightPos, GLuint shader)
{
	float aspect = ShadowMapSize.x / ShadowMapSize.y;
	float near = 0.1f;

	this->shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);

	std::vector<glm::mat4> shadowTransforms;
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

	for (size_t i = 0; i < 6; i++)
	{
		glUniformMatrix4fv(glGetUniformLocation(shader, ("shadowMatrices[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(shadowTransforms[i]));
	}
}

void FUSIONCORE::OmniShadowMap::Draw(Shader& shader, Light& BoundLight, std::vector<Model*>& models, Camera3D& camera)
{
	glm::vec3 lightPos = BoundLight.GetLightDirectionPosition();

	glUseProgram(shader.GetID());
	glBindFramebuffer(GL_FRAMEBUFFER, this->depthMapFBO);
	glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
	glViewport(0, 0, ShadowMapSize.x, ShadowMapSize.y);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->ShadowMapId);

	BoundLightID = BoundLight.GetLightID();

	LightMatrix(lightPos, shader.GetID());

	for (size_t i = 0; i < models.size(); i++)
	{
		std::function<void()> shaderPrep;
		auto model = models[i];
		if (model->IsAnimationEnabled())
		{
			shaderPrep = [&]()
				{
					auto& AnimationBoneMatrices = *model->GetAnimationMatricesPointer();
					for (int i = 0; i < AnimationBoneMatrices.size(); ++i)
					{
						shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", AnimationBoneMatrices[i]);
					}
					shader.setBool("EnableAnimation", model->IsAnimationEnabled());
					glUniformMatrix4fv(glGetUniformLocation(shader.GetID(), "shadowMapProj"), 1, GL_FALSE, glm::value_ptr(this->shadowProj));
					glUniform3f(glGetUniformLocation(shader.GetID(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
					glUniform1f(glGetUniformLocation(shader.GetID(), "farPlane"), far);
				};
		}
		else
		{
			shaderPrep = [&]()
				{
					shader.setBool("EnableAnimation", model->IsAnimationEnabled());
					glUniformMatrix4fv(glGetUniformLocation(shader.GetID(), "shadowMapProj"), 1, GL_FALSE, glm::value_ptr(this->shadowProj));
					glUniform3f(glGetUniformLocation(shader.GetID(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
					glUniform1f(glGetUniformLocation(shader.GetID(), "farPlane"), far);
				};
		}
		model->Draw(camera, shader, shaderPrep);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glUseProgram(0);
}

void FUSIONCORE::OmniShadowMap::Draw(Shader shader, Light& BoundLight, Model& model, Camera3D& camera)
{
	glUseProgram(shader.GetID());
	glBindFramebuffer(GL_FRAMEBUFFER, this->depthMapFBO);
	glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
	glViewport(0, 0, ShadowMapSize.x, ShadowMapSize.y);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->ShadowMapId);

	glm::vec3 lightPos = BoundLight.GetLightDirectionPosition();

	LightMatrix(lightPos, shader.GetID());

	glUniformMatrix4fv(glGetUniformLocation(shader.GetID(), "shadowMapProj"), 1, GL_FALSE, glm::value_ptr(this->shadowProj));
	glUniform3f(glGetUniformLocation(shader.GetID(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	glUniform1f(glGetUniformLocation(shader.GetID(), "farPlane"), far);

	std::function<void()> shaderPrep = []() {};

	model.Draw(camera, shader, shaderPrep);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glUseProgram(0);
}

void FUSIONCORE::OmniShadowMap::Draw(Shader shader, Light& BoundLight, std::vector<std::unique_ptr<Model>*>& models, Camera3D& camera)
{
	BoundLightID = BoundLight.GetLightID();

	glUseProgram(shader.GetID());
	glBindFramebuffer(GL_FRAMEBUFFER, this->depthMapFBO);
	glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
	glViewport(0, 0, ShadowMapSize.x, ShadowMapSize.y);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->ShadowMapId);

	glm::vec3 lightPos = BoundLight.GetLightDirectionPosition();

	LightMatrix(lightPos, shader.GetID());

	glUniformMatrix4fv(glGetUniformLocation(shader.GetID(), "shadowMapProj"), 1, GL_FALSE, glm::value_ptr(this->shadowProj));
	glUniform3f(glGetUniformLocation(shader.GetID(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	glUniform1f(glGetUniformLocation(shader.GetID(), "farPlane"), far);

	std::function<void()> shaderPrep = []() {};
	for (size_t i = 0; i < models.size(); i++)
	{
		auto model = models[i];
		if (model->get()->IsAnimationEnabled())
		{
			auto& AnimationBoneMatrices = *model->get()->GetAnimationMatricesPointer();
			for (int i = 0; i < AnimationBoneMatrices.size(); ++i)
			{
				shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", AnimationBoneMatrices[i]);
			}
		}
		shader.setBool("EnableAnimation", model->get()->IsAnimationEnabled());
		models[i]->get()->Draw(camera, shader, shaderPrep);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glUseProgram(0);
}

void FUSIONCORE::OmniShadowMap::BindShadowMapLight(Light& light)
{
	if (light.GetLightType() != FF_POINT_LIGHT)
	{
		return;
	}
	if (BoundLightID < 0)
	{
		if (OmniShadowMapBoundLightCount >= LightCount)
		{
			return;
		}
		OmniShadowMapBoundLightCount++;
	}
	FUSIONCORE::LightDatas[light.GetLightID()].ShadowMapIndex = OmniShadowMapBoundLightCount;
	BoundLightID = light.GetLightID();
}

FUSIONCORE::OmniShadowMap::~OmniShadowMap()
{
	glDeleteTextures(1, &ShadowMapId);
	glDeleteFramebuffers(1, &depthMapFBO);
	LOG_INF("OmniShadow map[ID:" << ID << "] disposed!");
}

FUSIONCORE::CascadedDirectionalShadowMap::CascadedDirectionalShadowMap(std::vector<glm::vec2> ShadowCascadeTextureSizes,std::vector<float> ShadowCascadeLevels)
{
	static unsigned int IDiterator = 0;
	ID = IDiterator;
	IDiterator++;

	this->shadowProj = glm::mat4(1.0f);
	this->ShadowCascadeLevels.assign(ShadowCascadeLevels.begin(), ShadowCascadeLevels.end());
	MetaData.CascadeCount = ShadowCascadeLevels.size();

	for (size_t i = 0; i < ShadowCascadeLevels.size(); i++)
	{
		MetaData.ShadowCascadeLevels[i] = ShadowCascadeLevels[i];
	}

	for (size_t i = 0; i < ShadowCascadeLevels.size() + 1; i++)
	{
		bool SpaceNotFound = false;
		auto CascadeTextureSize = ShadowCascadeTextureSizes[i];
		for (size_t LayerIndex = 0; LayerIndex < TextureLayers.size(); LayerIndex++)
		{
			auto& layer = TextureLayers[LayerIndex];
			auto& LayerBitMap = layer.layerBitmap;

			float BitSizeInPixel = layer.Size.x / LayerBitMap.size();
			float ShadowMapSizeInBits = CascadeTextureSize.x / BitSizeInPixel;
			bool ShouldBreak = false;

			for (int y = -1; y < (LayerBitMap.size() - ShadowMapSizeInBits) && !ShouldBreak; y++)
			{
				for (int x = -1; x < (LayerBitMap.size() - ShadowMapSizeInBits) && !ShouldBreak; x++)
				{
					bool fits = true;
					for (size_t j = 0; j < ShadowMapSizeInBits && fits; ++j)
					{
						for (size_t d = 0; d < ShadowMapSizeInBits && fits; ++d)
						{
							if (LayerBitMap[x + 1 + d][y + 1 + j] == true) 
							{
								fits = false;
								break;
							}
						}

					}

					SpaceNotFound = fits || SpaceNotFound;
					if (fits)
					{
						MetaData.Layer[i] = LayerIndex;
						MetaData.PositionAndSize[i] = { BitSizeInPixel * (x + 1) , BitSizeInPixel * (y + 1) , CascadeTextureSize.x,CascadeTextureSize.y };
						MetaData.PositionAndSize[i] /= UpperTextureSizeLimit;

						for (size_t j = 0; j < ShadowMapSizeInBits; ++j)
						{
							for (size_t d = 0; d < ShadowMapSizeInBits; ++d)
							{
								LayerBitMap[x + 1 + d][y + 1 + j] = true;
							}
						}

						ShouldBreak = true;
					}
				}
			}

			if (LayerIndex == (TextureLayers.size() - 1) && !SpaceNotFound)
			{
				TextureLayer Layer;
				Layer.ID = LayerIDiterator;
				Layer.Size = { UpperTextureSizeLimit,UpperTextureSizeLimit };
				Layer.layerBitmap.resize(RepresentativeTextureBitCount, std::vector<bool>(RepresentativeTextureBitCount, false));

				TextureLayers[LayerIDiterator] = Layer;
				LayerIDiterator++;
			}
		}
	}
	BoundLightID = -1;
}

glm::mat4 FUSIONCORE::CascadedDirectionalShadowMap::GetLightSpaceMatrix(Camera3D& camera, const float nearPlane, const float farPlane, const glm::vec3 LightDirection)
{
	const auto proj = glm::perspective(glm::radians(camera.GetCameraFOV()), camera.GetCameraAspectRatio(), nearPlane, farPlane);
	glm::mat4 inverseMatrix = glm::inverse(proj * camera.viewMat);

	std::vector<glm::vec4> FrustumCorners;
	for (size_t x = 0; x < 2; x++)
	{
		for (size_t y = 0; y < 2; y++)
		{
			for (size_t z = 0; z < 2; z++)
			{
				glm::vec4 Point = inverseMatrix * glm::vec4(2.0f * x - 1.0f,
					2.0f * y - 1.0f,
					2.0f * z - 1.0f,
					1.0f);
				FrustumCorners.push_back(Point / Point.w);
			}
		}
	}

	glm::vec3 center = glm::vec3(0.0f);
	for (size_t i = 0; i < FrustumCorners.size(); i++)
	{
		center += glm::vec3(FrustumCorners[i]);
	}
	center /= FrustumCorners.size();

	const auto LightView = glm::lookAt(center + LightDirection, center, camera.GetUpVector());

	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::lowest();
	float minZ = std::numeric_limits<float>::max();
	float maxZ = std::numeric_limits<float>::lowest();

	for (size_t i = 0; i < FrustumCorners.size(); i++)
	{
		const auto LightSpaceCorner = LightView * FrustumCorners[i];
		minX = std::min(minX, LightSpaceCorner.x);
		maxX = std::max(maxX, LightSpaceCorner.x);
		minY = std::min(minY, LightSpaceCorner.y);
		maxY = std::max(maxY, LightSpaceCorner.y);
		minZ = std::min(minZ, LightSpaceCorner.z);
		maxZ = std::max(maxZ, LightSpaceCorner.z);
	}

	const float Zmultiplier = std::abs(camera.FarPlane - camera.NearPlane) * 0.05f;
	if (minZ < 0.0f)
	{
		minZ *= Zmultiplier;
	}
	else
	{
		minZ /= Zmultiplier;
	}

	if (maxZ < 0.0f)
	{
		maxZ /= Zmultiplier;
	}
	else
	{
		maxZ *= Zmultiplier;
	}

	const glm::mat4 LightProjectionMatrix = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
	return LightProjectionMatrix * LightView;
}

std::vector<glm::mat4> FUSIONCORE::CascadedDirectionalShadowMap::GetLightSpaceMatrices(Camera3D& camera, const glm::vec3 LightDirection)
{
	std::vector<glm::mat4> Matrices;
	Matrices.reserve(this->ShadowCascadeLevels.size());
	for (size_t i = 0; i < this->ShadowCascadeLevels.size() + 1; i++)
	{
		if (i == 0)
		{
			Matrices.push_back(GetLightSpaceMatrix(camera, camera.NearPlane, ShadowCascadeLevels[i], LightDirection));
		}
		else if (i < ShadowCascadeLevels.size())
		{
			Matrices.push_back(GetLightSpaceMatrix(camera, ShadowCascadeLevels[i - 1], ShadowCascadeLevels[i], LightDirection));
		}
		else
		{
			Matrices.push_back(GetLightSpaceMatrix(camera, ShadowCascadeLevels[i - 1], camera.FarPlane, LightDirection));
		}
	}
	return Matrices;
}

void FUSIONCORE::CascadedDirectionalShadowMap::Draw(FUSIONUTIL::DefaultShaders& Shaders, Camera3D& camera, std::vector<Model*>& Models, Light& BoundLight)
{
	if (BoundLight.GetLightType() != FF_DIRECTIONAL_LIGHT)
	{
		throw FFexception("Unsupported light type!");
	}

	auto& shader = Shaders.CascadedDirectionalShadowShaderBasic;

	shader->use();
	glBindFramebuffer(GL_FRAMEBUFFER, CascadedShadowMapArray_FBO);

	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_FRONT);

	CascadedShadowMapsMetaData->BindSSBO(10);

	int PreviousLayer = -1;

	for (size_t y = 0; y < ShadowCascadeLevels.size() + 1; y++)
	{
		auto& Layer = MetaData.Layer[y];
		if (Layer != PreviousLayer)
		{
			glBindTexture(GL_TEXTURE_2D_ARRAY, CascadedShadowMapArray_Texture);
			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, CascadedShadowMapArray_Texture, 0, Layer);
			PreviousLayer = Layer;
		}
		auto PositionAndSize = MetaData.PositionAndSize[y];
		PositionAndSize *= UpperTextureSizeLimit;
		glViewport(PositionAndSize.x, PositionAndSize.y, PositionAndSize.z, PositionAndSize.w);

		for (size_t i = 0; i < Models.size(); i++)
		{
			auto model = Models[i];
			VBO* InstanceDataVBO = model->GetInstanceDataVBOpointer();
			std::function<void()> shaderPrep = [&]() {
				if (model->IsAnimationEnabled() && y == 0)
				{
					AnimationUniformBufferObject->Bind();
					auto& AnimationBoneMatrices = *model->GetAnimationMatricesPointer();
					for (size_t i = 0; i < AnimationBoneMatrices.size(); i++)
					{
						glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4), sizeof(glm::mat4), &AnimationBoneMatrices[i]);
					}
					glBindBuffer(GL_UNIFORM_BUFFER, 0);
				}
				shader->setBool("EnableAnimation", model->IsAnimationEnabled());

				bool EnableInstancing = false;
				if (InstanceDataVBO) EnableInstancing = true;
				shader->setBool("EnableInstancing", EnableInstancing);
				shader->setVec2("MetaDataMatrixIndex", glm::vec2(this->CurrentGlobalArrayIndex ,y));
				//shader->setMat4("LightMatrix", LightMatrices[y]);
			};

			auto& Model = Models[i];
			auto& Meshes = Model->Meshes;

			if (InstanceDataVBO)
			{	
				for (size_t i = 0; i < Meshes.size(); i++)
				{
					auto& mesh = Meshes[i];

					shader->use();
					mesh.GetMeshBuffer().BindVAO();
					Model->GetTransformation().SetModelMatrixUniformLocation(shader->GetID(), "model");

					shaderPrep();

					if (InstanceDataVBO->IsVBOchanged())
					{
						InstanceDataVBO->Bind();
						glEnableVertexAttribArray(7);
						glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
						glBindBuffer(GL_ARRAY_BUFFER, 0);
						glVertexAttribDivisor(7, 1);
						InstanceDataVBO->SetVBOstate(false);
					}

					glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(mesh.GetIndices().size()), GL_UNSIGNED_INT, 0, Model->GetInstanceDataInstanceCount());

					UseShaderProgram(0);
					BindVAONull();
				}
			}
			else
			{
				for (size_t i = 0; i < Meshes.size(); i++)
				{
					auto& mesh = Meshes[i];

					shader->use();
					mesh.GetMeshBuffer().BindVAO();
					Model->GetTransformation().SetModelMatrixUniformLocation(shader->GetID(), "model");

					shaderPrep();

					glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(mesh.GetIndices().size()), GL_UNSIGNED_INT, 0);

					UseShaderProgram(0);
					BindVAONull();
				}
			}
		}
	}
	glCullFace(GL_BACK);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
}

FUSIONCORE::CascadedDirectionalShadowMap::~CascadedDirectionalShadowMap()
{
}

void FUSIONCORE::CascadedDirectionalShadowMap::BindShadowMapLight(Light& light)
{
	if (light.GetLightType() != FF_DIRECTIONAL_LIGHT)
	{
		return;
	}
	if (BoundLightID < 0)
	{
		if (DirectionalShadowMapBoundLightCount >= LightCount)
		{
			return;
		}
		DirectionalShadowMapBoundLightCount++;
	}
	FUSIONCORE::LightDatas[light.GetLightID()].ShadowMapIndex = DirectionalShadowMapBoundLightCount;
	BoundLightID = light.GetLightID();
	this->MetaData.LightDirection = glm::vec4(light.GetLightDirectionPosition(),0.0f);
}
