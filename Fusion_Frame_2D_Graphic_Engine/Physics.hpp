#pragma once
#include <glew.h>
#include <glfw3.h>
#include "Log.h"
#include "VectorMath.h"
#include "Object.hpp"
#include "Buffer.h"
#include <memory>

namespace FUSIONPHYSICS
{
	class CollisionBox3DAABB : public FUSIONOPENGL::Object
	{
	public:
		CollisionBox3DAABB(FUSIONOPENGL::WorldTransform& transformation, glm::vec3 BoxSizeCoeff);
		void DrawBoxMesh(FUSIONOPENGL::Camera3D& camera, Shader& shader);
		FUSIONOPENGL::Mesh3D* GetBoxMesh() { return BoxMesh.get(); };

		glm::vec3 Min;
		glm::vec3 Max;
	private:

		std::vector<FUSIONOPENGL::Vertex> BoxVertices;
		std::vector<unsigned int> BoxIndices;
		std::unique_ptr<FUSIONOPENGL::Mesh3D> BoxMesh;
		glm::vec3 MeshColor;
	};

	bool BoxBoxIntersect(CollisionBox3DAABB& Box1, CollisionBox3DAABB& Box2);
}
