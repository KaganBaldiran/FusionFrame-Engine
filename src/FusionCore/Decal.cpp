#include <glew.h>
#include "Decal.hpp"
#include "Buffer.h"

std::unique_ptr<FUSIONCORE::Buffer3D> UnitBoxBuffer;

void FUSIONCORE::InitializeDecalUnitBox()
{
	UnitBoxBuffer = std::make_unique<FUSIONCORE::Buffer3D>();
	UnitBoxBuffer->Bind();

    GLfloat vertices[] = {
        // Positions          // Colors           // Texture Coords
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,  // Bottom-left
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,  // Bottom-right
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,  // Top-right
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,  // Top-left

        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f, 0.0f,  // Bottom-left
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,  1.0f, 0.0f,  // Bottom-right
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  // Top-right
        -0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f,  0.0f, 1.0f   // Top-left
    };


    GLuint indices[] = {
        // Back face
        0, 1, 1, 2, 2, 3, 3, 0,
        // Front face
        4, 5, 5, 6, 6, 7, 7, 4,
        // Left face
        0, 4, 3, 7,
        // Right face
        1, 5, 2, 6,
        // Bottom face
        0, 1, 4, 5,
        // Top face
        3, 2, 7, 6
    };


	UnitBoxBuffer->BufferDataFill(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);
    UnitBoxBuffer->AttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (GLvoid*)0);
    UnitBoxBuffer->AttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (GLvoid*)(3 * sizeof(float)));
    UnitBoxBuffer->AttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (GLvoid*)(6 * sizeof(float)));
    
    UnitBoxBuffer->BindEBO();
    UnitBoxBuffer->BufferDataFill(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    UnitBoxBuffer->Unbind();
}

FUSIONCORE::Buffer3D* FUSIONCORE::GetUnitBoxBuffer()
{
    return UnitBoxBuffer.get();
}

void FUSIONCORE::DecalDeferred::Draw(GeometryBuffer& GeometryBuffer, Material Material, Camera3D& camera,glm::ivec2 WindowSize, FUSIONUTIL::DefaultShaders& shaders)
{
    auto& shader = shaders.DecalShader;
    shader->use();
    UnitBoxBuffer->BindVAO();
    glDisable(GL_DEPTH_TEST);

    shader->setVec2("NormalizedWindowSize", glm::vec2(WindowSize) / glm::vec2(GeometryBuffer.GetFBOSize()));
    auto ModelMatrix = this->transformation.GetModelMat4();
    shader->setMat4("ModelMatrix", ModelMatrix);
    shader->setMat4("inverseModelMatrix", glm::inverse(ModelMatrix));
    shader->setMat4("ProjViewMatrix", camera.ProjectionViewMat);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, GeometryBuffer.GetPositionDepthPass());
    shader->setInt("WorldPositionBuffer", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, GeometryBuffer.GetNormalMetalicPass());
    shader->setInt("WorldSpaceNormalBuffer", 1);

    Material.SetMaterialShader(*shader);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    glEnable(GL_DEPTH_TEST);
    BindVAONull();
    UseShaderProgram(0);
}

void FUSIONCORE::DecalDeferred::VisualiseDecalCage(Camera3D& camera, Shader& shader,glm::vec3 CageColor)
{
    shader.use();
    UnitBoxBuffer->BindVAO();
    
    auto ModelMatrix = this->transformation.GetModelMat4();
    shader.setMat4("model", ModelMatrix);
    shader.setMat4("ProjView", camera.ProjectionViewMat);
    shader.setVec3("LightColor", CageColor);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    BindVAONull();
    UseShaderProgram(0);
}
