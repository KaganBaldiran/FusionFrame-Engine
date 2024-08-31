#include "Buffer.h"
#include <glew.h>
#include <glfw3.h>

FUSIONCORE::Buffer::Buffer()
{
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);
}

void FUSIONCORE::Buffer::clean()
{
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

void FUSIONCORE::Buffer::Bind()
{
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
}

void FUSIONCORE::Buffer::BindVAO()
{
	glBindVertexArray(vao);
}

void FUSIONCORE::Buffer::UnbindVAO()
{
	glBindVertexArray(0);
}

void FUSIONCORE::Buffer::Unbind()
{
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void FUSIONCORE::Buffer::BufferDataFill(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
	glBufferData(target, size, data, usage);
}

void FUSIONCORE::Buffer::AttribPointer(GLuint index , GLuint size , GLenum type , GLboolean normalized , GLsizei stride,const void* pointer)
{
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

void FUSIONCORE::Buffer::AttribIPointer(GLuint index, GLuint size, GLenum type, GLsizei stride, const void* pointer)
{
	glEnableVertexAttribArray(index);
	glVertexAttribIPointer(index, size, type, stride, pointer);
}

FUSIONCORE::Buffer3D::Buffer3D()
{
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glGenVertexArrays(1, &vao);
}

void FUSIONCORE::Buffer3D::clean()
{
	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

void FUSIONCORE::Buffer3D::BindEBO()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
}

void FUSIONCORE::Buffer3D::Unbind()
{
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

FUSIONCORE::VBO::VBO()
{
	glGenBuffers(1, &vbo);
	IsChanged = false;
}

GLuint FUSIONCORE::VBO::Bind()
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	return vbo;
}

GLuint FUSIONCORE::VBO::GetBufferID()
{
	return vbo;
}

FUSIONCORE::VBO::~VBO()
{
	glDeleteBuffers(1, &vbo);
}

FUSIONCORE::VAO::VAO()
{
	glGenVertexArrays(1, &vao);
}

GLuint FUSIONCORE::VAO::Bind()
{
	glBindVertexArray(vao);
	return vao;
}

GLuint FUSIONCORE::VAO::GetVertexArrID()
{
	return vao;
}

FUSIONCORE::VAO::~VAO()
{
	glDeleteVertexArrays(1, &vao);

}

void FUSIONCORE::BindVBONull()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void FUSIONCORE::BindVAONull()
{
	glBindVertexArray(0);
}

void FUSIONCORE::BindEBONull()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void FUSIONCORE::BindUBONull()
{
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void FUSIONCORE::BindSSBONull()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void FUSIONCORE::BindIndirectCommandBufferNull()
{
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
}

FUSIONCORE::EBO::EBO()
{
	glGenBuffers(1, &ebo);
}

GLuint FUSIONCORE::EBO::Bind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	return ebo;
}

GLuint FUSIONCORE::EBO::GetEBOID()
{
	return ebo;
}

FUSIONCORE::EBO::~EBO()
{
	glDeleteBuffers(1, &ebo);
}

FUSIONCORE::UBO::UBO()
{
	glGenBuffers(1, &ubo);
}

GLuint FUSIONCORE::UBO::Bind()
{
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	return ubo;
}

void FUSIONCORE::UBO::BindUBO(unsigned int BindingPoint)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, BindingPoint, this->ubo);
}

GLuint FUSIONCORE::UBO::GetUBOID()
{
	return ubo;
}

void FUSIONCORE::UBO::BufferDataFill(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
	glBufferData(target, size, data, usage);
}

FUSIONCORE::UBO::~UBO()
{
	glDeleteBuffers(1, &ubo);
}

void FUSIONCORE::SSBO::BindSSBO(unsigned int BindingPoint)
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BindingPoint, this->ssbo);
}

FUSIONCORE::SSBO::SSBO()
{
	glGenBuffers(1, &ssbo);
}

FUSIONCORE::SSBO::~SSBO()
{
	this->clean();
}

void FUSIONCORE::SSBO::clean()
{
	glDeleteBuffers(1, &this->ssbo);
}

void FUSIONCORE::SSBO::Bind()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
}

void FUSIONCORE::SSBO::Unbind()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void FUSIONCORE::SSBO::BufferDataFill(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
	glBufferData(target, size, data, usage);
}

FUSIONCORE::IndirectCommandBuffer::IndirectCommandBuffer()
{
	glGenBuffers(1, &this->icb);
}

GLuint FUSIONCORE::IndirectCommandBuffer::Bind()
{
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, icb);
	return icb;
}

GLuint FUSIONCORE::IndirectCommandBuffer::GetBufferID()
{
	return icb;
}

FUSIONCORE::IndirectCommandBuffer::~IndirectCommandBuffer()
{
	glDeleteBuffers(1, &icb);
}
