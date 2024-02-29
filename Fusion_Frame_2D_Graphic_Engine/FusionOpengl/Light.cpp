#include "Light.hpp"
namespace FUSIONOPENGL
{
	std::unique_ptr<FUSIONOPENGL::Model> LightIcon;
	std::vector<glm::vec3> LightPositions;
	std::vector<glm::vec3> LightColors;
	std::vector<int> LightTypes;
	std::vector<float> LightIntensity;
	int LightCount;
}
static int itr = 0;

FUSIONOPENGL::Light::Light()
{
	LightID = itr;
	itr++;
	
	LightCount++;
}

FUSIONOPENGL::Light::Light(glm::vec3 Position, glm::vec3 Color, float intensity)
{
	LightID = itr;
	itr++;

	transformation = std::make_unique<WorldTransformForLights>(LightPositions, LightID);

	LightPositions.push_back(Position);
	LightColors.push_back(Color);
	LightIntensity.push_back(intensity);

	transformation->Scale({ 0.1f,0.1f,0.1f});
	transformation->Translate(Position);

	LightCount++;
}

void FUSIONOPENGL::Light::SetAttrib(glm::vec3 Color, float intensity)
{
	LightColors[LightID] = Color;
	LightIntensity[LightID] = intensity;
}

void FUSIONOPENGL::Light::Draw(Camera3D& camera, Shader& shader)
{
	std::function<void()> LightPrep = [&]() {
		transformation->SetModelMatrixUniformLocation(shader.GetID(), "model");
		shader.setVec3("LightColor", LightColors[LightID]);
	};
	LightIcon->Draw(camera, shader, LightPrep);
}

void FUSIONOPENGL::SendLightsShader(Shader& shader)
{
	glUniform3fv(glGetUniformLocation(shader.GetID(), "LightPositions"), LightPositions.size(), &LightPositions[0][0]);
	glUniform3fv(glGetUniformLocation(shader.GetID(), "LightColors"), LightColors.size(), &LightColors[0][0]);
	glUniform1fv(glGetUniformLocation(shader.GetID(), "LightIntensities"), LightIntensity.size(), &LightIntensity[0]);
	shader.setInt("LightCount", LightCount);
}
