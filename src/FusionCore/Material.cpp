#include "Material.hpp"
#include <glew.h>
#include <glfw3.h>

FUSIONCORE::Material::Material(float roughness, float metalic, glm::vec4 Albedo)
{
	this->roughness = roughness;
	this->metalic = metalic;
	this->Albedo = Albedo;
	std::fill_n(this->DisableClayMaterial, 5, 1);
}

void FUSIONCORE::Material::PushTextureMap(const char* Key,Texture2D* TextureMap)
{
	TextureMaps[Key] = TextureMap;

	std::string KeyValue(Key);
	if (KeyValue.find("diffuse") != std::string::npos)
	{
		DisableClayMaterial[0] = 0;
	}
	else if (KeyValue.find("specular") != std::string::npos)
	{
		DisableClayMaterial[1] = 0;
	}
	else if (KeyValue.find("normal") != std::string::npos)
	{
		DisableClayMaterial[2] = 0;
	}
	else if (KeyValue.find("metalic") != std::string::npos)
	{
		DisableClayMaterial[3] = 0;
	}
	else if (KeyValue.find("alpha") != std::string::npos)
	{
		DisableClayMaterial[4] = 0;
	}
}

void FUSIONCORE::Material::PushTextureMap(const char* Key, const char* TextureMap, GLenum TextureType, GLenum PixelType, GLuint Mag_filter, GLuint Min_filter, GLuint Wrap_S_filter, GLuint Wrap_T_filter, bool Flip)
{
	std::shared_ptr<Texture2D> newTexture = std::make_shared<Texture2D>(TextureMap, TextureType, PixelType, 
		                                       Mag_filter, Min_filter, Wrap_S_filter, Wrap_T_filter, Flip);
	PushTextureMap(Key,newTexture.get());
}

void FUSIONCORE::Material::PopTextureMap(const char* Key)
{
	if (TextureMaps.find(Key) != TextureMaps.end())
	{
		std::string KeyValue(Key);
		if (KeyValue.find("diffuse") != std::string::npos)
		{
			DisableClayMaterial[0] = 1;
		}
		else if (KeyValue.find("specular") != std::string::npos)
		{
			DisableClayMaterial[1] = 1;
		}
		else if (KeyValue.find("normal") != std::string::npos)
		{
			DisableClayMaterial[2] = 1;
		}
		else if (KeyValue.find("metalic") != std::string::npos)
		{
			DisableClayMaterial[3] = 1;
		}
		else if (KeyValue.find("alpha") != std::string::npos)
		{
			DisableClayMaterial[4] = 1;
		}

		TextureMaps.erase(Key);
	}
}

FUSIONCORE::Texture2D* FUSIONCORE::Material::GetTextureMap(const char* Key)
{
	auto it = TextureMaps.find(Key);
	if (it != TextureMaps.end()) return it->second;
	return nullptr;
}

void FUSIONCORE::Material::SetMaterialShader(Shader& shader)
{
	int slot = 4;
	for (auto it = TextureMaps.begin(); it != TextureMaps.end(); ++it)
	{
		it->second->Bind(slot, shader.GetID(), it->first.c_str());
		slot++;
	}

	glUniform1iv(glGetUniformLocation(shader.GetID(), "disableclaymaterial"), 5, &DisableClayMaterial[0]);
	shader.setFloat("metallic", this->metalic);
	shader.setFloat("roughness", this->roughness);
	shader.setFloat("TilingCoeff", this->TilingCoeff);
	shader.setVec4("albedo", Albedo);
}

void FUSIONCORE::SetEnvironment(Shader& shader, float FogIntesity, glm::vec3 FogColor, glm::vec3 EnvironmentRadiance)
{
	shader.setFloat("FogIntesityUniform", FogIntesity);
	shader.setVec3("FogColor", FogColor);
	shader.setBool("IBLfog", false);
	shader.setVec3("EnvironmentRadiance", EnvironmentRadiance);
}

void FUSIONCORE::SetEnvironmentIBL(Shader& shader, float FogIntesity, glm::vec3 EnvironmentRadiance)
{
	shader.setFloat("FogIntesityUniform", FogIntesity);
	shader.setBool("IBLfog", true);
	shader.setVec3("EnvironmentRadiance", EnvironmentRadiance);
}
