#include "HalfEdge.hpp"

glm::vec3 FUSIONOPENGL::HalfEdge::GetMiddlePoint()
{
    return (this->StartingVertex->Position + this->EndingVertex->Position) / 2.0f;
}

glm::vec3 FUSIONOPENGL::HalfEdge::GetEdgeVector()
{
    return this->EndingVertex->Position - this->StartingVertex->Position;
}

float FUSIONOPENGL::HalfEdge::GetEdgeLength()
{
    return glm::length(GetEdgeVector());
}
