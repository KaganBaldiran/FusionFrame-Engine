#pragma once
#include <glew.h>
#include <glfw3.h>
#include "Log.h"
#include <memory>
#include "../FusionCore/Shader.h"

namespace FUSIONUTIL
{
	class DefaultShaders
	{
	public:
		std::unique_ptr<FUSIONCORE::Shader> BasicShader;
		std::unique_ptr<FUSIONCORE::Shader> MeshBasicShader;
		std::unique_ptr<FUSIONCORE::Shader> LightShader;
		std::unique_ptr<FUSIONCORE::Shader> FBOShader;
		std::unique_ptr<FUSIONCORE::Shader> PBRShader;
		std::unique_ptr<FUSIONCORE::Shader> ConvolutateCubeMapShader;
		std::unique_ptr<FUSIONCORE::Shader> PreFilterCubeMapShader;
		std::unique_ptr<FUSIONCORE::Shader> brdfLUTShader;
		std::unique_ptr<FUSIONCORE::Shader> HDRIShader;
		std::unique_ptr<FUSIONCORE::Shader> CubeMapShader;
		std::unique_ptr<FUSIONCORE::Shader> OmniShadowMapShader;
		std::unique_ptr<FUSIONCORE::Shader> GbufferShader;
		std::unique_ptr<FUSIONCORE::Shader> InstancedGbufferShader;
		std::unique_ptr<FUSIONCORE::Shader> DeferredPBRshader;
		std::unique_ptr<FUSIONCORE::Shader> InstancedPBRshader;
	};

	void InitializeDefaultShaders(DefaultShaders& shaders);
	void DisposeDefaultShaders(DefaultShaders& shaders);
	GLFWwindow* InitializeWindow(int width , int height , const char* WindowName);
}