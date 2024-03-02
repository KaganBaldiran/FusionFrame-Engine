#pragma once
#include <glew.h>
#include <glfw3.h>
#include "Texture.h"
#include "Shader.h"
#include "../FusionUtility/VectorMath.h"
#include "../FusionUtility/Log.h"
#include <map>

#define TEXTURE_DIFFUSE0 "texture_diffuse0"
#define TEXTURE_NORMAL0 "texture_normal0"
#define TEXTURE_SPECULAR0 "texture_specular0"
#define TEXTURE_METALIC0 "texture_metalic0"
#define TEXTURE_ALPHA0 "texture_alpha0"

#define TEXTURE_DIFFUSE1 "texture_diffuse1"
#define TEXTURE_NORMAL1 "texture_normal1"
#define TEXTURE_SPECULAR1 "texture_specular1"
#define TEXTURE_METALIC1 "texture_metalic1"
#define TEXTURE_ALPHA1 "texture_alpha1"

#define FF_IMPORTED_MATERIAL NULL

namespace FUSIONCORE
{
	static void SetEnvironment(Shader& shader,float FogIntesity = 1.0f, glm::vec3 FogColor = glm::vec3(1.0f), glm::vec3 EnvironmentRadiance = glm::vec3(1.0f))
	{
		shader.setFloat("FogIntesityUniform", FogIntesity);
		shader.setVec3("FogColor", FogColor);
		shader.setBool("IBLfog", false);
		shader.setVec3("EnvironmentRadiance", EnvironmentRadiance);
	}

	static void SetEnvironmentIBL(Shader& shader, float FogIntesity = 1.0f, glm::vec3 EnvironmentRadiance = glm::vec3(1.0f))
	{
		shader.setFloat("FogIntesityUniform", FogIntesity);
		shader.setBool("IBLfog", true);
		shader.setVec3("EnvironmentRadiance", EnvironmentRadiance);
	}

	class Material
	{
	public:


		Material(float roughness = 0.5f,float metalic = 0.0f, glm::vec4 Albedo = { 1.0f, 1.0f, 1.0f, 1.0f })
		{
			this->roughness = roughness;
			this->metalic = metalic;
			this->Albedo = Albedo;
			std::fill_n(this->DisableClayMaterial, 4, 1);
		}

		inline void PushTextureMap(const char* Key, Texture2D& TextureMap)
		{
			TextureMaps[Key] = &TextureMap;

			std::string KeyValue(Key);
			if (KeyValue == TEXTURE_DIFFUSE0)
			{
				DisableClayMaterial[0] = 0;
			}
			else if (KeyValue == TEXTURE_SPECULAR0)
			{
				DisableClayMaterial[1] = 0;
			}
			else if (KeyValue == TEXTURE_NORMAL0)
			{
				DisableClayMaterial[2] = 0;
			}
			else if (KeyValue == TEXTURE_METALIC0)
			{
				DisableClayMaterial[3] = 0;
			}
		}

		inline void PopTextureMap(const char* Key)
		{
			if (TextureMaps.find(Key) != TextureMaps.end())
			{
				std::string KeyValue(Key);
				if (KeyValue == TEXTURE_DIFFUSE0)
				{
					DisableClayMaterial[0] = 1;
				}
				else if (KeyValue == TEXTURE_SPECULAR0)
				{
					DisableClayMaterial[1] = 1;
				}
				else if (KeyValue == TEXTURE_NORMAL0)
				{
					DisableClayMaterial[2] = 1;
				}
				else if (KeyValue == TEXTURE_METALIC0)
				{
					DisableClayMaterial[3] = 1;
				}

				TextureMaps.erase(Key);
			}
		}

		inline Texture2D* GetTextureMap(const char* Key)
		{
			return TextureMaps[Key];
		}

		inline void SetTiling(float Amount = 1.0f)
		{
			this->TilingCoeff = Amount;
		}

		inline std::map<std::string, Texture2D*>& GetTextureMaps()
		{
			return TextureMaps;
		}

		inline void SetMaterialShader(Shader& shader)
		{
			int slot = 4;
			for (auto it = TextureMaps.begin(); it != TextureMaps.end(); ++it)
			{
				it->second->Bind(slot, shader.GetID(), it->first.c_str());
				slot++;
			}

			glUniform1iv(glGetUniformLocation(shader.GetID(), "disableclaymaterial"), 4, &DisableClayMaterial[0]);
			shader.setFloat("metallic", this->metalic);
			shader.setFloat("roughness", this->roughness);
			shader.setFloat("TilingCoeff", this->TilingCoeff);
			shader.setVec4("albedo", Albedo);
		}

		inline void EnableClayMaterial()
		{
			std::fill_n(this->DisableClayMaterial, 4, 1);
		}

		//If called , there is no need to dispose the individual taxtures.
		void Clean()
		{
			for (auto it = TextureMaps.begin(); it != TextureMaps.end(); ++it)
			{
				it->second->Clear();
			}
		}

		float roughness;
		float metalic;
		float TilingCoeff = 1.0f;
		glm::vec4 Albedo;

	private:
		std::map<std::string, Texture2D*> TextureMaps;
		int DisableClayMaterial[4];
	};

}


