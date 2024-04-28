#include "ShadowMaps.hpp"
#include "Animator.hpp"
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Initialize.h"

namespace FUSIONCORE
{
	//UBO ShadowMapsUniformBufferObject;
	ShadowMapsData ShadowMapsGlobalUniformData;
	unsigned int OmniShadowMapBoundLightCount = 0;
}

/*
void FUSIONCORE::InitializeShadowMapsUniformBuffer()
{
	ShadowMapsUniformBufferObject.Bind();
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ShadowMapsData), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, ShadowMapsUniformBufferObject.GetUBOID());
	BindUBONull();
}
*/

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

	ShadowMapsGlobalUniformData.OmniShadowMapCount++;
	ShadowMapsGlobalUniformData.CascadedShadowMaps[ID] = ShadowMapId;
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
	ShadowMapsGlobalUniformData.OmnilightPositions[ID] = lightPos;

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
	
	//auto temp = LightColors[light.GetLightID()];

}

FUSIONCORE::OmniShadowMap::~OmniShadowMap()
{
	glDeleteTextures(1, &ShadowMapId);
	glDeleteFramebuffers(1, &depthMapFBO);
	LOG_INF("OmniShadow map[ID:" << ID << "] disposed!");
}

FUSIONCORE::CascadedDirectionalShadowMap::CascadedDirectionalShadowMap(float width, float height, std::vector<float> ShadowCascadeLevels)
{
	static unsigned int IDiterator = 0;
	ID = IDiterator;
	IDiterator++;

	this->ShadowMapSize({ width,height });
	this->shadowProj = glm::mat4(1.0f);
	this->ShadowCascadeLevels.assign(ShadowCascadeLevels.begin(), ShadowCascadeLevels.end());

	glGenTextures(1, &this->ShadowMapId);
	glBindTexture(GL_TEXTURE_2D_ARRAY, ShadowMapId);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, width, height, ShadowCascadeLevels.size() + 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	constexpr float BorderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, BorderColor);

	glGenFramebuffers(1, &depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, this->depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, ShadowMapId, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOG_ERR("Error Completing the frameBuffer[ID:" << ID << "]!");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenBuffers(1, &this->MatricesUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, MatricesUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 16, nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, MatricesUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

glm::mat4 FUSIONCORE::CascadedDirectionalShadowMap::GetLightSpaceMatrix(Camera3D& camera,const float nearPlane, const float farPlane, const glm::vec3 LightDirection)
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

	BoundLightID = BoundLight.GetLightID();

	auto LightMatrices = GetLightSpaceMatrices(camera, BoundLight.GetLightDirectionPosition());
	glBindBuffer(GL_UNIFORM_BUFFER, MatricesUBO);
	for (size_t i = 0; i < LightMatrices.size(); i++)
	{
		glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4), sizeof(glm::mat4), &LightMatrices[i]);
	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	Shaders.CascadedDirectionalShadowShader->use();
	glBindFramebuffer(GL_FRAMEBUFFER, this->depthMapFBO);
	glViewport(0, 0, this->ShadowMapSize.x, this->ShadowMapSize.y);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_FRONT);

	for (size_t i = 0; i < Models.size(); i++)
	{
		auto model = Models[i];
		VBO* InstanceDataVBO = model->GetInstanceDataVBOpointer();
		std::function<void()> shaderPrep = [&]() {
			if (model->IsAnimationEnabled())
			{
				AnimationUniformBufferObject->Bind();
				auto& AnimationBoneMatrices = *model->GetAnimationMatricesPointer();
				for (size_t i = 0; i < AnimationBoneMatrices.size(); i++)
				{
					glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4), sizeof(glm::mat4), &AnimationBoneMatrices[i]);
				}
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			}
			Shaders.CascadedDirectionalShadowShader->setBool("EnableAnimation", model->IsAnimationEnabled());

			bool EnableInstancing = false;
			if (InstanceDataVBO) EnableInstancing = true;
			Shaders.CascadedDirectionalShadowShader->setBool("EnableInstancing", EnableInstancing);
		};

		if (InstanceDataVBO)
		{
			Models[i]->DrawDeferredInstanced(camera,*Shaders.CascadedDirectionalShadowShader, shaderPrep,*InstanceDataVBO, model->GetInstanceDataInstanceCount());
		}
		else
		{
		    Models[i]->Draw(camera, *Shaders.CascadedDirectionalShadowShader, shaderPrep);
		}
	}
	glCullFace(GL_BACK);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
}

FUSIONCORE::CascadedDirectionalShadowMap::~CascadedDirectionalShadowMap()
{
	glDeleteTextures(1, &ShadowMapId);
	glDeleteFramebuffers(1, &depthMapFBO);
	glDeleteBuffers(1, &MatricesUBO);
	LOG_INF("Cascade Shadow map[ID:" << ID << "] disposed!");
}
