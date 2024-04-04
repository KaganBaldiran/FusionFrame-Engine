#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "../FusionCore/Object.hpp"
#include "../FusionCore/Mesh.h"
#include "../FusionCore/Buffer.h"
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

	class CollisionBox : public FUSIONCORE::Object
	{
	public:
		CollisionBox() = default;
		CollisionBox(FUSIONCORE::Mesh& InputMesh, FUSIONCORE::WorldTransform transformation = FUSIONCORE::WorldTransform());
		FUSIONCORE::Mesh* GetBoxMesh() { return BoxMesh.get(); };
		const std::vector<glm::vec3>& GetLocalNormals() { return this->LocalBoxNormals; };
		const std::vector<glm::vec3>& GetLocalEdgeNormals() { return this->LocalEdgeNormals; };
		const std::vector<glm::vec3>& GetNormals() { return this->BoxNormals; };
		const std::vector<FUSIONCORE::Vertex>& GetVertices() { return this->BoxVertices; };
		void DrawBoxMesh(FUSIONCORE::Camera3D& camera, FUSIONCORE::Shader& shader);

		~CollisionBox();
		void Clean();
		void Update();
		//Update without a parent
		virtual void UpdateAttributes();
		inline void SetMeshColor(glm::vec3 color) { this->MeshColor = color; };

		glm::vec3 Min;
		glm::vec3 Max;
	protected:
		std::vector<FUSIONCORE::Vertex> BoxVertices;
		std::vector<unsigned int> BoxIndices;
		std::vector<glm::vec3> BoxNormals;
		std::vector<glm::vec3> LocalBoxNormals;
		std::vector<glm::vec3> LocalEdgeNormals;
		std::shared_ptr<FUSIONCORE::Mesh> BoxMesh;
		glm::vec3 ModelOriginPoint;
		glm::vec3 MeshColor;
	};

	class CollisionBoxPlane : public CollisionBox
	{
	public:
		CollisionBoxPlane(glm::vec3 Size, glm::vec3 BoxSizeCoeff);
		
		~CollisionBoxPlane();
		void Clear();		
		void Update() override;
		//Update without a parent
		void UpdateAttributes() override;

	private:
		std::vector<FUSIONCORE::Face> Faces;
	};

	class CollisionBox3DAABB : public CollisionBox
	{
	public:
		CollisionBox3DAABB(FUSIONCORE::WorldTransform& transformation, glm::vec3 BoxSizeCoeff);
		CollisionBox3DAABB(glm::vec3 Size, glm::vec3 BoxSizeCoeff = glm::vec3(1.0f));
		std::vector<FUSIONCORE::Face>& GetFaces() { return this->Faces; };

		~CollisionBox3DAABB();
		void Update() override;
		//Update without a parent
		void UpdateAttributes() override;
		float ProjectOntoAxis(const glm::vec3& axis);
	
		void Clear();
	private:
		std::vector<FUSIONCORE::Face> Faces;
	};

	std::pair<int, glm::vec3> CheckCollisionDirection(glm::vec3 targetVector, FUSIONCORE::WorldTransform& Entity1Transformation);
	std::pair<int, glm::vec3> CheckCollisionDirection(glm::vec3 targetVector, std::vector<glm::vec3> Normals);
	std::pair<bool, int> BoxBoxIntersect(CollisionBox3DAABB& Box1, CollisionBox3DAABB& Box2);
	std::pair<bool, float> FindMinSeparation(CollisionBox& Box1, CollisionBox& Box2, glm::vec3 Axis);
	bool IsCollidingSAT(CollisionBox3DAABB& Box1, CollisionBox3DAABB& Box2);
	bool IsCollidingSAT(CollisionBox& Plane, CollisionBox3DAABB& Box);
	std::pair<bool, glm::vec3> IsCollidingSAT(CollisionBox& Box1, CollisionBox& Box2);

	//Sphere-Sphere collision
	bool IsCollidingSphereCollision(glm::vec3 center1, glm::vec3 radius1, glm::vec3 center2, glm::vec3 radius2);
}
