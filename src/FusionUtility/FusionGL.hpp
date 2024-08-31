#pragma once
#include "Log.h"
#include "VectorMath.h"
#include "FusionDLLExport.h"
#include "Definitions.hpp"

namespace FUSIONUTIL
{
	//Opengl related helper functions

	FUSIONFRAME_EXPORT_FUNCTION void GLClearColor(glm::vec4 color);
	FUSIONFRAME_EXPORT_FUNCTION void GLClear(GLbitfield BitMask);
	FUSIONFRAME_EXPORT_FUNCTION void GLviewport(GLint x, GLint y, GLsizei width, GLsizei height);

	FUSIONFRAME_EXPORT_FUNCTION void ClearFrameBuffer(GLint x, GLint y, GLsizei width, GLsizei height, glm::vec4 color);
	
	FUSIONFRAME_EXPORT_FUNCTION void GLBindFrameBuffer(GLenum target, GLuint framebuffer);

	FUSIONFRAME_EXPORT_FUNCTION void GLPolygonMode(GLenum face, GLenum mode);

	FUSIONFRAME_EXPORT_FUNCTION GLuint CreateVBO(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
	FUSIONFRAME_EXPORT_FUNCTION void BindVBO(GLuint vbo, GLenum target);
	FUSIONFRAME_EXPORT_FUNCTION void FillVBOData(GLuint vbo, GLsizeiptr size, const void* data, GLenum usage);

	FUSIONFRAME_EXPORT_FUNCTION GLuint CreateEBO(GLsizeiptr size, const void* data, GLenum usage);
	FUSIONFRAME_EXPORT_FUNCTION void BindEBO(GLuint ebo);
	FUSIONFRAME_EXPORT_FUNCTION void FillEBOData(GLuint ebo, GLsizeiptr size, const void* data, GLenum usage);

	FUSIONFRAME_EXPORT_FUNCTION GLuint CreateFramebuffer();
	FUSIONFRAME_EXPORT_FUNCTION void AttachTextureToFramebuffer(GLuint framebuffer, GLuint texture, GLenum attachment, GLenum target = FF_TEXTURE_TARGET_GL_TEXTURE_2D);

	FUSIONFRAME_EXPORT_FUNCTION void BindTexture(GLuint texture, GLenum target = FF_TEXTURE_TARGET_GL_TEXTURE_2D, GLenum unit = FF_TEXTURE_UNIT_GL_TEXTURE0);
	FUSIONFRAME_EXPORT_FUNCTION GLuint CreateTexture2D(GLint internalFormat, GLint level, GLsizei width, GLsizei height, GLint Border, GLenum format, GLenum type, const void* data);
	FUSIONFRAME_EXPORT_FUNCTION void GLdeleteTextures(GLsizei n, const GLuint* texture);
	FUSIONFRAME_EXPORT_FUNCTION void SetTextureParameters(GLuint texture, GLenum wrapS, GLenum wrapT, GLenum minFilter, GLenum magFilter);

	FUSIONFRAME_EXPORT_FUNCTION void EnableDepthTest();
	FUSIONFRAME_EXPORT_FUNCTION void DisableDepthTest();
}