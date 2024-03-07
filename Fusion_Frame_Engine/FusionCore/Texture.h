#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/VectorMath.h"
#include "../FusionUtility/Log.h"
#include <string>
#define FF_TEXTURE_SUCCESS 1
#define FF_TEXTURE_ERROR -1

namespace FUSIONCORE
{
	class Texture2D
	{
	public:
		Texture2D() = default;
		Texture2D(const char* filePath, GLenum TextureType, GLenum PixelType, GLuint Mag_filter, GLuint Min_filter, GLuint Wrap_S_filter, GLuint Wrap_T_filter, bool Flip);
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
