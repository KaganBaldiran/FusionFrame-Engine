#include "FusionGL.hpp"
#include <glew.h>
#include <glfw3.h>

void FUSIONUTIL::GLClearColor(glm::vec4 color)
{
	glClearColor(color.x, color.y, color.z, color.w);
}

void FUSIONUTIL::GLClear(GLbitfield BitMask)
{
	glClear(BitMask);
}

void FUSIONUTIL::GLviewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    glViewport(x, y, width, height);
}

void FUSIONUTIL::ClearFrameBuffer(GLint x, GLint y, GLsizei width, GLsizei height, glm::vec4 color)
{
    FUSIONUTIL::GLClear(FF_CLEAR_BUFFER_BIT_GL_COLOR_BUFFER_BIT | FF_CLEAR_BUFFER_BIT_GL_DEPTH_BUFFER_BIT);
    FUSIONUTIL::GLClearColor(color);
    FUSIONUTIL::GLviewport(x,y,width,height);
}

void FUSIONUTIL::GLBindFrameBuffer(GLenum target, GLuint framebuffer)
{
	glBindFramebuffer(target, framebuffer);
}

void FUSIONUTIL::GLPolygonMode(GLenum face, GLenum mode)
{
	glPolygonMode(face, mode);
}

GLuint FUSIONUTIL::CreateVBO(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
	return GLuint();
}

void FUSIONUTIL::BindVBO(GLuint vbo, GLenum target)
{
    glBindBuffer(target, vbo);
}

void FUSIONUTIL::FillVBOData(GLuint vbo, GLsizeiptr size, const void* data, GLenum usage)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

GLuint FUSIONUTIL::CreateEBO(GLsizeiptr size, const void* data, GLenum usage)
{
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
    return ebo;
}

void FUSIONUTIL::BindEBO(GLuint ebo)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
}

void FUSIONUTIL::FillEBOData(GLuint ebo, GLsizeiptr size, const void* data, GLenum usage)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
}

GLuint FUSIONUTIL::CreateFramebuffer()
{
    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    return framebuffer;
}

void FUSIONUTIL::AttachTextureToFramebuffer(GLuint framebuffer, GLuint texture, GLenum attachment, GLenum target)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target, texture, 0);
}

void FUSIONUTIL::BindTexture(GLuint texture, GLenum target, GLenum unit)
{
    glActiveTexture(unit);
    glBindTexture(target, texture);
}

void FUSIONUTIL::EnableDepthTest()
{
    glEnable(GL_DEPTH_TEST);
}

void FUSIONUTIL::DisableDepthTest()
{
    glDisable(GL_DEPTH_TEST);
}



