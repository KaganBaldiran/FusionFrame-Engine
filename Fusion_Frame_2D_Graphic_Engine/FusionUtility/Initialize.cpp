#include "Initialize.h"
#include "../FusionOpengl/Camera.h"
#include "../FusionOpengl/Cubemap.h"

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

	glfwSetScrollCallback(window, FUSIONOPENGL::scrollCallback);

    return window;
}

void FUSIONUTIL::InitializeDefaultShaders(DefaultShaders &shaders)
{
	shaders.BasicShader = std::make_unique<FUSIONOPENGL::Shader>("Shaders/Basic.vs", "Shaders/Basic.fs");
	shaders.MeshBasicShader = std::make_unique<FUSIONOPENGL::Shader>("Shaders/MeshBasic.vs", "Shaders/MeshBasic.fs");
	shaders.LightShader = std::make_unique<FUSIONOPENGL::Shader>("Shaders/Light.vs", "Shaders/Light.fs");
	shaders.FBOShader = std::make_unique<FUSIONOPENGL::Shader>("Shaders/FBO.vs", "Shaders/FBO.fs");
	shaders.PBRShader = std::make_unique<FUSIONOPENGL::Shader>("Shaders/PBR.vs", "Shaders/PBR.fs");
	shaders.ConvolutateCubeMapShader = std::make_unique<FUSIONOPENGL::Shader>("Shaders/ConvolutationCubeMap.vs", "Shaders/ConvolutationCubeMap.fs");
	shaders.PreFilterCubeMapShader = std::make_unique<FUSIONOPENGL::Shader>("Shaders/PreFilterCubeMap.vs", "Shaders/PreFilterCubeMap.fs");
	shaders.brdfLUTShader = std::make_unique<FUSIONOPENGL::Shader>("Shaders/brdfLUT.vs", "Shaders/brdfLUT.fs");
	shaders.HDRIShader = std::make_unique<FUSIONOPENGL::Shader>("Shaders/HDRI.vs", "Shaders/HDRI.fs");
	shaders.CubeMapShader = std::make_unique<FUSIONOPENGL::Shader>("Shaders/CubeMap.vert", "Shaders/CubeMap.frag");
	shaders.OmniShadowMapShader = std::make_unique<FUSIONOPENGL::Shader>("Shaders/OmniShadowMap.vs", "Shaders/OmniShadowMap.gs", "Shaders/OmniShadowMap.fs");
	shaders.GbufferShader = std::make_unique<FUSIONOPENGL::Shader>("Shaders/Gbuffer.vs", "Shaders/Gbuffer.fs");
	shaders.DeferredPBRshader = std::make_unique<FUSIONOPENGL::Shader>("Shaders/DeferredPBR.vs", "Shaders/DeferredPBR.fs");

	FUSIONOPENGL::brdfLUT = FUSIONOPENGL::ComputeLUT(*shaders.brdfLUTShader).first;
}

void FUSIONUTIL::DisposeDefaultShaders(DefaultShaders& shaders)
{
	FUSIONOPENGL::DeleteShaderProgram(shaders.BasicShader->GetID());
	FUSIONOPENGL::DeleteShaderProgram(shaders.MeshBasicShader->GetID());
	FUSIONOPENGL::DeleteShaderProgram(shaders.LightShader->GetID());
	FUSIONOPENGL::DeleteShaderProgram(shaders.FBOShader->GetID());
	FUSIONOPENGL::DeleteShaderProgram(shaders.PBRShader->GetID());
	FUSIONOPENGL::DeleteShaderProgram(shaders.ConvolutateCubeMapShader->GetID());
	FUSIONOPENGL::DeleteShaderProgram(shaders.HDRIShader->GetID());
	FUSIONOPENGL::DeleteShaderProgram(shaders.PreFilterCubeMapShader->GetID());
	FUSIONOPENGL::DeleteShaderProgram(shaders.brdfLUTShader->GetID());
	FUSIONOPENGL::DeleteShaderProgram(shaders.CubeMapShader->GetID());
	FUSIONOPENGL::DeleteShaderProgram(shaders.OmniShadowMapShader->GetID());
	FUSIONOPENGL::DeleteShaderProgram(shaders.GbufferShader->GetID());
	FUSIONOPENGL::DeleteShaderProgram(shaders.DeferredPBRshader->GetID());
}
