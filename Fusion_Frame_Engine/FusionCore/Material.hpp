#pragma once
#include "Texture.h"
#include "Shader.h"
#include "../FusionUtility/VectorMath.h"
#include "../FusionUtility/Definitions.hpp"
#include "../FusionUtility/Log.h"
#include <map>
#include "../FusionUtility/FusionDLLExport.h"
#define TEXTURE_DIFFUSE0 "texture_diffuse0"
#define TEXTURE_NORMAL0 "texture_normal0"
#define TEXTURE_SPECULAR0 "texture_specular0"
#define TEXTURE_METALIC0 "texture_metalic0"
#define TEXTURE_ALPHA0 "texture_alpha0"
#define TEXTURE_EMISSIVE0 "texture_emissive0"

#define TEXTURE_DIFFUSE1 "texture_diffuse1"
#define TEXTURE_NORMAL1 "texture_normal1"
#define TEXTURE_SPECULAR1 "texture_specular1"
#define TEXTURE_METALIC1 "texture_metalic1"
#define TEXTURE_ALPHA1 "texture_alpha1"
#define TEXTURE_EMISSIVE0 "texture_emissive1"

#define FF_IMPORTED_MATERIAL NULL

namespace FUSIONCORE
{
	FUSIONFRAME_EXPORT_FUNCTION void SetEnvironment(Shader& shader, float FogIntesity = 1.0f, glm::vec3 FogColor = glm::vec3(1.0f), glm::vec3 EnvironmentRadiance = glm::vec3(1.0f));
	FUSIONFRAME_EXPORT_FUNCTION void SetEnvironmentIBL(Shader& shader, float FogIntesity = 1.0f, glm::vec3 EnvironmentRadiance = glm::vec3(1.0f));
	
	/*
	 Represents a material used for shading 3D models.

	 The Material class encapsulates properties of a material used for shading 3D models,
	 including roughness, metallic, albedo color, and texture maps. It provides methods
	 for managing texture maps.
 
	 Example usage:
	 // Create a material with default properties
	 Material myMaterial;

	 // Set the albedo color of the material
	 myMaterial.Albedo = {1.0f, 0.0f, 0.0f, 1.0f};

	 // Add a texture map to the material
	 myMaterial.PushTextureMap("diffuse",Texture);

	 // Enable clay material mode for the material
	 myMaterial.EnableClayMaterial();
	 */
	class FUSIONFRAME_EXPORT Material
	{
	public:

		Material(float roughness = 0.5f, float metalic = 0.0f, glm::vec4 Albedo = { 1.0f, 1.0f, 1.0f, 1.0f });
		
		void PushTextureMap(const char* Key, Texture2D& TextureMap);
		void PushTextureMap(const char* Key, const char* TextureMap, GLenum TextureType = FF_TEXTURE_TARGET_GL_TEXTURE_2D, GLenum PixelType = FF_DATA_TYPE_GL_UNSIGNED_INT,
			                GLuint Mag_filter = FF_TEXTURE_FILTER_MODE_GL_LINEAR, GLuint Min_filter = FF_TEXTURE_FILTER_MODE_GL_LINEAR, GLuint Wrap_S_filter = FF_TEXTURE_WRAP_MODE_GL_CLAMP_TO_EDGE, GLuint Wrap_T_filter = FF_TEXTURE_WRAP_MODE_GL_CLAMP_TO_EDGE, bool Flip = true);
		void PopTextureMap(const char* Key);
		
		inline Texture2D* GetTextureMap(const char* Key) { return TextureMaps[Key]; };
		inline void SetTiling(float Amount = 1.0f) { this->TilingCoeff = Amount; };

		inline std::map<std::string, Texture2D*>& GetTextureMaps() { return TextureMaps; };
		void SetMaterialShader(Shader& shader);
		inline void EnableClayMaterial() { std::fill_n(this->DisableClayMaterial, 5, 1); };

		//If called , there is no need to dispose the individual taxtures.
		void Clean();
		
		float roughness;
		float metalic;
		float TilingCoeff = 1.0f;
		float Alpha = 1.0f;
		glm::vec4 Albedo;

	private:
		std::map<std::string, Texture2D*> TextureMaps;
		int DisableClayMaterial[5];
	};
}


