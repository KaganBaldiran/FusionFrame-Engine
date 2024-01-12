#pragma once
#ifndef CUBEMAP
#define CUBEMAP

#include <glew.h>
#include <glfw3.h>
#include "Buffer.h"
#include <vector>
#include <string>
#include "Shader.h"
#include "Camera.h"

#define FF_HDRI_COMPLETE 0x345
#define FF_HDRI_ERROR 0x346
#define FF_HDRI_INCOMPATIBLE_FILE 0x347

namespace FUSIONOPENGL
{
	extern GLuint brdfLUT;
	
	std::pair<GLuint, int> HDRItoCubeMap(const char* HDRI, unsigned int CubeMapSize, GLuint HDRItoCubeMapShader);
	std::pair<GLuint, int> ConvolutateCubeMap(GLuint CubeMap, GLuint ConvolutateCubeMapShader);
	std::pair<GLuint, int> PreFilterCubeMap(GLuint CubeMap, GLuint PreFilterCubeMapShader);
	std::pair<GLuint, int> ComputeLUT(Shader& LUTshader);

	class CubeMap
	{
	public:

		CubeMap(std::vector<std::string> texture_faces, Shader& CubeMapShader);
		CubeMap(GLuint CubeMap, Shader& CubeMapShader);
		CubeMap(Shader& CubeMapShader);
		~CubeMap();
		void Draw(Camera3D& camera, Vec2<float> windowSize);
		GLuint GetCubeMapTexture() { return this->cubemaptextureID; };
		void SetCubeMapTexture(GLuint& CubeMapTexture);
		void SetPreFilteredEnvMap(GLuint preFilteredEnvironmentMapID);
		void SetConvDiffCubeMap(GLuint ConvDiffCubeMapID);
		GLuint GetPreFilteredEnvMap() { return this->PrefilteredEnvMap; };
		GLuint GetConvDiffCubeMap() { return this->ConvDiffCubeMap; };
	private:

		GLuint cubemaptextureID;
		VBO vbo;
		VAO vao;
		std::vector<std::string> texture_faces;
		Shader* cubemapshader;
		GLuint PrefilteredEnvMap;
		GLuint ConvDiffCubeMap;
	};

	int ImportCubeMap(const char* HDRIfilePath, unsigned int CubeMapSize, CubeMap& cubemap, GLuint HDRItoCubeMapShader, GLuint ConvolutateCubeMapShader, GLuint PrefilterHDRIShader);
}

#endif 

