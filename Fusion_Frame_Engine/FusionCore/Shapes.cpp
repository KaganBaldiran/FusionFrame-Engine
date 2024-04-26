#include "Shapes.hpp"
#include <memory>

std::unique_ptr<FUSIONCORE::VBO> ShapesVBO;
std::unique_ptr<FUSIONCORE::EBO> ShapesEBO;
std::unique_ptr<FUSIONCORE::VAO> ShapesVAO;

unsigned int RectangleIndicesCount;
unsigned int TriangleIndicesCount;
unsigned int HexagonIndicesCount;

void FUSIONCORE::SHAPES::InitializeShapeBuffers()
{
	ShapesVBO = std::make_unique<FUSIONCORE::VBO>();
	ShapesVAO = std::make_unique<FUSIONCORE::VAO>();
	ShapesEBO = std::make_unique<FUSIONCORE::EBO>();

	ShapesVAO->Bind();
	ShapesVBO->Bind();

	float quadVertices[] = {
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	unsigned int quadIndices[] = {
	    0, 1, 2,  
	    0, 2, 3   
	};

	RectangleIndicesCount = sizeof(quadIndices) / sizeof(unsigned int);

	float TriangleVertices[] = {
		// positions   // texCoords
		 0.0f,  1.0f,  0.0f, 0.5f,
		 1.0f, -1.0f,  1.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f
	};


	unsigned int TriangleIndices[] = {
		4,5,6
	};

	TriangleIndicesCount = sizeof(TriangleIndices) / sizeof(unsigned int);

	float HexagonVertices[] = {
		// positions      // texture coordinates
		 0.0f,  1.0f,     0.5f, 1.0f,
		-0.866f,  0.5f,   0.0f, 0.75f,
		-0.866f, -0.5f,   0.0f, 0.25f,
		 0.0f, -1.0f,     0.5f, 0.0f,
		 0.866f, -0.5f,   1.0f, 0.25f,
		 0.866f,  0.5f,   1.0f, 0.75f
	};

	unsigned int HexagonIndices[] = {
	    7, 8, 9,  // Triangle 1
	    7, 9, 10,  // Triangle 2
	    7, 10, 11,  // Triangle 3
	    7, 11, 12   // Triangle 4
	};

	HexagonIndicesCount = sizeof(HexagonIndices) / sizeof(unsigned int);

	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices) + sizeof(TriangleVertices) + sizeof(HexagonVertices), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quadVertices), &quadVertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(quadVertices), sizeof(TriangleVertices), &TriangleVertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(quadVertices) + sizeof(TriangleVertices), sizeof(HexagonVertices), &HexagonVertices);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	ShapesEBO->Bind();

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices) + sizeof(TriangleIndices) + sizeof(HexagonIndices), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(quadIndices), &quadIndices);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), sizeof(TriangleIndices), &TriangleIndices);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices) + sizeof(TriangleIndices), sizeof(HexagonIndices), &HexagonIndices);

	FUSIONCORE::BindVAONull();
	FUSIONCORE::BindVBONull();
	FUSIONCORE::BindEBONull();
};

void FUSIONCORE::SHAPES::DrawRectangle(glm::vec4 Color,glm::vec2 Position, glm::vec2 Scale, float RotationInAngles, Camera2D& camera , FUSIONUTIL::DefaultShaders& shaders)
{
	auto &shader = *shaders.ShapeBasicShader;
	shader.use();
	glDisable(GL_DEPTH_TEST);
	ShapesVAO->Bind();

	camera.SetProjMatrixUniformLocation(shader.GetID(), "ProjMat");
	shader.setVec2("ShapePosition", Position);
	shader.setVec2("ShapeScale", Scale);
	shader.setVec4("ShapeColor", Color);

	glm::mat4 RotationMatrix(1.0f);
	if (RotationInAngles != 0.0f)
	{
		RotationMatrix = glm::rotate(RotationMatrix, glm::radians(RotationInAngles), { 0.0f,0.0,1.0f });
	}
	shader.setMat4("ModelMatrix", RotationMatrix);

	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(RectangleIndicesCount), GL_UNSIGNED_INT, 0);

	UseShaderProgram(0);
	FUSIONCORE::BindVAONull();
	glEnable(GL_DEPTH_TEST);
}

void FUSIONCORE::SHAPES::DrawRectangleTextured(Texture2D& Texture, glm::vec2 Position, glm::vec2 Scale, float RotationInAngles, Camera2D& camera, FUSIONUTIL::DefaultShaders& shaders)
{
	auto& shader = *shaders.ShapeTexturedShader;
	shader.use();
	glDisable(GL_DEPTH_TEST);
	ShapesVAO->Bind();

	camera.SetProjMatrixUniformLocation(shader.GetID(), "ProjMat");
	shader.setVec2("ShapePosition", Position);
	shader.setVec2("ShapeScale", Scale);
	Texture.Bind(0, shader.GetID(), "AlbedoTexture");

	glm::mat4 RotationMatrix(1.0f);
	if (RotationInAngles != 0.0f)
	{
		RotationMatrix = glm::rotate(RotationMatrix, glm::radians(RotationInAngles), { 0.0f,0.0,1.0f });
	}
	shader.setMat4("ModelMatrix", RotationMatrix);

	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(RectangleIndicesCount), GL_UNSIGNED_INT, 0);

	UseShaderProgram(0);
	FUSIONCORE::BindVAONull();
	glEnable(GL_DEPTH_TEST);
}

void FUSIONCORE::SHAPES::DrawTriangle(glm::vec4 Color, glm::vec2 Position, glm::vec2 Scale, float RotationInAngles, Camera2D& camera, FUSIONUTIL::DefaultShaders& shaders)
{
	auto& shader = *shaders.ShapeBasicShader;
	shader.use();
	glDisable(GL_DEPTH_TEST);
	ShapesVAO->Bind();

	camera.SetProjMatrixUniformLocation(shader.GetID(), "ProjMat");
	shader.setVec2("ShapePosition", Position);
	shader.setVec2("ShapeScale", Scale);
	shader.setVec4("ShapeColor", Color);

	glm::mat4 RotationMatrix(1.0f);
	if (RotationInAngles != 0.0f)
	{
		RotationMatrix = glm::rotate(RotationMatrix, glm::radians(RotationInAngles), { 0.0f,0.0,1.0f });
	}
	shader.setMat4("ModelMatrix", RotationMatrix);

	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(TriangleIndicesCount), GL_UNSIGNED_INT, (const void*)(sizeof(unsigned int) * RectangleIndicesCount));

	glEnable(GL_DEPTH_TEST);
	UseShaderProgram(0);
	FUSIONCORE::BindVAONull();
}

void FUSIONCORE::SHAPES::DrawTriangleTextured(Texture2D& Texture, glm::vec2 Position, glm::vec2 Scale, float RotationInAngles, Camera2D& camera, FUSIONUTIL::DefaultShaders& shaders)
{
	auto& shader = *shaders.ShapeTexturedShader;
	shader.use();
	glDisable(GL_DEPTH_TEST);
	ShapesVAO->Bind();

	camera.SetProjMatrixUniformLocation(shader.GetID(), "ProjMat");
	shader.setVec2("ShapePosition", Position);
	shader.setVec2("ShapeScale", Scale);
	Texture.Bind(0, shader.GetID(), "AlbedoTexture");

	glm::mat4 RotationMatrix(1.0f);
	if (RotationInAngles != 0.0f)
	{
		RotationMatrix = glm::rotate(RotationMatrix, glm::radians(RotationInAngles), { 0.0f,0.0,1.0f });
	}
	shader.setMat4("ModelMatrix", RotationMatrix);

	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(TriangleIndicesCount), GL_UNSIGNED_INT, (const void*)(sizeof(unsigned int) * RectangleIndicesCount));

	glEnable(GL_DEPTH_TEST);
	UseShaderProgram(0);
	FUSIONCORE::BindVAONull();
}

void FUSIONCORE::SHAPES::DrawHexagon(glm::vec4 Color, glm::vec2 Position, glm::vec2 Scale, float RotationInAngles, Camera2D& camera, FUSIONUTIL::DefaultShaders& shaders)
{
	auto& shader = *shaders.ShapeBasicShader;
	shader.use();
	glDisable(GL_DEPTH_TEST);
	ShapesVAO->Bind();

	camera.SetProjMatrixUniformLocation(shader.GetID(), "ProjMat");
	shader.setVec2("ShapePosition", Position);
	shader.setVec2("ShapeScale", Scale);
	shader.setVec4("ShapeColor", Color);

	glm::mat4 RotationMatrix(1.0f);
	if (RotationInAngles != 0.0f)
	{
		RotationMatrix = glm::rotate(RotationMatrix, glm::radians(RotationInAngles), { 0.0f,0.0,1.0f });
	}
	shader.setMat4("ModelMatrix", RotationMatrix);

	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(HexagonIndicesCount), GL_UNSIGNED_INT, (const void*)(sizeof(unsigned int) * (RectangleIndicesCount + TriangleIndicesCount)));

	glEnable(GL_DEPTH_TEST);
	UseShaderProgram(0);
	FUSIONCORE::BindVAONull();
}

void FUSIONCORE::SHAPES::DrawHexagonTextured(Texture2D& Texture, glm::vec2 Position, glm::vec2 Scale, float RotationInAngles, Camera2D& camera, FUSIONUTIL::DefaultShaders& shaders)
{
	auto& shader = *shaders.ShapeTexturedShader;
	shader.use();
	glDisable(GL_DEPTH_TEST);
	ShapesVAO->Bind();

	camera.SetProjMatrixUniformLocation(shader.GetID(), "ProjMat");
	shader.setVec2("ShapePosition", Position);
	shader.setVec2("ShapeScale", Scale);
	Texture.Bind(0, shader.GetID(), "AlbedoTexture");

	glm::mat4 RotationMatrix(1.0f);
	if (RotationInAngles != 0.0f)
	{
		RotationMatrix = glm::rotate(RotationMatrix, glm::radians(RotationInAngles), { 0.0f,0.0,1.0f });
	}
	shader.setMat4("ModelMatrix", RotationMatrix);

	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(HexagonIndicesCount), GL_UNSIGNED_INT, (const void*)(sizeof(unsigned int) * (RectangleIndicesCount + TriangleIndicesCount)));

	glEnable(GL_DEPTH_TEST);
	UseShaderProgram(0);
	FUSIONCORE::BindVAONull();
}
