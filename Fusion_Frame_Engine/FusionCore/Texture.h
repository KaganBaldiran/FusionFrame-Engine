#pragma once
#include "../FusionUtility/VectorMath.h"
#include "../FusionUtility/Log.h"
#include <string>
#include "../FusionUtility/FusionDLLExport.h"
#include "../FusionUtility/Definitions.hpp"
#define FF_TEXTURE_SUCCESS 1
#define FF_TEXTURE_ERROR -1

namespace FUSIONCORE
{
	/*
	 Represents a 2D texture object used for texture mapping in rendering.

	 The Texture2D class encapsulates the properties and functionality related to 2D textures.
	 It provides methods for loading, binding, and accessing texture properties such as width, height, and channels.

	 Key functionalities include:
	 - Loading textures from image files or existing OpenGL texture objects.
	 - Binding and unbinding textures to specific texture units.
	 - Retrieving texture properties such as width, height, channels, and file path.

	 Example usage:
	 // Create a 2D texture object from an image file
	 Texture2D texture("texture.jpg");

	 // Get the width and height of the texture
	 int width = texture.GetWidth();
	 int height = texture.GetHeight();

	 // Bind the texture to a specific texture unit and shader uniform
	 texture.Bind(0, shader, "textureSampler");

	 // Unbind the texture after use
	 texture.Unbind();
	*/
	class FUSIONFRAME_EXPORT Texture2D
	{
	public:
		Texture2D() = default;
		Texture2D(const char* filePath, GLuint Wrap_S_filter = FF_TEXTURE_WRAP_MODE_GL_CLAMP_TO_EDGE, GLuint Wrap_T_filter = FF_TEXTURE_WRAP_MODE_GL_CLAMP_TO_EDGE,GLenum TextureType = FF_TEXTURE_TARGET_GL_TEXTURE_2D, 
			     GLenum PixelType = FF_DATA_TYPE_GL_UNSIGNED_BYTE,GLuint Mag_filter = FF_TEXTURE_FILTER_MODE_GL_LINEAR, GLuint Min_filter = FF_TEXTURE_FILTER_MODE_GL_LINEAR,bool Flip = true);
		Texture2D(const GLuint SourceTexture, const GLenum SourceTextureInternalFormat, const glm::vec2 SourceTextureSize, 
			      const char* SourceTextureFilePath,GLenum texturetype, GLenum pixeltype, GLenum MAG_FILTER, GLenum MIN_FILTER);
		
		void Clear();
		GLuint GetTexture();
		int GetWidth();
		int GetHeight();
		void Bind(GLuint slot, GLuint shader, const char* uniform);
		void Unbind();
		std::string GetFilePath();
		int GetChannelCount();
		GLenum GetInternalFormat();
		GLenum GetPixelType();
		GLenum GetTextureType();
		inline const int GetTextureState() { return this->TextureState; };

		std::string PbrMapType;

	private:
		GLuint id;
		int width, height, channels;
		std::string PathData;
		int TextureState;
		GLenum PixelType , InternalFormat , TextureType;
	};
}
