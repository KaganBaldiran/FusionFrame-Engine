#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glew.h>
#include <glfw3.h>
#include "Framebuffer.hpp"
#include <FreeImage.h>

FUSIONCORE::Texture2D::Texture2D()
{
	width = 0;
	height = 0;
	TextureHandle = 0;
	TextureState = FF_TEXTURE_UNINITIALIZED;  
	channels = 0;
	glGenTextures(1, &id);
	IsResident = false;
}

FUSIONCORE::Texture2D::Texture2D(const unsigned char* data, const int& Width, const int& Height, const int& ChannelCount, const char* FilePath,
	                             GLuint Wrap_S_filter, GLuint Wrap_T_filter, GLenum TextureType, GLenum PixelType, GLuint Mag_filter, GLuint Min_filter, bool Flip)
{
	PathData = std::string(FilePath);
	this->PixelType = PixelType;
	this->TextureType = TextureType;
	TextureHandle = 0;
	IsResident = false;

	glGenTextures(1, &id);
	glBindTexture(TextureType, id);

	if (!data)
	{
		TextureState = FF_TEXTURE_ERROR;
		LOG_ERR("Error importing the texture! :: " << FilePath);
		return;
	}

	channels = ChannelCount;
	width = Width;
	height = Height;

	GLenum format;
	if (channels == 1)
	{
		format = GL_RED;
	}
	else if (channels == 2)
	{
		format = GL_RG;
	}
	else if (channels == 3)
	{
		format = GL_RGB;
	}
	else if (channels == 4)
	{
		format = GL_RGBA;
	}
	else
	{
		TextureState = FF_TEXTURE_ERROR;
		LOG_ERR("Unrecognised texture format! :: " << FilePath);
		return;
	}

	this->InternalFormat = format;

	glTexImage2D(TextureType, 0, format, width, height, 0, format, PixelType, data);
	glTextureParameteri(TextureType, GL_TEXTURE_MAG_FILTER, Mag_filter);
	glTextureParameteri(TextureType, GL_TEXTURE_MIN_FILTER, Min_filter);
	glTextureParameteri(TextureType, GL_TEXTURE_WRAP_S, Wrap_S_filter);
	glTextureParameteri(TextureType, GL_TEXTURE_WRAP_T, Wrap_T_filter);

	glGenerateMipmap(TextureType);

	glBindTexture(TextureType, 0);

	LOG_INF("Texture Imported : " << PathData);
	TextureState = FF_TEXTURE_SUCCESS;
}

FUSIONCORE::Texture2D::Texture2D(const char* filePath,GLuint Wrap_S_filter,GLuint Wrap_T_filter,GLenum TextureType , GLenum PixelType , GLuint Mag_filter , GLuint Min_filter, bool Flip)
{
	PathData = std::string(filePath);
	this->PixelType = PixelType;
	this->TextureType = TextureType;
	TextureHandle = 0;
	IsResident = false;

	glGenTextures(1, &id);
	glBindTexture(TextureType, id);

	bool IsDDS = PathData.find(".dds") != std::string::npos;

	unsigned char* data;
	FIBITMAP* bitmap;

	if (!IsDDS)
	{
		stbi_set_flip_vertically_on_load(Flip);
		data = stbi_load(filePath, &width, &height, &channels, NULL);
	}
	else
	{
		bitmap = FreeImage_Load(FIF_DDS,filePath, DDS_DEFAULT);
		data = FreeImage_GetBits(bitmap);

		width = FreeImage_GetWidth(bitmap);
	    height = FreeImage_GetHeight(bitmap);
		int bpp = FreeImage_GetBPP(bitmap);
		channels = (bpp == 32) ? 4 : ((bpp == 24) ? 3 : 1);
	}

	if (!data || (IsDDS && !bitmap))
	{
		TextureState = FF_TEXTURE_ERROR;
		LOG_ERR("Error importing the texture! :: " << filePath);
		return;
	}

	GLenum format;
	if (channels == 1)
	{
		format = GL_RED;
	}
	else if (channels == 2)
	{
		format = GL_RG;
	}
	else if (channels == 3)
	{
		format = GL_RGB;
	}
	else if (channels == 4)
	{
		format = GL_RGBA;
	}
	else
	{
		TextureState = FF_TEXTURE_ERROR;
		LOG_ERR("Unrecognised texture format! :: " << filePath);
		return;
	}

	this->InternalFormat = format;

	glTexImage2D(TextureType, 0, format, width, height, 0, format, PixelType, data);
	glTextureParameteri(TextureType, GL_TEXTURE_MAG_FILTER, Mag_filter);
	glTextureParameteri(TextureType, GL_TEXTURE_MIN_FILTER, Min_filter);
	glTextureParameteri(TextureType, GL_TEXTURE_WRAP_S, Wrap_S_filter);
	glTextureParameteri(TextureType, GL_TEXTURE_WRAP_T, Wrap_T_filter);

	glGenerateMipmap(TextureType);

	glBindTexture(TextureType, 0);

	if (IsDDS && bitmap)
	{
		FreeImage_Unload(bitmap);
	}
	else
	{
		stbi_image_free(data);

		if (Flip)
		{
			stbi_set_flip_vertically_on_load(false);
		}
	}

	LOG_INF("Texture Imported : " << PathData);
	TextureState = FF_TEXTURE_SUCCESS;
}

FUSIONCORE::Texture2D::Texture2D(const GLuint SourceTexture, const GLenum SourceTextureInternalFormat, const glm::vec2 SourceTextureSize, const char* SourceTextureFilePath
	, GLenum texturetype, GLenum pixeltype, GLenum MAG_FILTER, GLenum MIN_FILTER)
{
	glGenTextures(1, &this->id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(texturetype, this->id);

	glTexParameteri(texturetype, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(texturetype, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(texturetype, GL_TEXTURE_MIN_FILTER, MIN_FILTER);
	glTexParameteri(texturetype, GL_TEXTURE_MAG_FILTER, MAG_FILTER);

	glTexImage2D(texturetype, 0, SourceTextureInternalFormat, SourceTextureSize.x, SourceTextureSize.y, 0, SourceTextureInternalFormat, pixeltype, (void*)0);
	glGenerateMipmap(texturetype);

	glCopyImageSubData(
		SourceTexture, texturetype, 0, 0, 0, 0,
		this->id, texturetype, 0, 0, 0, 0,
		SourceTextureSize.x, SourceTextureSize.y, 1
	);

	glBindTexture(texturetype, 0);
	this->width = SourceTextureSize.x;
	this->height = SourceTextureSize.y;

	LOG_INF("Texture was copied from texture[ID:" << SourceTexture << "] to texture[ID:" << this->id << "]");
	TextureState = FF_TEXTURE_SUCCESS;
	TextureHandle = 0;
	IsResident = false;
}

FUSIONCORE::Texture2D::~Texture2D()
{
	Clear();
}

void FUSIONCORE::Texture2D::Clear()
{
	if (TextureHandle != 0 && IsResident) glMakeImageHandleNonResidentARB(TextureHandle);
	glDeleteTextures(1, &id);
	LOG_INF("Texture Cleaned : " << PathData);
}

GLuint FUSIONCORE::Texture2D::GetTexture()
{
	return this->id;
}

int FUSIONCORE::Texture2D::GetWidth()
{
	return this->width;
}

int FUSIONCORE::Texture2D::GetHeight()
{
	return this->height;
}

void FUSIONCORE::Texture2D::Bind(const GLuint& slot ,const GLuint& shader,const char* uniform)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, id);
	glUniform1i(glGetUniformLocation(shader, uniform), slot);
}

void FUSIONCORE::Texture2D::MakeBindless()
{
	if (TextureHandle > 0) return; 
	if (!this->id) throw FFexception("Uninitialized textures cannot be altered!"); 

	TextureHandle = glGetTextureHandleARB(this->id);
}

void FUSIONCORE::Texture2D::MakeResident()
{
	if (IsResident) return;
	if (TextureHandle == 0) throw FFexception("Non-bindless textures cannot be made resident");
	glMakeTextureHandleResidentARB(this->TextureHandle);
	IsResident = true; 
}

void FUSIONCORE::Texture2D::MakeNonResident()
{
	if (!IsResident) return;
	if (TextureHandle == 0) throw FFexception("Non-bindless textures cannot be made non-resident");
	glMakeTextureHandleNonResidentARB(this->TextureHandle);
	IsResident = false;
}

void FUSIONCORE::Texture2D::InvalidateBindlessHandle()
{
	TextureHandle = 0;
}

void FUSIONCORE::Texture2D::SendBindlessHandle(GLuint Shader, std::string Uniform)
{
	if (TextureHandle == 0 && !IsResident) throw FFexception("Non-bindless textures cannot be sent to shaders");

	GLint location = glGetUniformLocation(Shader, Uniform.c_str());
	glUniformHandleui64ARB(location, this->TextureHandle);
}

void FUSIONCORE::Texture2D::Unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
}

void FUSIONCORE::Texture2D::RenderMipmaps(const uint& MipMapCount,Shader& MipmapShader)
{
	static std::shared_ptr<Framebuffer> MipmapBuffer = std::make_shared<Framebuffer>();
	MipmapBuffer->Bind();
	MipmapShader.use();
	GetRectangleBuffer()->Bind();
	glDisable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,GetTexture());
	glUniform1i(glGetUniformLocation(MipmapShader.GetID(), "AlbedoTexture"), 1);

	for (size_t i = 0; i < MipMapCount; i++)
	{
		uint MipmapWidth = glm::max(1, width >> (i + 1));
		uint MipmapHeight = glm::max(1, height >> (i + 1));

		MipmapBuffer->AttachTexture2DTarget(GetTexture(),i + 1);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			LOG_ERR("Error completing the framebuffer while rendering mipmap");
		}

		glViewport(0, 0, MipmapWidth, MipmapHeight);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	BindVAONull();

	glEnable(GL_DEPTH_TEST);
	UseShaderProgram(0);
}

std::string FUSIONCORE::Texture2D::GetFilePath()
{
	return this->PathData;
}

int FUSIONCORE::Texture2D::GetChannelCount()
{
	return this->channels;
}

GLenum FUSIONCORE::Texture2D::GetInternalFormat()
{
	return this->InternalFormat;
}

GLenum FUSIONCORE::Texture2D::GetPixelType()
{
	return PixelType;
}

GLenum FUSIONCORE::Texture2D::GetTextureType()
{
	return TextureType;
}

GLuint64 FUSIONCORE::Texture2D::GetTextureHandle()
{
	return TextureHandle;
}

void FUSIONCORE::Texture2D::Bind(GLenum target)
{
	glBindTexture(target, id);
}

void FUSIONCORE::Texture2D::BindTextureBuffer(const GLuint& slot, const GLuint& shader, const char* uniform)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_BUFFER, id);
	glUniform1i(glGetUniformLocation(shader, uniform), slot);
}
