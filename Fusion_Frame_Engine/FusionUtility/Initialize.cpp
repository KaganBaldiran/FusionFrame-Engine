#include "Initialize.h"
#include <glew.h>
#include <glfw3.h>
#include "../FusionCore/Camera.h"
#include "../FusionCore/Cubemap.h"
#include "../FusionCore/Animator.hpp"
#include "../FusionCore/Light.hpp"
#include "../FusionCore/ShadowMaps.hpp"
#include "../FusionPhysics/ParticleSystem.hpp"

void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {

	const char* sourceStr;
	switch (source) {
	case GL_DEBUG_SOURCE_API: sourceStr = "API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceStr = "Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY: sourceStr = "Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION: sourceStr = "Application"; break;
	case GL_DEBUG_SOURCE_OTHER: sourceStr = "Other"; break;
	default: sourceStr = "Unknown"; break;
	}

	const char* typeStr;
	switch (type) {
	case GL_DEBUG_TYPE_ERROR: typeStr = "Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behavior"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typeStr = "Undefined Behavior"; break;
	case GL_DEBUG_TYPE_PORTABILITY: typeStr = "Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE: typeStr = "Performance"; break;
	case GL_DEBUG_TYPE_MARKER: typeStr = "Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP: typeStr = "Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP: typeStr = "Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER: typeStr = "Other"; break;
	default: typeStr = "Unknown"; break;
	}

	const char* severityStr;
	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH: severityStr = "High"; break;
	case GL_DEBUG_SEVERITY_MEDIUM: severityStr = "Medium"; break;
	case GL_DEBUG_SEVERITY_LOW: severityStr = "Low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = "Notification"; break;
	default: severityStr = "Unknown"; break;
	}


	fprintf(stderr, "OpenGL Debug Message:\n");
	fprintf(stderr, "    Source: %s\n", sourceStr);
	fprintf(stderr, "    Type: %s\n", typeStr);
	fprintf(stderr, "    ID: %u\n", id);
	fprintf(stderr, "    Severity: %s\n", severityStr);
	fprintf(stderr, "    Message: %s\n", message);
}

GLFWwindow* FUSIONUTIL::InitializeWindow(int width, int height,unsigned int MajorGLversion , unsigned int MinorGLversion,bool EnableGLdebug,const char* WindowName)
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

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MajorGLversion);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MinorGLversion);
	if (EnableGLdebug)
	{
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	}

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

	int major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	LOG_INF("OpenGL version : " << major << "." << minor << " core");

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glfwSetScrollCallback(window, FUSIONCORE::scrollCallback);

	if (EnableGLdebug)
	{
		glDebugMessageCallback(debugCallback, nullptr);
	}

	FUSIONCORE::InitializeAnimationUniformBuffer();
	FUSIONPHYSICS::InitializeParticleEmitterUBO();
	FUSIONCORE::InitializeLightsShaderStorageBufferObject();

    return window;
}

void FUSIONUTIL::TerminateWindow()
{
	glfwTerminate();
	LOG_INF("Window terminated!");
}

void FUSIONUTIL::SwapBuffers(GLFWwindow* window)
{
	glfwSwapBuffers(window);
}

void FUSIONUTIL::PollEvents()
{
	glfwPollEvents();
}

void FUSIONUTIL::RefreshWindow(GLFWwindow* window)
{
	SwapBuffers(window);
	PollEvents();
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

	shaders.DeferredPBRshader = std::make_unique<FUSIONCORE::Shader>();
	shaders.DeferredPBRshader->PushShaderSource(FUSIONCORE::FF_VERTEX_SHADER_SOURCE,"Shaders/DeferredPBR.vs");
	shaders.DeferredPBRshader->PushShaderSource(FUSIONCORE::FF_FRAGMENT_SHADER_SOURCE,"Shaders/DeferredPBR.fs");
	shaders.DeferredPBRshader->AlterShaderMacroDefinitionValue(FUSIONCORE::FF_FRAGMENT_SHADER_SOURCE, "MAX_LIGHT_COUNT",std::to_string(MAX_LIGHT_COUNT));
	//shaders.DeferredPBRshader->AlterShaderMacroDefinitionValue(FUSIONCORE::FF_FRAGMENT_SHADER_SOURCE, "MAX_CASCADE_PLANE_COUNT", std::to_string(FF_MAX_CASCADES));
	//shaders.DeferredPBRshader->AlterShaderMacroDefinitionValue(FUSIONCORE::FF_FRAGMENT_SHADER_SOURCE, "MAX_CASCADED_SHADOW_MAP_COUNT", std::to_string(FF_MAX_CASCADED_SHADOW_MAP_COUNT));
	//LOG(shaders.DeferredPBRshader->GetShaderSource(FUSIONCORE::FF_FRAGMENT_SHADER_SOURCE));
	shaders.DeferredPBRshader->Compile();

	shaders.InstancedPBRshader = std::make_unique<FUSIONCORE::Shader>("Shaders/PBRinstanced.vs", "Shaders/PBR.fs");
	shaders.InstancedGbufferShader = std::make_unique<FUSIONCORE::Shader>("Shaders/PBRinstanced.vs", "Shaders/Gbuffer.fs");
	shaders.CascadedDirectionalShadowShader = std::make_unique<FUSIONCORE::Shader>("Shaders/CascadedDirectionalShadowShader.vs", "Shaders/CascadedDirectionalShadowShader.gs", "Shaders/CascadedDirectionalShadowShader.fs");
	
	shaders.CascadedDirectionalShadowShaderBasic = std::make_unique<FUSIONCORE::Shader>();
	shaders.CascadedDirectionalShadowShaderBasic->PushShaderSource(FUSIONCORE::FF_VERTEX_SHADER_SOURCE, "Shaders/CascadedDirectionalShadowShaderBasic.vs");
	shaders.CascadedDirectionalShadowShaderBasic->PushShaderSource(FUSIONCORE::FF_FRAGMENT_SHADER_SOURCE, "Shaders/CascadedDirectionalShadowShader.fs");
	//shaders.CascadedDirectionalShadowShaderBasic->AlterShaderMacroDefinitionValue(FUSIONCORE::FF_VERTEX_SHADER_SOURCE, "MAX_CASCADE_PLANE_COUNT", std::to_string(FF_MAX_CASCADES));
	//shaders.CascadedDirectionalShadowShaderBasic->AlterShaderMacroDefinitionValue(FUSIONCORE::FF_VERTEX_SHADER_SOURCE, "MAX_CASCADED_SHADOW_MAP_COUNT", std::to_string(FF_MAX_CASCADED_SHADOW_MAP_COUNT));
	//LOG(shaders.DeferredPBRshader->GetShaderSource(FUSIONCORE::FF_FRAGMENT_SHADER_SOURCE));
	shaders.CascadedDirectionalShadowShaderBasic->Compile();
	
	shaders.ParticleSpawnComputeShader = std::make_unique<FUSIONCORE::Shader>("Shaders/ParticlesSpawn.comp.glsl");
	shaders.ParticleUpdateComputeShader = std::make_unique<FUSIONCORE::Shader>("Shaders/ParticlesUpdate.comp.glsl");
	shaders.ParticleRenderShader = std::make_unique<FUSIONCORE::Shader>("Shaders/ParticleRenderShader.vs", "Shaders/ParticleRenderShader.fs");
	shaders.ParticleInitializeShader = std::make_unique<FUSIONCORE::Shader>("Shaders/ParticlesInitialize.comp.glsl");
	shaders.CameraClusterComputeShader = std::make_unique<FUSIONCORE::Shader>("Shaders/CameraClusterComputeShader.comp.glsl");
	shaders.CameraLightCullingComputeShader = std::make_unique<FUSIONCORE::Shader>("Shaders/CameraLightCulling.comp.glsl");
	shaders.SSRshader = std::make_unique<FUSIONCORE::Shader>("Shaders/DeferredPBR.vs","Shaders/SSR.fs");
	shaders.ShapeBasicShader = std::make_unique<FUSIONCORE::Shader>("Shaders/ShapeBasic.vs","Shaders/ShapeBasic.fs");
	shaders.ShapeTexturedShader = std::make_unique<FUSIONCORE::Shader>("Shaders/ShapeBasic.vs","Shaders/ShapeTextured.fs");
	
	shaders.CascadedLightSpaceMatrixComputeShader = std::make_unique<FUSIONCORE::Shader>();
	shaders.CascadedLightSpaceMatrixComputeShader->PushShaderSource(FUSIONCORE::FF_COMPUTE_SHADER_SOURCE, "Shaders/CascadedShadowMapsLightSpaceMatrix.comp.glsl");
	//shaders.CascadedLightSpaceMatrixComputeShader->AlterShaderMacroDefinitionValue(FUSIONCORE::FF_COMPUTE_SHADER_SOURCE, "MAX_CASCADE_PLANE_COUNT", std::to_string(FF_MAX_CASCADES));
	//shaders.CascadedLightSpaceMatrixComputeShader->AlterShaderMacroDefinitionValue(FUSIONCORE::FF_COMPUTE_SHADER_SOURCE, "MAX_CASCADED_SHADOW_MAP_COUNT", std::to_string(FF_MAX_CASCADED_SHADOW_MAP_COUNT));
	//LOG(shaders.DeferredPBRshader->GetShaderSource(FUSIONCORE::FF_FRAGMENT_SHADER_SOURCE));
	shaders.CascadedLightSpaceMatrixComputeShader->Compile();

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
	FUSIONCORE::DeleteShaderProgram(shaders.CascadedDirectionalShadowShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.CascadedDirectionalShadowShaderBasic->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.ParticleSpawnComputeShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.ParticleUpdateComputeShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.ParticleRenderShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.ParticleInitializeShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.CameraClusterComputeShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.CameraLightCullingComputeShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.SSRshader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.ShapeBasicShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.ShapeTexturedShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.CascadedLightSpaceMatrixComputeShader->GetID());
}
