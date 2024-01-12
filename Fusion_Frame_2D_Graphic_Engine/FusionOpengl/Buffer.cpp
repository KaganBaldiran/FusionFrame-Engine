#include "Buffer.h"

FUSIONOPENGL::Buffer::Buffer()
{
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);
}

void FUSIONOPENGL::Buffer::clean()
{
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

void FUSIONOPENGL::Buffer::Bind()
{
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
}

void FUSIONOPENGL::Buffer::BindVAO()
{
	glBindVertexArray(vao);
}

void FUSIONOPENGL::Buffer::UnbindVAO()
{
	glBindVertexArray(0);
}

void FUSIONOPENGL::Buffer::Unbind()
{
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void FUSIONOPENGL::Buffer::BufferDataFill(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
	glBufferData(target, size, data, usage);
}

void FUSIONOPENGL::Buffer::AttribPointer(GLuint index , GLuint size , GLenum type , GLboolean normalized , GLsizei stride,const void* pointer)
{
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

FUSIONOPENGL::Buffer3D::Buffer3D()
{
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glGenVertexArrays(1, &vao);
}

void FUSIONOPENGL::Buffer3D::clean()
{
	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

void FUSIONOPENGL::Buffer3D::BindEBO()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
}

void FUSIONOPENGL::Buffer3D::Unbind()
{
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

FUSIONOPENGL::VBO::VBO()
{
	glGenBuffers(1, &vbo);
}

GLuint FUSIONOPENGL::VBO::Bind()
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	return vbo;
}

GLuint FUSIONOPENGL::VBO::GetBufferID()
{
	return vbo;
}

FUSIONOPENGL::VBO::~VBO()
{
	glDeleteBuffers(1, &vbo);
}

FUSIONOPENGL::VAO::VAO()
{
	glGenVertexArrays(1, &vao);

}

GLuint FUSIONOPENGL::VAO::Bind()
{
	glBindVertexArray(vao);
	return vao;
}

GLuint FUSIONOPENGL::VAO::GetVertexArrID()
{
	return vao;
}

FUSIONOPENGL::VAO::~VAO()
{
	glDeleteVertexArrays(1, &vao);

}

void FUSIONOPENGL::BindVBONull()
{
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
}

void FUSIONOPENGL::BindVAONull()
{
	glBindVertexArray(NULL);
}

void FUSIONOPENGL::BindEBONull()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
}

FUSIONOPENGL::EBO::EBO()
{
	glGenBuffers(1, &ebo);
}

GLuint FUSIONOPENGL::EBO::Bind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	return ebo;
}

GLuint FUSIONOPENGL::EBO::GetEBOID()
{
	return ebo;
}

FUSIONOPENGL::EBO::~EBO()
{
	glDeleteBuffers(1, &ebo);
}



