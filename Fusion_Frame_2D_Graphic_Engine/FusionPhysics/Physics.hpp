#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "../FusionOpengl/Object.hpp"
#include "../FusionOpengl/Buffer.h"
#include <memory>
#define FF_TOP_SIDE 0
#define FF_BOTTOM_SIDE 1
#define FF_RIGHT_SIDE 2
#define FF_LEFT_SIDE 3
#define FF_FRONT_SIDE 4
#define FF_BEHIND_SIDE 5

namespace FUSIONPHYSICS
{
	class CollisionState
	{
	public:
		bool state;
		glm::vec3 directionVector;
		int direction;
		CollisionState(bool state, glm::vec3 directionVector, int direction)
		{
			this->state = state;
			this->directionVector = directionVector;
			this->direction = direction;
		}
	};

	class CollisionBox3DAABB : public FUSIONOPENGL::Object
	{
	public:
		CollisionBox3DAABB(FUSIONOPENGL::WorldTransform& transformation, glm::vec3 BoxSizeCoeff);
		void DrawBoxMesh(FUSIONOPENGL::Camera3D& camera, FUSIONOPENGL::Shader& shader);
		FUSIONOPENGL::Mesh3D* GetBoxMesh() { return BoxMesh.get(); };
		const std::vector<glm::vec3> &GetLocalNormals() { return this->LocalBoxNormals; };
		const std::vector<glm::vec3> &GetNormals() { return this->BoxNormals; };
		const std::vector<FUSIONOPENGL::Vertex> &GetVertices() { return this->BoxVertices; };
		std::vector<FUSIONOPENGL::Face<4>>& GetFaces() { return this->Faces; };
		void Update();
		float ProjectOntoAxis(const glm::vec3& axis);

		glm::vec3 Min;
		glm::vec3 Max;
	private:

		std::vector<FUSIONOPENGL::Vertex> BoxVertices;
		std::vector<unsigned int> BoxIndices;
		std::vector<glm::vec3> BoxNormals;
		std::vector<glm::vec3> LocalBoxNormals;
		std::vector<FUSIONOPENGL::Face<4>> Faces;
		std::unique_ptr<FUSIONOPENGL::Mesh3D> BoxMesh;
		glm::vec3 MeshColor;
		glm::vec3 ModelOriginPoint;
	};

	std::pair<int, glm::vec3> CheckCollisionDirection(glm::vec3 targetVector, glm::mat4 Entity1ModelMatrix);
	std::pair<bool, int> BoxBoxIntersect(CollisionBox3DAABB& Box1, CollisionBox3DAABB& Box2);
	float FindMinSeparation(CollisionBox3DAABB& Box1, CollisionBox3DAABB& Box2, glm::vec3 Axis);
	bool IsCollidingSAT(CollisionBox3DAABB& Box1, CollisionBox3DAABB& Box2);
}
