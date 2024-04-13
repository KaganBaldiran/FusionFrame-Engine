#include "Light.hpp"
namespace FUSIONCORE
{
	std::unique_ptr<FUSIONCORE::Model> LightIcon;

	std::unordered_map<int, LightData> LightDatas;
	int LightCount;
	std::unique_ptr<SSBO> LightsShaderStorageBufferObject;
}
static int itr = 0;

FUSIONCORE::Light::Light():LightColor(1.0f,1.0f,1.0f),LightIntensity(1.0f),LightType(FF_POINT_LIGHT)
{
	LightID = itr;
	itr++;
	LightState = false;
	
	transformation = std::make_unique<WorldTransformForLights>(&FUSIONCORE::LightDatas[LightID], LightID);
	transformation->Scale({ 0.1f,0.1f,0.1f });

	LightCount++;
}

void InsertLightData(glm::vec3 Position_Direction,float Radius, glm::vec3 Color, float intensity, int LightType ,unsigned int LightID)
{
	FUSIONCORE::LightData newLightData;

	newLightData.Color = glm::vec4(Color, 1.0f);
	newLightData.Type = LightType;
	newLightData.Position = glm::vec4(Position_Direction, 1.0f);
	newLightData.Intensity = intensity;
	newLightData.Radius = Radius;

	FUSIONCORE::LightDatas[LightID] = newLightData;
}

FUSIONCORE::Light::Light(glm::vec3 Position_Direction, glm::vec3 Color, float intensity, int LightType, float Radius)
{
	LightID = itr;
	itr++;

	if (!(LightType == FF_DIRECTIONAL_LIGHT || LightType == FF_POINT_LIGHT || LightType == FF_SPOT_LIGHT))
	{
		throw FFexception("Invalid light type!");
	}

	InsertLightData(Position_Direction, Radius, Color, intensity, LightType, LightID);
	this->LightType = LightType;
	transformation = std::make_unique<WorldTransformForLights>(&FUSIONCORE::LightDatas[LightID], LightID);
	this->LightColor = Color;
	this->LightIntensity = intensity;
	this->LightRadius = Radius;

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
	
}

void FUSIONCORE::Light::Draw(Camera3D& camera, Shader& shader)
{
	std::function<void()> LightPrep = [&]() {
		transformation->SetModelMatrixUniformLocation(shader.GetID(), "model");
		shader.setVec3("LightColor", glm::vec3(LightDatas[LightID].Color));
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
		return GetTransformation()->Position;
	}
	assert(0);
}

void FUSIONCORE::Light::PopLight()
{
	if (LightState)
	{
		LightDatas.erase(LightID);
		LightCount--;
		this->LightState = false;
	}
}

void FUSIONCORE::Light::PushBackLight()
{
	if (!LightState)
	{
		InsertLightData(this->GetLightDirectionPosition(),LightRadius,LightColor, LightIntensity, LightType, LightID);
		this->LightState = true;
		LightCount++;
	}
}

void FUSIONCORE::InitializeLightsShaderStorageBufferObject()
{
	LightsShaderStorageBufferObject = std::make_unique<SSBO>();
	LightsShaderStorageBufferObject->Bind();
	LightsShaderStorageBufferObject->BufferDataFill(GL_SHADER_STORAGE_BUFFER, sizeof(LightData) * MAX_LIGHT_COUNT,nullptr, GL_DYNAMIC_DRAW);
	BindUBONull();
}

void FUSIONCORE::UploadLightsShaderUniformBuffer()
{
	std::vector<LightData> TempLightDatas;
	TempLightDatas.reserve(LightCount);
	for (auto const& lightdata : LightDatas)
	{
		TempLightDatas.push_back(lightdata.second);  
	}

	LightsShaderStorageBufferObject->Bind();
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0,sizeof(LightData) * LightCount, TempLightDatas.data());
	LightsShaderStorageBufferObject->BindSSBO(4);
	BindSSBONull();
}

void FUSIONCORE::SendLightsShader(Shader& shader)
{
	shader.setInt("LightCount", LightCount);
	LightsShaderStorageBufferObject->BindSSBO(4);
}

void FUSIONCORE::UploadSingleLightShaderUniformBuffer(LightData& Light)
{
	LightsShaderStorageBufferObject->Bind();
	glBufferSubData(GL_SHADER_STORAGE_BUFFER,sizeof(LightData) * LightCount, sizeof(LightData), &Light);
	LightsShaderStorageBufferObject->BindSSBO(4);
	BindUBONull();
}
