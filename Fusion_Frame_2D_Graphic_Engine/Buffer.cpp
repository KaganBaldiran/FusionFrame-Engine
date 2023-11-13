#include "Buffer.h"

Buffer::Buffer()
{
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);
}

Buffer::~Buffer()
{
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

void Buffer::Bind()
{
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
}

void Buffer::BindVAO()
{
	glBindVertexArray(vao);
}

void Buffer::UnbindVAO()
{
	glBindVertexArray(0);
}

void Buffer::Unbind()
{
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Buffer::BufferDataFill(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
	glBufferData(target, size, data, usage);
}

void Buffer::AttribPointer(GLuint index , GLuint size , GLenum type , GLboolean normalized , GLsizei stride,const void* pointer)
{
	glVertexAttribPointer(index, size, type, normalized, stride, pointer);
	glEnableVertexAttribArray(index);
}

Buffer3D::Buffer3D()
{
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glGenVertexArrays(1, &vao);
}

Buffer3D::~Buffer3D()
{
	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

void Buffer3D::Bind()
{
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
}

void Buffer3D::Unbind()
{
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


