#pragma once
#include <glew.h>
#include <glfw3.h>
#include "Log.h"
#include <memory>
#include "../FusionOpengl/Shader.h"

namespace FUSIONUTIL
{
	class DefaultShaders
	{
	public:
		std::unique_ptr<FUSIONOPENGL::Shader> BasicShader;
		std::unique_ptr<FUSIONOPENGL::Shader> MeshBasicShader;
		std::unique_ptr<FUSIONOPENGL::Shader> LightShader;
		std::unique_ptr<FUSIONOPENGL::Shader> FBOShader;
		std::unique_ptr<FUSIONOPENGL::Shader> PBRShader;
		std::unique_ptr<FUSIONOPENGL::Shader> ConvolutateCubeMapShader;
		std::unique_ptr<FUSIONOPENGL::Shader> PreFilterCubeMapShader;
		std::unique_ptr<FUSIONOPENGL::Shader> brdfLUTShader;
		std::unique_ptr<FUSIONOPENGL::Shader> HDRIShader;
		std::unique_ptr<FUSIONOPENGL::Shader> CubeMapShader;
	};

	void InitializeDefaultShaders(DefaultShaders& shaders);
	void DisposeDefaultShaders(DefaultShaders& shaders);
	GLFWwindow* InitializeWindow(int width , int height , const char* WindowName);
}