#include "Light.hpp"
namespace FUSIONCORE
{
	std::unique_ptr<FUSIONCORE::Model> LightIcon;
	std::vector<glm::vec3> LightPositions;
	std::vector<glm::vec3> LightColors;
	std::vector<int> LightTypes;
	std::vector<float> LightIntensities;
	int LightCount;
}
static int itr = 0;

FUSIONCORE::Light::Light():LightColor(1.0f,1.0f,1.0f),LightIntensity(1.0f),LightType(FF_POINT_LIGHT)
{
	LightID = itr;
	itr++;
	LightState = false;
	
	transformation = std::make_unique<WorldTransformForLights>(LightPositions, LightID);
	transformation->Scale({ 0.1f,0.1f,0.1f });

	LightCount++;
}

FUSIONCORE::Light::Light(glm::vec3 Position_Direction, glm::vec3 Color, float intensity, int LightType)
{
	LightID = itr;
	itr++;

	if (!(LightType == FF_DIRECTIONAL_LIGHT || LightType == FF_POINT_LIGHT || LightType == FF_SPOT_LIGHT))
	{
		throw FFexception("Invalid light type!");
	}

	this->LightType = LightType;
	LightTypes.push_back(LightType);

	transformation = std::make_unique<WorldTransformForLights>(LightPositions, LightID);

	LightPositions.push_back(Position_Direction);
	LightColors.push_back(Color);
	this->LightColor = Color;
	LightIntensities.push_back(intensity);

	transformation->Scale({ 0.1f,0.1f,0.1f});
	if (this->LightType == FF_DIRECTIONAL_LIGHT)
	{
		LightDirection = Position_Direction;
	}
	else
	{
		transformation->Translate(Position_Direction);
	}

	this->LightState = true;

	LightCount++;
}

void FUSIONCORE::Light::SetAttrib(glm::vec3 Color, float intensity)
{
	LightColors[LightID] = Color;
	LightIntensities[LightID] = intensity;
}

void FUSIONCORE::Light::Draw(Camera3D& camera, Shader& shader)
{
	std::function<void()> LightPrep = [&]() {
		transformation->SetModelMatrixUniformLocation(shader.GetID(), "model");
		shader.setVec3("LightColor", LightColors[LightID]);
	};
	LightIcon->Draw(camera, shader, LightPrep);
}

const glm::vec3 FUSIONCORE::Light::GetLightDirectionPosition()
{
	if (this->LightType == FF_DIRECTIONAL_LIGHT)
	{
		return GetTransformation()->Position;
	}
	else
	{
		return LightDirection;
	}
	assert(0);
}

void FUSIONCORE::Light::PopLight()
{
	if (LightState)
	{
		LightTypes.erase(LightTypes.begin() + this->LightID);
		LightPositions.erase(LightPositions.begin() + this->LightID);
		LightColors.erase(LightColors.begin() + this->LightID);
		LightIntensities.erase(LightIntensities.begin() + this->LightID);
		LightCount--;
		this->LightState = false;
	}
}

void FUSIONCORE::Light::PushBackLight()
{
	if (!LightState)
	{
		LightTypes.push_back(LightType);
		LightPositions.push_back(this->GetTransformation()->Position);
		LightColors.push_back(LightColor);
		LightIntensities.push_back(LightIntensity);
		this->LightState = true;
		LightCount++;
	}
}

void FUSIONCORE::SendLightsShader(Shader& shader)
{
	glUniform3fv(glGetUniformLocation(shader.GetID(), "LightPositions"), LightPositions.size(), &LightPositions[0][0]);
	glUniform3fv(glGetUniformLocation(shader.GetID(), "LightColors"), LightColors.size(), &LightColors[0][0]);
	glUniform1fv(glGetUniformLocation(shader.GetID(), "LightIntensities"), LightIntensities.size(), &LightIntensities[0]);
	glUniform1iv(glGetUniformLocation(shader.GetID(), "LightTypes"), LightTypes.size(), &LightTypes[0]);
	shader.setInt("LightCount", LightCount);
}
