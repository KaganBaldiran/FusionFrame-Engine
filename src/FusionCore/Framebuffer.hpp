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
#include "../FusionUtility/Definitions.hpp"

#define FF_FRAMEBUFFER_SCENE_IMAGE_ATTACHMENT 0x8CE0 // GL_COLOR_ATTACHMENT0
#define FF_FRAMEBUFFER_DEPTH_ATTACHMENT 0x8CE1 // GL_COLOR_ATTACHMENT1
#define FF_FRAMEBUFFER_SSR_IMAGE_ATTACHMENT 0x8CE2 // GL_COLOR_ATTACHMENT2
#define FF_FRAMEBUFFER_MODEL_ID_IMAGE_ATTACHMENT 0x8CE3 // GL_COLOR_ATTACHMENT3

#define FF_GBUFFER_WORLD_POSITION_IMAGE_ATTACHMENT 0x8CE2

namespace FUSIONCORE
{
	void DrawTextureOnQuad(const GLuint& TargetImage, const glm::vec2& LocalQuadPosition, const glm::vec2& QuadSizeInPixel, FUSIONCORE::Camera3D& camera, FUSIONCORE::Shader& shader, const glm::vec2& SampleMultiplier = {1.0f,1.0f}, float Gamma = 0.9f, float Exposure = 1.0f);

	enum FBOattachmentType
	{
		FBO_ATTACHMENT_COLOR = 0x008923,
		FBO_ATTACHMENT_DEPTH = 0x008924
	};

	class Framebuffer
	{
	public:
		Framebuffer();
		Framebuffer(unsigned int width, unsigned int height);
		void PushFramebufferAttachment2D(FBOattachmentType attachmentType = FBO_ATTACHMENT_COLOR, GLint InternalFormat = FF_INTERNAL_FORMAT_GL_RGB16F, GLenum Format = FF_PIXEL_FORMAT_GL_RGB,
			                             GLenum type = FF_DATA_TYPE_GL_FLOAT, GLint MinFilter = FF_TEXTURE_FILTER_MODE_GL_NEAREST, GLint MaxFilter = FF_TEXTURE_FILTER_MODE_GL_NEAREST, 
			                             GLint Wrap_S = FF_TEXTURE_WRAP_MODE_GL_CLAMP_TO_EDGE, GLint Wrap_T = FF_TEXTURE_WRAP_MODE_GL_CLAMP_TO_EDGE);
		void SetDrawModeDefault();
		void DrawAttachment(size_t index, Shader& shader, const std::function<void()>& ShaderPrep = [](){});

	    void AttachRenderBufferTexture2DTarget(Texture2D& InputTexture, const GLuint& MipmapWidth = 0, const GLuint& MipmapHeight = 0, const GLuint& MipmapLevel = 0);
	    void AttachRenderBufferTexture2DTarget(const GLuint& InputTexture, const GLuint& MipmapWidth = 0, const GLuint& MipmapHeight = 0, const GLuint& MipmapLevel = 0);
	    void AttachTexture2DTarget(Texture2D& InputTexture, const GLuint& MipmapLevel = 0);
	    void AttachTexture2DTarget(const GLuint& InputTexture, const GLuint& MipmapLevel = 0);
        void Bind();
		void Unbind();
		inline GLuint GetRBO() { return rbo; };
		inline GLuint& GetFBOattachment(size_t index) { return Attachments[index]; };
		inline glm::ivec2 GetFBOsize() { return FramebufferSize; };
		~Framebuffer();
	private:
		GLuint fbo, rbo;
		std::vector<GLuint> Attachments;
		size_t ColorAttachmentCount;

		glm::ivec2 FramebufferSize;
		size_t ID;
	};


	 /**
	 Represents a framebuffer object used in forward rendering pipelines.

	 The FrameBuffer class encapsulates functionality related to managing a framebuffer
	 for rendering purposes. It provides methods to create, bind, and draw the framebuffer,
	 as well as access various properties of the framebuffer such as size and attached textures.

	 This class is specifically designed for forward rendering pipelines and includes support
	 for depth-of-field effects, gamma correction, and exposure adjustment.

	 Example usage:
	 // Create a framebuffer with a width of 800 pixels and a height of 600 pixels
	 FrameBuffer myFrameBuffer(800, 600);

	 // Bind the framebuffer before rendering
	 myFrameBuffer.Bind();

	 // Draw the framebuffer
	 myFrameBuffer.Draw(myCamera, myShader, PrepareShader, windowSize, true, 0.09f, 0.02f, 1.0f, 0.9f, 1.0f);

	 // Unbind the framebuffer after rendering
	 myFrameBuffer.Unbind();
	 @endcode
	*/
	class FUSIONFRAME_EXPORT ScreenFrameBuffer
	{
	public:
		ScreenFrameBuffer(int width, int height);
		
		inline GLuint GetFBOimage() { return fboImage; };
		inline GLuint GetFBO() { return fbo; };
		inline GLuint GetFBODepth() { return fboDepth; };
		inline GLuint GetFBOID() { return IDtexture; };
		inline void SetFBOimage(GLuint Texture) { this->fboImage = Texture; };
		inline Vec2<int> GetFBOSize() { return FBOSize; };

		void Bind();
		void Unbind();

		void Draw(Camera3D& camera, Shader& shader,std::function<void()> ShaderPrep, const glm::ivec2& WindowSize, bool DOFenabled = false, float DOFdistanceFar = 0.09f, float DOFdistanceClose = 0.02f, float DOFintensity = 1.0f, float Gamma = 0.9f, float Exposure = 1.0f);
		void clean();
		
	private:

		GLuint fbo,fboImage,fboDepth,IDtexture,rbo,SSRtexture;
		Vec2<int> FBOSize;
		int ID;
	};


	 /**
	 Represents a G-buffer object used in a deferred rendering pipeline.

	 The Gbuffer class manages a G-buffer, which is a set of textures used in deferred rendering
	 pipelines to store various attributes of rendered objects, such as albedo, normals, positions,
	 and material properties.

	 This class provides methods to create, bind, and draw the final scene using G-buffer, as well as access various
	 properties of the G-buffer such as size and individual texture passes.

	 Example usage:
	 // Create a G-buffer with a width of 800 pixels and a height of 600 pixels
	 Gbuffer myGBuffer(800, 600);

	 // Bind the G-buffer before rendering
	 myGBuffer.Bind();

	 // Draw the scene using G-buffer
	 myGBuffer.Draw(myCamera, myShader, PrepareShader, windowSize, myShadowMaps, myCubeMap);

	 // Unbind the G-buffer after rendering
	 myGBuffer.Unbind();
	*/
	class FUSIONFRAME_EXPORT GeometryBuffer
	{
	public:
		GeometryBuffer(int width, int height, bool EnableHighPrecisionPositionBuffer = true);
		
		GLuint GetAlbedoSpecularPass() { return AlbedoSpecularPass; };
		GLuint GetFBO() { return fbo; };
		GLuint GetNormalMetalicPass() { return NormalMetalicPass; };
		GLuint GetPositionDepthPass() { return PositionDepthPass; };
		GLuint GetDecalNormalPass() { return DecalNormalPass; };
		glm::ivec2 GetFBOSize() { return FBOSize; };

		//Set the drawbuffer mode so it would draw into all attachments 
		void SetDrawModeDefault();
		//Set the drawbuffer mode so it would draw into other attachments excluding default normal and world-space position attachment
		void SetDrawModeDecalPass();
		//Set the drawbuffer mode so it would draw into all attachments excluding decal normal attachment
		void SetDrawModeDefaultRestricted();

		void Bind();
		void Unbind();

		void DrawSceneDeferred(Camera3D& camera, Shader& shader, std::function<void()> ShaderPrep, const glm::ivec2& WindowSize, std::vector<OmniShadowMap*> &ShadowMaps, CubeMap& cubeMap, glm::vec4 BackgroundColor = glm::vec4(0.0f), float EnvironmentAmbientAmount = 0.2f);
		void DrawSSR(Camera3D& camera, Shader& shader, std::function<void()> ShaderPrep, Vec2<int> WindowSize);
		//If there are decals using this Geometry buffer , call this .
		//It coppies the
		void DrawDecalNormalPass();
		void clean();

	private:

		GLuint fbo, AlbedoSpecularPass, NormalMetalicPass, rbo, PositionDepthPass,DecalNormalPass, MetalicRoughnessPass;
		glm::ivec2 FBOSize;
		int ID;
	};
	/**
	 * Copies depth information from one framebuffer to another.
	 *
	 * Copies depth information from the source framebuffer to the destination framebuffer.
	 *
	 * @param src The ID of the source framebuffer.
	 * @param srcSize The size of the source framebuffer.
	 * @param dest The ID of the destination framebuffer.
	 */
	FUSIONFRAME_EXPORT_FUNCTION void CopyDepthInfoFBOtoFBO(GLuint src, glm::vec2 srcSize, GLuint dest);
	/**
	 * Reads the color of a pixel from a specified framebuffer attachment.
	 *
	 * Reads the color of a pixel at the specified coordinates from the specified framebuffer attachment.
	 *
	 * @param Xcoord The x-coordinate of the pixel.
	 * @param Ycoord The y-coordinate of the pixel.
	 * @param FramebufferAttachmentMode The framebuffer attachment mode.
	 * @param AttachmentFormat The format of the attachment.
	 * @param CurrentWindowSize The current size of the window.
	 * @return The color of the pixel.
	 */
	FUSIONFRAME_EXPORT_FUNCTION Color ReadFrameBufferPixel(int Xcoord, int Ycoord, unsigned int FramebufferAttachmentMode, GLenum AttachmentFormat, glm::vec2 CurrentWindowSize);
	FUSIONFRAME_EXPORT_FUNCTION void SaveFrameBufferImage(const int& width, const int& height,const char* path, const GLenum& Attachment);
}


