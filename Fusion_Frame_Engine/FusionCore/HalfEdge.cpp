#include "HalfEdge.hpp"
#include <glew.h>
#include <glfw3.h>

glm::vec3 FUSIONCORE::HalfEdge::GetMiddlePoint()
{
    return (this->StartingVertex->Position + this->EndingVertex->Position) / 2.0f;
}

glm::vec3 FUSIONCORE::HalfEdge::GetEdgeVector()
{
    return this->EndingVertex->Position - this->StartingVertex->Position;
}

float FUSIONCORE::HalfEdge::GetEdgeLength()
{
    return glm::length(GetEdgeVector());
}

glm::vec3 FUSIONCORE::TranslateVertex(glm::mat4 Matrix, glm::vec3 VertexPos)
{
    glm::vec4 transformed = Matrix * glm::vec4(VertexPos, 1.0f);
    return glm::vec3(transformed);
}

glm::vec3 FUSIONCORE::TranslateNormal(glm::mat4 ModelMatrix, glm::vec3 Normal)
{
    return glm::normalize(glm::transpose(glm::inverse(glm::mat3(ModelMatrix))) * Normal);
}

glm::vec3 FUSIONCORE::FindNormal(std::vector<Vertex> Vertices)
{
    glm::vec3 Normal = glm::cross((Vertices[1].Position - Vertices[0].Position), (Vertices[2].Position - Vertices[0].Position));
    if (glm::length(Normal) < glm::epsilon<float>())
    {
        LOG_ERR("Error: Attempting to normalize a zero-length vector.");
    }
    else {
        Normal = glm::normalize(glm::abs(Normal));
    }
    return Normal;
}

glm::vec3 FUSIONCORE::FindNormal(glm::mat4 ModelMatrix, std::vector<Vertex> Vertices)
{
    glm::vec3 Normal = glm::cross((TranslateVertex(ModelMatrix, Vertices[1].Position) - TranslateVertex(ModelMatrix, Vertices[0].Position)), (TranslateVertex(ModelMatrix, Vertices[2].Position) - TranslateVertex(ModelMatrix, Vertices[0].Position)));
    if (glm::length(Normal) < glm::epsilon<float>())
    {
        LOG_ERR("Error: Attempting to normalize a zero-length vector.");
    }
    else {
        Normal = glm::normalize(glm::abs(Normal));
    }
    return Normal;
}

glm::vec3 FUSIONCORE::CalculateAverage(const glm::vec3& v1, const glm::vec3& v2)
{
    return (v1 + v2) * 0.5f;
}

glm::vec3 FUSIONCORE::CalculateAverageNormalized(const glm::vec3& v1, const glm::vec3& v2)
{
    return glm::normalize((v1 + v2) * 0.5f);
}

glm::vec2 FUSIONCORE::CalculateAverage(const glm::vec2& uv1, const glm::vec2& uv2) {
    return (uv1 + uv2) * 0.5f;
}

FUSIONCORE::Vertex FUSIONCORE::GetAveragedVertex(FUSIONCORE::Vertex vertex1, FUSIONCORE::Vertex vertex2)
{
    Vertex newVertex;
    newVertex.Normal = CalculateAverage(vertex1.Normal, vertex2.Normal);
    newVertex.Bitangent = CalculateAverage(vertex1.Bitangent, vertex2.Bitangent);
    newVertex.Tangent = CalculateAverage(vertex1.Tangent, vertex2.Tangent);
    newVertex.Position = CalculateAverage(vertex1.Position, vertex2.Position);
    newVertex.TexCoords = CalculateAverage(vertex1.TexCoords, vertex2.TexCoords);
    return newVertex;
}


