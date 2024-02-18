#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "../FusionOpengl/Object.hpp"
#include "../FusionOpengl/Mesh.h"
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

	class CollisionBox : public FUSIONOPENGL::Object
	{
	public:
		FUSIONOPENGL::Mesh3D* GetBoxMesh() { return BoxMesh.get(); };
		const std::vector<glm::vec3>& GetLocalNormals() { return this->LocalBoxNormals; };
		const std::vector<glm::vec3>& GetNormals() { return this->BoxNormals; };
		const std::vector<FUSIONOPENGL::Vertex>& GetVertices() { return this->BoxVertices; };
		void DrawBoxMesh(FUSIONOPENGL::Camera3D& camera, FUSIONOPENGL::Shader& shader);

		glm::vec3 Min;
		glm::vec3 Max;
	protected:
		std::vector<FUSIONOPENGL::Vertex> BoxVertices;
		std::vector<unsigned int> BoxIndices;
		std::vector<glm::vec3> BoxNormals;
		std::vector<glm::vec3> LocalBoxNormals;
		std::unique_ptr<FUSIONOPENGL::Mesh3D> BoxMesh;
		glm::vec3 ModelOriginPoint;
		glm::vec3 MeshColor;
	};

	class CollisionBoxPlane : public CollisionBox
	{
	public:
		CollisionBoxPlane(glm::vec3 Size, glm::vec3 BoxSizeCoeff);
		
		void Clear();		
		void Update();
		void UpdateAttributes();

	private:
		std::vector<FUSIONOPENGL::Face<4>> Faces;
	};

	class CollisionBox3DAABB : public CollisionBox
	{
	public:
		CollisionBox3DAABB(FUSIONOPENGL::WorldTransform& transformation, glm::vec3 BoxSizeCoeff);
		CollisionBox3DAABB(glm::vec3 Size, glm::vec3 BoxSizeCoeff = glm::vec3(1.0f));
		std::vector<FUSIONOPENGL::Face<4>>& GetFaces() { return this->Faces; };

		void Update();
		//Update without a parent
		void UpdateAttributes();
		float ProjectOntoAxis(const glm::vec3& axis);

		void Clear();
	private:
		std::vector<FUSIONOPENGL::Face<4>> Faces;
	};

	std::pair<int, glm::vec3> CheckCollisionDirection(glm::vec3 targetVector, glm::mat4 Entity1ModelMatrix);
	std::pair<int, glm::vec3> CheckCollisionDirection(glm::vec3 targetVector, std::vector<glm::vec3> Normals);
	std::pair<bool, int> BoxBoxIntersect(CollisionBox3DAABB& Box1, CollisionBox3DAABB& Box2);
	float FindMinSeparation(CollisionBox& Box1, CollisionBox& Box2, glm::vec3 Axis);
	bool IsCollidingSAT(CollisionBox3DAABB& Box1, CollisionBox3DAABB& Box2);
	bool IsCollidingSAT(CollisionBox& Plane, CollisionBox3DAABB& Box);
	bool IsCollidingSAT(CollisionBox& Box1, CollisionBox& Box2);
}
