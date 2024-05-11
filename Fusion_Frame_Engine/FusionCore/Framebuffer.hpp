#pragma once
#include "Texture.h"
#include "Shader.h"
#include "../FusionUtility/VectorMath.h"
#include "../FusionUtility/Log.h"
#include "ShadowMaps.hpp"
#include "Cubemap.h"
#include "Light.hpp"
#include "Buffer.h"
#include <functional>
#include "Color.hpp"
#include "../FusionUtility/FusionDLLExport.h"

#define FF_FRAMEBUFFER_SCENE_IMAGE_ATTACHMENT 0x8CE0 // GL_COLOR_ATTACHMENT0
#define FF_FRAMEBUFFER_DEPTH_ATTACHMENT 0x8CE1 // GL_COLOR_ATTACHMENT1
#define FF_FRAMEBUFFER_SSR_IMAGE_ATTACHMENT 0x8CE2 // GL_COLOR_ATTACHMENT2
#define FF_FRAMEBUFFER_MODEL_ID_IMAGE_ATTACHMENT 0x8CE3 // GL_COLOR_ATTACHMENT3

#define FF_GBUFFER_WORLD_POSITION_IMAGE_ATTACHMENT 0x8CE2

namespace FUSIONCORE
{

	//Meant for forward rendering pipeline
	class FUSIONFRAME_EXPORT FrameBuffer
	{
	public:
		FrameBuffer(int width, int height);
		
		inline GLuint GetFBOimage() { return fboImage; };
		inline GLuint GetFBO() { return fbo; };
		inline GLuint GetFBODepth() { return fboDepth; };
		inline GLuint GetFBOID() { return IDtexture; };
		inline void SetFBOimage(GLuint Texture) { this->fboImage = Texture; };
		inline Vec2<int> GetFBOSize() { return FBOSize; };

		void Bind();
		void Unbind();

		void Draw(Camera3D& camera, Shader& shader, GLuint ShadowMap,std::function<void()> ShaderPrep, Vec2<int> WindowSize, bool DOFenabled = false, float DOFdistanceFar = 0.09f, float DOFdistanceClose = 0.02f, float DOFintensity = 1.0f, float Gamma = 0.9f, float Exposure = 1.0f);
		void clean();
		
	private:

		GLuint fbo, fboImage, fboDepth ,IDtexture, rbo , SSRtexture;
		Buffer ObjectBuffer;
		Vec2<int> FBOSize;
		int ID;
	};


	//Meant for deferred rendering pipeline
	class FUSIONFRAME_EXPORT Gbuffer
	{
	public:
		Gbuffer(int width, int height);
		
		GLuint GetAlbedoSpecularPass() { return AlbedoSpecularPass; };
		GLuint GetFBO() { return fbo; };
		GLuint GetNormalMetalicPass() { return NormalMetalicPass; };
		GLuint GetPositionDepthPass() { return PositionDepthPass; };
		Vec2<int> GetFBOSize() { return FBOSize; };

		void Bind();
		void Unbind();

		void Draw(Camera3D& camera, Shader& shader, std::function<void()> ShaderPrep, Vec2<int> WindowSize, std::vector<OmniShadowMap*> &ShadowMaps, std::vector<CascadedDirectionalShadowMap*>& CascadedDirectionalShadowMaps, CubeMap& cubeMap, glm::vec4 BackgroundColor = glm::vec4(0.0f), float EnvironmentAmbientAmount = 0.2f);
		void DrawSSR(Camera3D& camera, Shader& shader, std::function<void()> ShaderPrep, Vec2<int> WindowSize);
		void clean();

	private:

		GLuint fbo, AlbedoSpecularPass, NormalMetalicPass, rbo, PositionDepthPass , MetalicRoughnessPass;
		Buffer ObjectBuffer;
		Vec2<int> FBOSize;
		int ID;
	};

	FUSIONFRAME_EXPORT_FUNCTION void CopyDepthInfoFBOtoFBO(GLuint src, glm::vec2 srcSize, GLuint dest);
	FUSIONFRAME_EXPORT_FUNCTION Color ReadFrameBufferPixel(int Xcoord, int Ycoord, unsigned int FramebufferAttachmentMode, GLenum AttachmentFormat, glm::vec2 CurrentWindowSize);
}


