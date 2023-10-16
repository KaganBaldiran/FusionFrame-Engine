#pragma once
#include <glew.h>
#include <glfw3.h>
#include "Log.h"
#include "VectorMath.h"
#include "Buffer.h"
#include "Camera.h"
#include "Texture.h"

class Mesh
{
public:
	Mesh();
	~Mesh();
	void SetModelMatrixUniformLocation(GLuint shader, const char* uniform);
	void Translate(glm::vec3 v);
	void Scale(glm::vec3 v);
	void Rotate(glm::vec3 v, float angle);

protected:
	Buffer ObjectBuffer;
	glm::mat4 ModelMatrix;
};

class TextureObj : public Mesh
{
public:

	TextureObj();
	~TextureObj();

	void Draw(Camera2D& camera, GLuint shader, Texture2D& texture);

protected:

};



