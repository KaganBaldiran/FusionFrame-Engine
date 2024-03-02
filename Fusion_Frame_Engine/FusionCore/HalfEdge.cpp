#include "HalfEdge.hpp"

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
    newVertex.Normal = CalculateAverageNormalized(vertex1.Normal, vertex2.Normal);
    newVertex.Bitangent = CalculateAverageNormalized(vertex1.Bitangent, vertex2.Bitangent);
    newVertex.Tangent = CalculateAverageNormalized(vertex1.Tangent, vertex2.Tangent);
    newVertex.Position = CalculateAverage(vertex1.Position, vertex2.Position);
    newVertex.TexCoords = CalculateAverage(vertex1.TexCoords, vertex2.TexCoords);
    return newVertex;
}


