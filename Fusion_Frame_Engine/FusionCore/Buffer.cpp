#include "Buffer.h"

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
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
}

void FUSIONCORE::BindVAONull()
{
	glBindVertexArray(NULL);
}

void FUSIONCORE::BindEBONull()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
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



