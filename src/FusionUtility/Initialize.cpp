#include "Initialize.h"
#include <glew.h>
#include <glfw3.h>
#include "../FusionCore/Cubemap.h"
#include "../FusionCore/Light.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "imgui_internal.h"

void FUSIONUTIL::InitializeImguiGLFW(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

void FUSIONUTIL::RenderImguiGLFW()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool FUSIONUTIL::IsAnyItemActive()
{
	return ImGui::IsAnyItemHovered() || ImGui::IsAnyItemActive() || ImGui::IsAnyItemFocused();
}

void FUSIONUTIL::TerminateRenderImguiGLFW()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void FUSIONUTIL::CreateFrameImguiGLFW()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void FUSIONUTIL::TerminateGLFW()
{
	glfwTerminate();
	LOG_INF("GLFW terminated!");
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
	shaders.TextureOnQuadShader = std::make_unique<FUSIONCORE::Shader>("Shaders/FBO.vs", "Shaders/TextureOnQuad.fs");
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
	shaders.DecalShader = std::make_unique<FUSIONCORE::Shader>("Shaders/Decal.vs","Shaders/Decal.fs");
	shaders.TextureMipmapRenderShader = std::make_unique<FUSIONCORE::Shader>("Shaders/FBO.vs", "Shaders/ShapeTextured.fs");
	
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
	FUSIONCORE::DeleteShaderProgram(shaders.DecalShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.TextureMipmapRenderShader->GetID());
	FUSIONCORE::DeleteShaderProgram(shaders.TextureOnQuadShader->GetID());
}

FUSIONUTIL::DefaultShaders::DefaultShaders()
{
	InitializeDefaultShaders(*this);
}

FUSIONUTIL::DefaultShaders::~DefaultShaders()
{
	DisposeDefaultShaders(*this);
}
