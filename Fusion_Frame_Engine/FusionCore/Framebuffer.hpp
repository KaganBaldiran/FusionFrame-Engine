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

		void Draw(Camera3D& camera, Shader& shader,std::function<void()> ShaderPrep, Vec2<int> WindowSize, bool DOFenabled = false, float DOFdistanceFar = 0.09f, float DOFdistanceClose = 0.02f, float DOFintensity = 1.0f, float Gamma = 0.9f, float Exposure = 1.0f);
		void clean();
		
	private:

		GLuint fbo, fboImage, fboDepth ,IDtexture, rbo , SSRtexture;
		Buffer ObjectBuffer;
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
	class FUSIONFRAME_EXPORT Gbuffer
	{
	public:
		Gbuffer(int width, int height);
		
		GLuint GetAlbedoSpecularPass() { return AlbedoSpecularPass; };
		GLuint GetFBO() { return fbo; };
		GLuint GetNormalMetalicPass() { return NormalMetalicPass; };
		GLuint GetPositionDepthPass() { return PositionDepthPass; };
		glm::ivec2 GetFBOSize() { return FBOSize; };

		void Bind();
		void Unbind();

		void DrawSceneDeferred(Camera3D& camera, Shader& shader, std::function<void()> ShaderPrep, Vec2<int> WindowSize, std::vector<OmniShadowMap*> &ShadowMaps, CubeMap& cubeMap, glm::vec4 BackgroundColor = glm::vec4(0.0f), float EnvironmentAmbientAmount = 0.2f);
		void DrawSSR(Camera3D& camera, Shader& shader, std::function<void()> ShaderPrep, Vec2<int> WindowSize);
		void clean();

	private:

		GLuint fbo, AlbedoSpecularPass, NormalMetalicPass, rbo, PositionDepthPass , MetalicRoughnessPass;
		Buffer ObjectBuffer;
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
}


