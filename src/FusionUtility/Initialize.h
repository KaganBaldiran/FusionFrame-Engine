#pragma once
#include "Log.h"
#include <memory>
#include "../FusionCore/Shader.h"
#include "FusionDLLExport.h"

struct GLFWwindow;

namespace FUSIONUTIL
{
	class FUSIONFRAME_EXPORT DefaultShaders
	{
	public:
        DefaultShaders();
        ~DefaultShaders();

		/**
        * @brief Default shader for rendering basic objects with a single color.
        * This shader does not support lighting or texturing.
        */
		std::unique_ptr<FUSIONCORE::Shader> BasicShader;
		/**
        * @brief Shader for rendering basic mesh objects with per-vertex lighting.
        * This shader supports diffuse and specular lighting.
        */
		std::unique_ptr<FUSIONCORE::Shader> MeshBasicShader;
        /**
        * @brief Default shader for rendering light representation meshes with a single color.
        * This shader could also be used to render other meshes with a solid color.
        */
		std::unique_ptr<FUSIONCORE::Shader> LightShader;
		/**
        * @brief Shader for rendering frame buffer objects (FBOs).
        * This shader is typically used for post-processing effects.
        */
		std::unique_ptr<FUSIONCORE::Shader> FBOShader;
		/**
        * @brief Physically Based Rendering (PBR) shader.
        * This shader simulates light interactions with materials using PBR principles.
        */
		std::unique_ptr<FUSIONCORE::Shader> PBRShader;
		/**
	    * @brief Shader for convoluting a cube map.
        * This shader is used in pre-processing steps for environment mapping.
       */
		std::unique_ptr<FUSIONCORE::Shader> ConvolutateCubeMapShader;
		/**
        * @brief Shader for pre-filtering a cube map.
        * This shader is used in pre-processing steps for environment mapping.
        */
		std::unique_ptr<FUSIONCORE::Shader> PreFilterCubeMapShader;
		/**
        * @brief Shader for generating a BRDF (Bidirectional Reflectance Distribution Function) Look-Up Table (LUT).
        * This shader is used in pre-processing steps for PBR rendering.
        */
		std::unique_ptr<FUSIONCORE::Shader> brdfLUTShader;
		/**
        * @brief Shader for rendering High Dynamic Range (HDR) images.
        * This shader is typically used for environment mapping.
        */
		std::unique_ptr<FUSIONCORE::Shader> HDRIShader;
		/**
        * @brief Shader for rendering cube maps.
        * This shader is typically used for environment mapping.
        */
		std::unique_ptr<FUSIONCORE::Shader> CubeMapShader;
		/**
        * @brief Shader for rendering omni-directional shadow maps.
        * This shader is used for shadow mapping from point light sources.
        */
		std::unique_ptr<FUSIONCORE::Shader> OmniShadowMapShader;
		/**
        * @brief Shader for rendering geometry buffer (G-buffer) components.
        * This shader is used in deferred rendering pipeline.
        */
		std::unique_ptr<FUSIONCORE::Shader> GbufferShader;
		/**
        * @brief Shader for rendering instanced geometry buffer (G-buffer) components.
        * This shader supports rendering multiple instances of geometry in a single draw call.
        */
		std::unique_ptr<FUSIONCORE::Shader> InstancedGbufferShader;
		/**
        * @brief Deferred Physically Based Rendering (PBR) shader.
        * This shader is used in deferred rendering pipeline for PBR materials.
        */
		std::unique_ptr<FUSIONCORE::Shader> DeferredPBRshader;
		/**
        * @brief Instanced Deferred Physically Based Rendering (PBR) shader.
        * This shader supports rendering multiple instances of PBR materials in a single draw call.
        */
		std::unique_ptr<FUSIONCORE::Shader> InstancedPBRshader;
		/**
        * @brief Shader for rendering cascaded directional shadow maps.
        * This shader is used for shadow mapping from directional light sources with cascaded shadow maps.
        */
		std::unique_ptr<FUSIONCORE::Shader> CascadedDirectionalShadowShader;
        std::unique_ptr<FUSIONCORE::Shader> CascadedDirectionalShadowShaderBasic;

        std::unique_ptr<FUSIONCORE::Shader> ParticleSpawnComputeShader;
        std::unique_ptr<FUSIONCORE::Shader> ParticleUpdateComputeShader;
        std::unique_ptr<FUSIONCORE::Shader> ParticleRenderShader;
        std::unique_ptr<FUSIONCORE::Shader> ParticleInitializeShader;
        std::unique_ptr<FUSIONCORE::Shader> CameraClusterComputeShader;
        std::unique_ptr<FUSIONCORE::Shader> CameraLightCullingComputeShader;
        std::unique_ptr<FUSIONCORE::Shader> SSRshader;
        std::unique_ptr<FUSIONCORE::Shader> ShapeBasicShader;
        std::unique_ptr<FUSIONCORE::Shader> ShapeTexturedShader;
        std::unique_ptr<FUSIONCORE::Shader> CascadedLightSpaceMatrixComputeShader;
        std::unique_ptr<FUSIONCORE::Shader> DecalShader;
        std::unique_ptr<FUSIONCORE::Shader> TextureMipmapRenderShader;
        std::unique_ptr<FUSIONCORE::Shader> TextureOnQuadShader;
	};

	FUSIONFRAME_EXPORT_FUNCTION void InitializeDefaultShaders(DefaultShaders& shaders);
    FUSIONFRAME_EXPORT_FUNCTION void DisposeDefaultShaders(DefaultShaders& shaders);
    
    FUSIONFRAME_EXPORT_FUNCTION void InitializeImguiGLFW(GLFWwindow* window);
    FUSIONFRAME_EXPORT_FUNCTION void RenderImguiGLFW();
    FUSIONFRAME_EXPORT_FUNCTION bool IsAnyItemActive();
    FUSIONFRAME_EXPORT_FUNCTION void TerminateRenderImguiGLFW();
    FUSIONFRAME_EXPORT_FUNCTION void CreateFrameImguiGLFW();
    
    /*
    Terminates GLFW and associated resources.
    */
    FUSIONFRAME_EXPORT_FUNCTION void TerminateGLFW();

    FUSIONFRAME_EXPORT_FUNCTION void SwapBuffers(GLFWwindow* window);
    FUSIONFRAME_EXPORT_FUNCTION void PollEvents();

    //Refreses the window and swaps the buffers
    //Internally calls "SwapBuffers()" and "PollEvents()"
    FUSIONFRAME_EXPORT_FUNCTION void RefreshWindow(GLFWwindow* window);
}