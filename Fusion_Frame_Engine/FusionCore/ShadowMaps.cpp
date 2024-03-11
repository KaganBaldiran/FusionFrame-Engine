#include "ShadowMaps.hpp"
#include "Animator.hpp"

namespace FUSIONCORE
{
	//UBO ShadowMapsUniformBufferObject;
	ShadowMapsData ShadowMapsGlobalUniformData;
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

	const auto proj = glm::perspective(glm::radians(camera.GetCameraFOV()), (float)this->ShadowMapSize.x / (float)this->ShadowMapSize.y, nearPlane,farPlane);
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

void FUSIONCORE::CascadedDirectionalShadowMap::Draw(Shader& CascadedShadowMapShader, Camera3D& camera, std::vector<Model*>& Models, Light& BoundLight)
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

	CascadedShadowMapShader.use();
	glBindFramebuffer(GL_FRAMEBUFFER, this->depthMapFBO);
	glViewport(0, 0, this->ShadowMapSize.x, this->ShadowMapSize.y);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);
	glCullFace(GL_FRONT);

	for (size_t i = 0; i < Models.size(); i++)
	{
		auto model = Models[i];
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
			CascadedShadowMapShader.setBool("EnableAnimation", model->IsAnimationEnabled());
		};
		Models[i]->Draw(camera, CascadedShadowMapShader, shaderPrep);
	}
	glCullFace(GL_BACK);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);
}
