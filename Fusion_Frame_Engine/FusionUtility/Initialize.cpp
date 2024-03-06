#include "Initialize.h"
#include "../FusionCore/Camera.h"
#include "../FusionCore/Cubemap.h"

GLFWwindow* FUSIONUTIL::InitializeWindow(int width, int height, const char* WindowName)
{
	if (!glfwInit())
	{
		LOG_ERR("Initializing GLFW!");
		glfwTerminate();
		return nullptr;
	}
	LOG_INF("GLFW initialized!");

	glfwWindowHint(GLFW_SAMPLES, 4);

	glEnable(GL_MULTISAMPLE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, WindowName, NULL, NULL);

	if (window == NULL)
	{
		LOG_ERR("Initializing Window!");
		glfwTerminate();
		return nullptr;
	}
	LOG_INF("Window initialized!");

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
	{
		LOG_ERR("Initializing GLEW!");
		glfwTerminate();
		return nullptr;
	}
	LOG_INF("Glew initialized!");
	LOG_INF("OpenGL version : 3.3 core");

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glfwSetScrollCallback(window, FUSIONCORE::scrollCallback);

    return window;
}

void FUSIONUTIL::InitializeDefaultShaders(DefaultShaders &shaders)
{
	shaders.BasicShader = std::make_unique<FUSIONCORE::Shader>("Shaders/Basic.vs", "Shaders/Basic.fs");
	shaders.MeshBasicShader = std::make_unique<FUSIONCORE::Shader>("Shaders/MeshBasic.vs", "Shaders/MeshBasic.fs");
	shaders.LightShader = std::make_unique<FUSIONCORE::Shader>("Shaders/Light.vs", "Shaders/Light.fs");
	shaders.FBOShader = std::make_unique<FUSIONCORE::Shader>("Shaders/FBO.vs", "Shaders/FBO.fs");
	shaders.PBRShader = std::make_unique<FUSIONCORE::Shader>("Shaders/PBR.vs", "Shaders/PBR.fs");
	shaders.ConvolutateCubeMapShader = std::make_unique<FUSIONCORE::Shader>("Shaders/ConvolutationCubeMap.vs", "Shaders/ConvolutationCubeMap.fs");
	shaders.PreFilterCubeMapShader = std::make_unique<FUSIONCORE::Shader>("Shaders/PreFilterCubeMap.vs", "Shaders/PreFilterCubeMap.fs");
	shaders.brdfLUTShader = std::make_unique<FUSIONCORE::Shader>("Shaders/brdfLUT.vs", "Shaders/brdfLUT.fs");
	shaders.HDRIShader = std::make_unique<FUSIONCORE::Shader>("Shaders/HDRI.vs", "Shaders/HDRI.fs");
	shaders.CubeMapShader = std::make_unique<FUSIONCORE::Shader>("Shaders/CubeMap.vert", "Shaders/CubeMap.frag");
	shaders.OmniShadowMapShader = std::make_unique<FUSIONCORE::Shader>("Shaders/OmniShadowMap.vs", "Shaders/OmniShadowMap.gs", "Shaders/OmniShadowMap.fs");
	shaders.GbufferShader = std::make_unique<FUSIONCORE::Shader>("Shaders/Gbuffer.vs", "Shaders/Gbuffer.fs");
	shaders.DeferredPBRshader = std::make_unique<FUSIONCORE::Shader>("Shaders/DeferredPBR.vs", "Shaders/DeferredPBR.fs");
	shaders.InstancedPBRshader = std::make_unique<FUSIONCORE::Shader>("Shaders/PBRinstanced.vs", "Shaders/PBR.fs");
	shaders.InstancedGbufferShader = std::make_unique<FUSIONCORE::Shader>("Shaders/PBRinstanced.vs", "Shaders/Gbuffer.fs");

	FUSIONCORE::brdfLUT = FUSIONCORE::ComputeLUT(*shaders.brdfLUTShader).first;
}

void FUSIONUTIL::DisposeDefaultShaders(DefaultShaders& shaders)
{
	FUSIONCORE::DeleteShaderProgram(shaders.BasicShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.MeshBasicShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.LightShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.FBOShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.PBRShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.ConvolutateCubeMapShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.HDRIShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.PreFilterCubeMapShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.brdfLUTShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.CubeMapShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.OmniShadowMapShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.GbufferShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.DeferredPBRshader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.InstancedGbufferShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.InstancedPBRshader->GetID());
}
