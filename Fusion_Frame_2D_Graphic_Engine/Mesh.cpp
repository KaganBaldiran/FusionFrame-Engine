#include "Mesh.h"

void FUSIONOPENGL::WorldTransform::SetModelMatrixUniformLocation(GLuint shader, const char* uniform)
{
	glUniformMatrix4fv(glGetUniformLocation(shader, uniform), 1, GL_FALSE, glm::value_ptr(ModelMatrix));
}

void FUSIONOPENGL::WorldTransform::Translate(glm::vec3 v)
{
	ModelMatrix = glm::translate(ModelMatrix, v);
	TransformAction action;
	action.Transformation = v;
	this->LastTransforms.push_back(action);

	Position.x = ModelMatrix[3][0];
	Position.y = ModelMatrix[3][1];
	Position.z = ModelMatrix[3][2];
}

void FUSIONOPENGL::WorldTransform::Scale(glm::vec3 v)
{
	ModelMatrix = glm::scale(ModelMatrix, v);
	ObjectScales *= v;

	ScaleAction action;
	action.Scale = v;
	this->LastScales.push_back(action);
}

void FUSIONOPENGL::WorldTransform::Rotate(glm::vec3 v , float angle)
{
	ModelMatrix = glm::rotate(ModelMatrix, glm::radians(angle), v);
	RotateAction action;
	action.Degree = angle;
	action.Vector = v;
	this->LastRotations.push_back(action);
}

FUSIONOPENGL::TextureObj::TextureObj()
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

FUSIONOPENGL::TextureObj::~TextureObj()
{
	this->ObjectBuffer.clean();
}

void FUSIONOPENGL::TextureObj::Draw(Camera2D& camera, GLuint shader , Texture2D& texture)
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

void FUSIONOPENGL::TextureObj::Draw(Camera3D& camera, GLuint shader, Texture2D& texture, std::function<void()> ShaderPreperations)
{
	glUseProgram(shader);
	ObjectBuffer.BindVAO();

	transformation.SetModelMatrixUniformLocation(shader, "model");

	camera.SetProjMatrixUniformLocation(shader, "proj");
	camera.SetRatioMatrixUniformLocation(shader, "ratioMat");
	camera.SetViewMatrixUniformLocation(shader, "view");

	ShaderPreperations();

	texture.Bind(0, shader, "texture0");

	glDrawArrays(GL_TRIANGLES, 0, 6);

	ObjectBuffer.UnbindVAO();
	texture.Unbind();
	glUseProgram(0);
}

FUSIONOPENGL::Mesh3D::Mesh3D(std::vector<FUSIONOPENGL::Vertex> &vertices_i, std::vector<unsigned int> &indices_i, std::vector<Texture2D>& textures_i)
{
	this->vertices.assign(vertices_i.begin(), vertices_i.end());
	this->indices.assign(indices_i.begin(), indices_i.end());
	this->textures.assign(textures_i.begin(), textures_i.end());

	ObjectBuffer.Bind();

	ObjectBuffer.BufferDataFill(GL_ARRAY_BUFFER, vertices.size() * sizeof(FUSIONOPENGL::Vertex), &vertices[0], GL_STATIC_DRAW);
	ObjectBuffer.BindEBO();
	ObjectBuffer.BufferDataFill(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	ObjectBuffer.AttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONOPENGL::Vertex), (void*)0);
	ObjectBuffer.AttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONOPENGL::Vertex), (void*)offsetof(Vertex,Normal));
	ObjectBuffer.AttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(FUSIONOPENGL::Vertex), (void*)offsetof(Vertex, TexCoords));
	ObjectBuffer.AttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONOPENGL::Vertex), (void*)offsetof(Vertex, Tangent));
	ObjectBuffer.AttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(FUSIONOPENGL::Vertex), (void*)offsetof(Vertex, Bitangent));

	ObjectBuffer.Unbind();
}

void FUSIONOPENGL::Mesh3D::Draw(Camera3D& camera, Shader& shader, std::function<void()> &ShaderPreperations)
{
	shader.use();
	ObjectBuffer.BindVAO();
    ShaderPreperations();

	camera.SetProjMatrixUniformLocation(shader.GetID(), "proj");
	camera.SetRatioMatrixUniformLocation(shader.GetID(), "ratioMat");
	camera.SetViewMatrixUniformLocation(shader.GetID(), "view");
	shader.setVec3("CameraPos", camera.Position);
	shader.setFloat("FarPlane", camera.FarPlane);
	shader.setFloat("NearPlane", camera.NearPlane);

	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);

	UseShaderProgram(0);
	ObjectBuffer.UnbindVAO();

}

void FUSIONOPENGL::Mesh3D::Draw(Camera3D& camera, Shader& shader, Material& material, std::function<void()>& ShaderPreperations)
{
	shader.use();
	ObjectBuffer.BindVAO();
	ShaderPreperations();

	camera.SetProjMatrixUniformLocation(shader.GetID(), "proj");
	camera.SetRatioMatrixUniformLocation(shader.GetID(), "ratioMat");
	camera.SetViewMatrixUniformLocation(shader.GetID(), "view");
	shader.setVec3("CameraPos", camera.Position);
	shader.setFloat("FarPlane", camera.FarPlane);
	shader.setFloat("NearPlane", camera.NearPlane);
	material.SetMaterialShader(shader);

	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);

	UseShaderProgram(0);
	ObjectBuffer.UnbindVAO();
}
