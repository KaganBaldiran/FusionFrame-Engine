#include "Light.hpp"
#include <glew.h>
#include <glfw3.h>

namespace FUSIONCORE
{
	std::unique_ptr<FUSIONCORE::Model> LightIcon;

	std::map<int, std::pair<LightData,FUSIONCORE::WorldTransform*>> LightDatas;
	int LightCount = 0;
	float TotalLightEnergy = 0.0f;
	std::unique_ptr<SSBO> LightsShaderStorageBufferObject;
}
static int itr = 0;

void InsertLightData(glm::vec3 Position_Direction, float Radius, glm::vec3 Color, float intensity, int LightType, unsigned int LightID, FUSIONCORE::WorldTransform* transformation)
{
	FUSIONCORE::LightData newLightData;

	newLightData.Color = glm::vec4(Color, 1.0f);
	newLightData.Type = LightType;
	newLightData.Position = glm::vec4(Position_Direction, 1.0f);
	newLightData.Intensity = intensity;
	newLightData.Radius = Radius;
	newLightData.ShadowMapIndex = -1;

	FUSIONCORE::LightDatas[LightID] = std::make_pair(newLightData,transformation);
}

FUSIONCORE::Light::Light():LightColor(1.0f,1.0f,1.0f),LightIntensity(1.0f),LightType(FF_POINT_LIGHT), LightRadius(1.0f)
{
	LightID = itr;
	itr++;
	LightState = false;
	
	LightDirection = glm::vec3(0.0f);
	InsertLightData(LightDirection, LightRadius, LightColor, LightIntensity, LightType, LightID,&transformation);
	this->transformation.Scale({ 0.1f,0.1f,0.1f });
	TotalLightEnergy += LightIntensity;
	this->ObjectType = FF_OBJECT_TYPE_LIGHT;

	LightCount++;
}

FUSIONCORE::Light::Light(glm::vec3 Position_Direction, glm::vec3 Color, float intensity, int LightType, float Radius)
{
	LightID = itr;
	itr++;
	this->ObjectType = FF_OBJECT_TYPE_LIGHT;

	if (!(LightType == FF_DIRECTIONAL_LIGHT || LightType == FF_POINT_LIGHT || LightType == FF_SPOT_LIGHT))
	{
		throw FFexception("Invalid light type!");
	}

	transformation.Scale({ 0.1f,0.1f,0.1f});
	if (this->LightType == FF_DIRECTIONAL_LIGHT)
	{
		LightDirection = Position_Direction;
	}
	else
	{
		transformation.Translate(Position_Direction);
		LightDirection = transformation.Position;
	}
	
	InsertLightData(LightDirection, Radius, Color, intensity, LightType, LightID, &transformation);
	this->LightType = LightType;
	this->LightColor = Color;
	this->LightIntensity = intensity;
	this->LightRadius = Radius;

	this->LightState = true;
	TotalLightEnergy += intensity;
	LightCount++;
}

void FUSIONCORE::Light::SetAttrib(glm::vec3 Color, float intensity)
{
	
}

void FUSIONCORE::Light::Draw(Camera3D& camera, Shader& shader)
{
	std::function<void()> LightPrep = [&]() {
		transformation.SetModelMatrixUniformLocation(shader.GetID(), "model");
		shader.setFloat("ModelID", ObjectID);
		shader.setVec3("LightColor", glm::vec3(LightDatas[LightID].first.Color));
	};
	LightIcon->Draw(camera, shader, LightPrep);
}

const glm::vec3 FUSIONCORE::Light::GetLightDirectionPosition()
{
	if (this->LightType == FF_DIRECTIONAL_LIGHT)
	{
		return LightDirection;
	}
	else
	{
		return transformation.Position;
	}
	assert(0);
}

void FUSIONCORE::Light::PopLight()
{
	if (LightState)
	{
		LightDatas.erase(LightID);
		LightCount--;
		TotalLightEnergy -= this->LightIntensity;
		this->LightState = false;
	}
}

void FUSIONCORE::Light::PushBackLight()
{
	if (!LightState)
	{
		InsertLightData(this->GetLightDirectionPosition(),LightRadius,LightColor, LightIntensity, LightType, LightID ,&transformation);
		this->LightState = true;
		LightCount++;
		TotalLightEnergy += this->LightIntensity;
	}
}

void FUSIONCORE::InitializeLightsShaderStorageBufferObject()
{
	LightsShaderStorageBufferObject = std::make_unique<SSBO>();
	LightsShaderStorageBufferObject->Bind();
	LightsShaderStorageBufferObject->BufferDataFill(GL_SHADER_STORAGE_BUFFER, sizeof(LightData) * MAX_LIGHT_COUNT,nullptr, GL_DYNAMIC_DRAW);
	BindUBONull();
}

void FUSIONCORE::UploadLightsShader(FUSIONCORE::Shader& DestShader)
{
	std::vector<LightData> TempLightDatas;
	TempLightDatas.reserve(LightCount);
	for (auto& lightdata : LightDatas)
	{
		if (lightdata.second.first.Type == FF_POINT_LIGHT) lightdata.second.first.Position = glm::vec4(lightdata.second.second->Position,1.0f);
		TempLightDatas.push_back(lightdata.second.first);  
	}
	DestShader.use();
	LightsShaderStorageBufferObject->Bind();
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,sizeof(LightData) * LightCount, TempLightDatas.data());
	LightsShaderStorageBufferObject->BindSSBO(4);
	FUSIONCORE::UseShaderProgram(0);
	BindSSBONull();
}

void FUSIONCORE::SendLightsShader(Shader& shader)
{
	shader.setInt("LightCount", LightCount);
	shader.setFloat("TotalLightIntensity", TotalLightEnergy);
	LightsShaderStorageBufferObject->BindSSBO(4);
}

void FUSIONCORE::UploadSingleLightShaderUniformBuffer(LightData& Light)
{
	LightsShaderStorageBufferObject->Bind();
	glBufferSubData(GL_SHADER_STORAGE_BUFFER,sizeof(LightData) * LightCount, sizeof(LightData), &Light);
	LightsShaderStorageBufferObject->BindSSBO(4);
	BindUBONull();
}
