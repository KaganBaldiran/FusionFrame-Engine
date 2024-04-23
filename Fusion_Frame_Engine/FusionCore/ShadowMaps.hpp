#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/VectorMath.h"
#include "../FusionUtility/Log.h"
#include "Buffer.h"
#include "Texture.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.hpp"
#include "Light.hpp"
#include "../FusionUtility/Initialize.h"
#define MAX_SHADOW_MAP_COUNT glm::max(int(MAX_LIGHT_COUNT / 5.0f),1)
#define MAX_CASCADES 16

namespace FUSIONCORE
{
	struct ShadowMapsData {
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
	extern ShadowMapsData ShadowMapsGlobalUniformData;

	//void InitializeShadowMapsUniformBuffer();

	class OmniShadowMap
	{
	public:
		OmniShadowMap(float width, float height , float FarPlane = 25.0f)
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

		void LightMatrix(glm::vec3 lightPos, GLuint shader)
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

		void Draw(Shader &shader, Light& BoundLight, std::vector<Model*>& models, Camera3D& camera)
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

		void Draw(Shader shader, Light& BoundLight, Model &model, Camera3D& camera)
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

		void Draw(Shader shader, Light& BoundLight, std::vector<std::unique_ptr<Model>*>& models, Camera3D& camera)
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

		GLuint GetShadowMap() { return this->ShadowMapId; };
		Vec2<float> GetShadowMapSize() { this->ShadowMapSize; };
		float GetFarPlane() { return far; };
		unsigned int GetID() { return ID; };
		unsigned int GetBoundLightID() { return BoundLightID; };
		void BindShadowMapLight(Light& light);

		~OmniShadowMap()
		{
			glDeleteTextures(1, &ShadowMapId);
			glDeleteFramebuffers(1, &depthMapFBO);
			LOG_INF("OmniShadow map[ID:" << ID << "] disposed!");
		}

	private:
		GLuint ShadowMapId, depthMapFBO;
		Vec2<float> ShadowMapSize;
		glm::mat4 shadowProj;
		float far;
		unsigned int ID , BoundLightID;
	};


	class CascadedDirectionalShadowMap
	{
	public:

		CascadedDirectionalShadowMap(float width, float height , std::vector<float> ShadowCascadeLevels);
		glm::mat4 GetLightSpaceMatrix(Camera3D& camera, const float nearPlane, const float farPlane, const glm::vec3 LightDirection);
		std::vector<glm::mat4> GetLightSpaceMatrices(Camera3D& camera, const glm::vec3 LightDirection);
		void Draw(FUSIONUTIL::DefaultShaders& Shaders, Camera3D& camera, std::vector<Model*> &Models, Light& BoundLight);
		inline ~CascadedDirectionalShadowMap()
		{
			glDeleteTextures(1, &ShadowMapId);
			glDeleteFramebuffers(1, &depthMapFBO);
			glDeleteBuffers(1, &MatricesUBO);
			LOG_INF("Cascade Shadow map[ID:" << ID << "] disposed!");
		}

		inline GLuint GetShadowMap() { return this->ShadowMapId; };
		inline Vec2<float> GetShadowMapSize() { this->ShadowMapSize; };
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