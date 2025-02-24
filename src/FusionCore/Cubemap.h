#pragma once
#ifndef CUBEMAP
#define CUBEMAP

#include "Buffer.h"
#include <vector>
#include <string>
#include "Shader.h"
#include "Camera.h"
#include "../FusionUtility/FusionDLLExport.h"
#include "../FusionUtility/Initialize.h"

#define FF_HDRI_COMPLETE 0x345
#define FF_HDRI_ERROR 0x346
#define FF_HDRI_INCOMPATIBLE_FILE 0x347

namespace FUSIONCORE
{
	FUSIONFRAME_EXPORT_FUNCTION GLuint brdfLUT;
	
	FUSIONFRAME_EXPORT_FUNCTION std::pair<GLuint, int> HDRItoCubeMap(const char* HDRI, unsigned int CubeMapSize, GLuint HDRItoCubeMapShader);
	FUSIONFRAME_EXPORT_FUNCTION std::pair<GLuint, int> ConvolutateCubeMap(GLuint CubeMap, GLuint ConvolutateCubeMapShader);
	FUSIONFRAME_EXPORT_FUNCTION std::pair<GLuint, int> PreFilterCubeMap(GLuint CubeMap, GLuint PreFilterCubeMapShader);
	FUSIONFRAME_EXPORT_FUNCTION std::pair<GLuint, int> ComputeLUT(Shader& LUTshader);

	/*
	 Represents a cube map used for environment mapping in rendering.

	 The CubeMap class encapsulates the functionality to create, manage, and draw cube maps, which are commonly used for environment mapping.
	 It provides methods for loading cube map textures and drawing the cube map.
	 Also can be created from a HDRI by calling ImportCubeMap() function.

	 Key functionalities include:
	 - Creating cube maps from image files or existing OpenGL texture objects.
	 - Drawing the cube map using a specified camera and window size.

	 Example usage:
	 // Create a cube map from a set of texture faces
	 CubeMap cubemap({"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"}, cubemapShader);

	 // Draw the cube map using a camera and window size
	 cubemap.Draw(camera, windowSize);
	*/
	class FUSIONFRAME_EXPORT CubeMap
	{
	public:

		CubeMap(std::vector<std::string> texture_faces, Shader& CubeMapShader,int BinningSizeX = 0, int BinningSizeY = 0);
		CubeMap(GLuint CubeMap, Shader& CubeMapShader, int BinningSizeX = 0, int BinningSizeY = 0);
		CubeMap(Shader& CubeMapShader, int BinningSizeX = 0, int BinningSizeY = 0);
		~CubeMap();
		void Draw(Camera3D& camera, const glm::vec2& windowSize);
		GLuint GetCubeMapTexture() { return this->cubemaptextureID; };
		void SetCubeMapTexture(GLuint CubeMapTexture);
		void SetPreFilteredEnvMap(GLuint preFilteredEnvironmentMapID);
		void SetConvDiffCubeMap(GLuint ConvDiffCubeMapID);
		GLuint GetPreFilteredEnvMap() { return this->PrefilteredEnvMap; };
		GLuint GetConvDiffCubeMap() { return this->ConvDiffCubeMap; };
		void CalculateBinRadiances(Shader& ShaderBinning, Shader& ShaderPrefixSum, Shader& ShaderGroupPrefixSum, Shader& ShaderAggregatePrefixSums);
		//
		inline const bool IsBinned() { return isBinned; };
		inline void SetBinned(const bool& isBinned) { this->isBinned = isBinned; };
		inline glm::ivec2 GetBinningSize() { return BinningSize; };

		//Used to increase the count of HDRIs imported over the current instance of cubemap.
		inline void BookKeepImportedHDRI() { ImportedHDRCount++; };
		//Used to indicate the count of HDRIs imported over the current instance of cubemap.
		inline unsigned int GetImportedHDRICount() { return ImportedHDRCount; };

		SSBO RadianceBuffer;
		SSBO RadianceSumBuffer;
		SSBO SummedWorkgroupBuffer;
	private:

		GLuint cubemaptextureID;
		VBO vbo;
		VAO vao;
		
		std::vector<std::string> texture_faces;
		Shader* cubemapshader;
		GLuint PrefilteredEnvMap;
		GLuint ConvDiffCubeMap;
		glm::ivec2 BinningSize;
		bool isBinned;
		unsigned int ImportedHDRCount;

		void InitializeRadianceBuffers();
	};

	FUSIONFRAME_EXPORT_FUNCTION int ImportCubeMap(const char* HDRIfilePath, unsigned int CubeMapSize, CubeMap& cubemap,FUSIONUTIL::DefaultShaders& shaders);
}

#endif 

