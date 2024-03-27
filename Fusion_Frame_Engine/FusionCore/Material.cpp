#include "Material.hpp"

void FUSIONCORE::Material::PushTextureMap(const char* Key, Texture2D& TextureMap)
{
	TextureMaps[Key] = &TextureMap;

	std::string KeyValue(Key);
	if (KeyValue.find("texture_diffuse") != std::string::npos)
	{
		DisableClayMaterial[0] = 0;
	}
	else if (KeyValue.find("texture_specular") != std::string::npos)
	{
		DisableClayMaterial[1] = 0;
	}
	else if (KeyValue.find("texture_normal") != std::string::npos)
	{
		DisableClayMaterial[2] = 0;
	}
	else if (KeyValue.find("texture_metalic") != std::string::npos)
	{
		DisableClayMaterial[3] = 0;
	}
	else if (KeyValue.find("texture_alpha") != std::string::npos)
	{
		DisableClayMaterial[4] = 0;
	}
}

void FUSIONCORE::Material::PushTextureMap(const char* Key, const char* TextureMap, GLenum TextureType, GLenum PixelType, GLuint Mag_filter, GLuint Min_filter, GLuint Wrap_S_filter, GLuint Wrap_T_filter, bool Flip)
{
	std::shared_ptr<Texture2D> newTexture = std::make_shared<Texture2D>(TextureMap, TextureType, PixelType, 
		                                       Mag_filter, Min_filter, Wrap_S_filter, Wrap_T_filter, Flip);
	PushTextureMap(Key,*newTexture);
}

void FUSIONCORE::Material::PopTextureMap(const char* Key)
{
	if (TextureMaps.find(Key) != TextureMaps.end())
	{
		std::string KeyValue(Key);
		if (KeyValue.find("texture_diffuse") != std::string::npos)
		{
			DisableClayMaterial[0] = 1;
		}
		else if (KeyValue.find("texture_specular") != std::string::npos)
		{
			DisableClayMaterial[1] = 1;
		}
		else if (KeyValue.find("texture_normal") != std::string::npos)
		{
			DisableClayMaterial[2] = 1;
		}
		else if (KeyValue.find("texture_metalic") != std::string::npos)
		{
			DisableClayMaterial[3] = 1;
		}
		else if (KeyValue.find("texture_alpha") != std::string::npos)
		{
			DisableClayMaterial[4] = 1;
		}

		TextureMaps.erase(Key);
	}
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

void FUSIONCORE::Material::Clean()
{
	for (auto it = TextureMaps.begin(); it != TextureMaps.end(); ++it)
	{
		it->second->Clear();
	}
}
