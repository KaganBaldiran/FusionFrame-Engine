#pragma once
#include "../FusionUtility/VectorMath.h"
#include "../FusionUtility/Log.h"
#include <string>
#include "../FusionUtility/FusionDLLExport.h"
#include "../FusionUtility/Definitions.hpp"
#include "Shader.h"
#define FF_TEXTURE_SUCCESS 1
#define FF_TEXTURE_ERROR -1
#define FF_TEXTURE_UNINITIALIZED 0

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
		Texture2D();
		Texture2D(const unsigned char* data,const int& Width, const int& Height,const int& ChannelCount,const char* FilePath = 0, GLuint Wrap_S_filter = FF_TEXTURE_WRAP_MODE_GL_CLAMP_TO_EDGE, GLuint Wrap_T_filter = FF_TEXTURE_WRAP_MODE_GL_CLAMP_TO_EDGE, GLenum TextureType = FF_TEXTURE_TARGET_GL_TEXTURE_2D,
			GLenum PixelType = FF_DATA_TYPE_GL_UNSIGNED_BYTE, GLuint Mag_filter = FF_TEXTURE_FILTER_MODE_GL_LINEAR, GLuint Min_filter = FF_TEXTURE_FILTER_MODE_GL_LINEAR_MIPMAP_LINEAR, bool Flip = true);
		Texture2D(const char* filePath, GLuint Wrap_S_filter = FF_TEXTURE_WRAP_MODE_GL_CLAMP_TO_EDGE, GLuint Wrap_T_filter = FF_TEXTURE_WRAP_MODE_GL_CLAMP_TO_EDGE,GLenum TextureType = FF_TEXTURE_TARGET_GL_TEXTURE_2D, 
			     GLenum PixelType = FF_DATA_TYPE_GL_UNSIGNED_BYTE,GLuint Mag_filter = FF_TEXTURE_FILTER_MODE_GL_LINEAR, GLuint Min_filter = FF_TEXTURE_FILTER_MODE_GL_LINEAR_MIPMAP_LINEAR,bool Flip = true);
		Texture2D(const GLuint SourceTexture, const GLenum SourceTextureInternalFormat, const glm::vec2 SourceTextureSize, 
			      const char* SourceTextureFilePath,GLenum texturetype, GLenum pixeltype, GLenum MAG_FILTER, GLenum MIN_FILTER);
		
		~Texture2D();

		void Clear();
		void Bind(const GLuint& slot, const GLuint& shader, const char* uniform);
		void Bind(GLenum target);
		void BindTextureBuffer(const GLuint& slot, const GLuint& shader, const char* uniform);
		//Used to recieve handle for the texture in order to make it bindless.
		//Requires ARB bindless texture extention. 
		//For more information: https://registry.khronos.org/OpenGL/extensions/ARB/ARB_bindless_texture.txt
		void MakeBindless();
		//Makes a bindless texture resident to prepare for rendering.
		//If a texture is already resident , no need to make it resident unless you explicitly made it non-resident afterwards.
		//All altering operations on the texture must be done before the texture is made resident.
		void MakeResident();
		//Makes a bindless texture non-resident to allow altering operations.
		void MakeNonResident();
		void InvalidateBindlessHandle();
		//Returns if the texture is already made resident
		inline bool IsTextureResident() { return IsResident; };
		//Sends the bindless texture to a shader sampler.
		void SendBindlessHandle(GLuint Shader, std::string Uniform);
		void Unbind();

		//Renders specified amount of mipmaps.
		//Mipmap sizes decreases logarithmically. 
		void RenderMipmaps(const uint& MipMapCount,Shader& MipmapShader);

		GLuint GetTexture();
		int GetWidth();
		int GetHeight();
		std::string GetFilePath();
		int GetChannelCount();
		GLenum GetInternalFormat();
		GLenum GetPixelType();
		GLenum GetTextureType();
		GLuint64 GetTextureHandle();
		inline const int GetTextureState() { return this->TextureState; };

		std::string PbrMapType;

	private:
		GLuint id;
		int width, height, channels;
		std::string PathData;
		int TextureState;
		bool IsResident;
		GLenum PixelType , InternalFormat , TextureType;
		GLuint64 TextureHandle;
	};
}
