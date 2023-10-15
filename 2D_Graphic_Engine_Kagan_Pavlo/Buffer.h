#pragma once
#include <glew.h>
#include <glfw3.h>

class Buffer
{
public:
	Buffer();
	~Buffer();

	void Bind();
	void BindVAO();
	void UnbindVAO();
	void Unbind();
	void BufferDataFill(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
	void AttribPointer(GLuint index, GLuint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);

private:

	GLuint vao, vbo;

};

