#include "Mesh.h"

void WorldTransform::SetModelMatrixUniformLocation(GLuint shader, const char* uniform)
{
	glUniformMatrix4fv(glGetUniformLocation(shader, uniform), 1, GL_FALSE, glm::value_ptr(ModelMatrix));
}

void WorldTransform::Translate(glm::vec3 v)
{
	ModelMatrix = glm::translate(ModelMatrix, v);

	Position.x = ModelMatrix[3][0];
	Position.y = ModelMatrix[3][1];
	Position.z = ModelMatrix[3][2];
}

void WorldTransform::Scale(glm::vec3 v)
{
	ModelMatrix = glm::scale(ModelMatrix, v);
}

void WorldTransform::Rotate(glm::vec3 v , float angle)
{
	ModelMatrix = glm::rotate(ModelMatrix, glm::radians(angle), v);
}

TextureObj::TextureObj()
{
	ObjectBuffer.Bind();

	float quadVertices[] = {

		-1.0f,  1.0f,  0.0f,    1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f,    0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
		 1.0f, -1.0f,  0.0f,    0.0f, 0.0f, 1.0f,   1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f,    1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
		 1.0f, -1.0f,  0.0f,    0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
		 1.0f,  1.0f,  0.0f,    1.0f, 1.0f, 0.0f,   1.0f, 1.0f
	};

	ObjectBuffer.BufferDataFill(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	ObjectBuffer.AttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	ObjectBuffer.AttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	ObjectBuffer.AttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	ObjectBuffer.Unbind();
}

TextureObj::~TextureObj()
{
}

void TextureObj::Draw(Camera2D& camera, GLuint shader , Texture2D& texture)
{
	glUseProgram(shader);
	ObjectBuffer.BindVAO();

	transformation.SetModelMatrixUniformLocation(shader, "model");

	camera.SetProjMatrixUniformLocation(shader, "proj");
	camera.SetRatioMatrixUniformLocation(shader, "ratioMat");

	texture.Bind(0, shader, "texture0");

	glDrawArrays(GL_TRIANGLES, 0, 6);

	ObjectBuffer.UnbindVAO();
	texture.Unbind();
	glUseProgram(0);
}

void TextureObj::Draw(Camera2D& camera, GLuint shader, Texture2D& texture, std::function<void()> ShaderPreperations)
{
	glUseProgram(shader);
	ObjectBuffer.BindVAO();

	transformation.SetModelMatrixUniformLocation(shader, "model");

	camera.SetProjMatrixUniformLocation(shader, "proj");
	camera.SetRatioMatrixUniformLocation(shader, "ratioMat");

	ShaderPreperations();

	texture.Bind(0, shader, "texture0");

	glDrawArrays(GL_TRIANGLES, 0, 6);

	ObjectBuffer.UnbindVAO();
	texture.Unbind();
	glUseProgram(0);
}

Mesh3D::Mesh3D(std::vector<Vertex> vertices_i, std::vector<unsigned int> indices_i)
{
	this->vertices = vertices_i;
	this->indices = indices_i;

	ObjectBuffer.Bind();

	ObjectBuffer.BufferDataFill(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
	ObjectBuffer.BufferDataFill(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	ObjectBuffer.AttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	ObjectBuffer.AttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,Normal));
	ObjectBuffer.AttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	ObjectBuffer.AttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	ObjectBuffer.AttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

	ObjectBuffer.Unbind();
}

void Mesh3D::Draw(Camera3D& camera, GLuint shader, std::function<void()> ShaderPreperations)
{
	ObjectBuffer.BindVAO();
	glUseProgram(shader);
	ShaderPreperations();

	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);

	glUseProgram(0);
	ObjectBuffer.UnbindVAO();
}
