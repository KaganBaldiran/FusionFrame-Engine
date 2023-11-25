#include "Physics.hpp"
#include <time.h>
#include <random>
#include <chrono>

FUSIONPHYSICS::CollisionBox3DAABB::CollisionBox3DAABB(FUSIONOPENGL::WorldTransform& transformation , glm::vec3 BoxSizeCoeff)
{
	FUSIONOPENGL::Vertex vertex;

	auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::uniform_real_distribution<float> RandomFloats(0.0f, 1.0f);
	std::default_random_engine engine(seed);

	MeshColor.x = RandomFloats(engine);
	MeshColor.y = RandomFloats(engine);
	MeshColor.z = RandomFloats(engine);

	float Xsize = (transformation.ObjectScales.x / 2.0f) * BoxSizeCoeff.x;
	float Ysize = (transformation.ObjectScales.y / 2.0f) * BoxSizeCoeff.y;
	float Zsize = (transformation.ObjectScales.z / 2.0f) * BoxSizeCoeff.z;

    //Upper Part
	vertex.Position.x = transformation.Position.x + Xsize;
	vertex.Position.y = transformation.Position.y + Ysize;
	vertex.Position.z = transformation.Position.z + Zsize;

	BoxVertices.push_back(vertex);

	vertex.Position.x = transformation.Position.x + Xsize;
	vertex.Position.y = transformation.Position.y + Ysize;
	vertex.Position.z = transformation.Position.z - Zsize;

	BoxVertices.push_back(vertex);

	vertex.Position.x = transformation.Position.x - Xsize;
	vertex.Position.y = transformation.Position.y + Ysize;
	vertex.Position.z = transformation.Position.z + Zsize;

	BoxVertices.push_back(vertex);

	vertex.Position.x = transformation.Position.x - Xsize;
	vertex.Position.y = transformation.Position.y + Ysize;
	vertex.Position.z = transformation.Position.z - Zsize;

	BoxVertices.push_back(vertex);



	//Bottom Part
	vertex.Position.x = transformation.Position.x + Xsize;
	vertex.Position.y = transformation.Position.y - Ysize;
	vertex.Position.z = transformation.Position.z + Zsize;

	BoxVertices.push_back(vertex);

	vertex.Position.x = transformation.Position.x + Xsize;
	vertex.Position.y = transformation.Position.y - Ysize;
	vertex.Position.z = transformation.Position.z - Zsize;

	BoxVertices.push_back(vertex);

	vertex.Position.x = transformation.Position.x - Xsize;
	vertex.Position.y = transformation.Position.y - Ysize;
	vertex.Position.z = transformation.Position.z + Zsize;

	BoxVertices.push_back(vertex);

	vertex.Position.x = transformation.Position.x - Xsize;
	vertex.Position.y = transformation.Position.y - Ysize;
	vertex.Position.z = transformation.Position.z - Zsize;

	BoxVertices.push_back(vertex);

	const GLuint indices[] = {
		// Upper part
		0, 1, 2,
		2, 1, 3,

		// Bottom part
		4, 5, 6,
		6, 5, 7,

		// Side faces
		0, 2, 4,
		2, 4, 6,
		1, 3, 5,
		3, 5, 7,

		// Caps
		0, 1, 4, 
		1, 4, 5,
		2, 3, 6, 
		3, 6, 7
	};

	BoxIndices.assign(indices, indices + sizeof(indices) / sizeof(indices[0]));

	Min.x = transformation.Position.x - Xsize;
	Min.y = transformation.Position.y - Ysize;
	Min.z = transformation.Position.z - Zsize;

	Max.x = transformation.Position.x + Xsize;
	Max.y = transformation.Position.y + Ysize;
	Max.z = transformation.Position.z + Zsize;

	std::vector<FUSIONOPENGL::Texture2D> textures;
	BoxMesh = std::make_unique<FUSIONOPENGL::Mesh3D>(BoxVertices, BoxIndices, textures);
}

void FUSIONPHYSICS::CollisionBox3DAABB::DrawBoxMesh(FUSIONOPENGL::Camera3D& camera, Shader& shader)
{
	std::function<void()> boxprep = [&]() 
	{
		shader.setVec3("LightColor", MeshColor);
		shader.setMat4("model", glm::mat4(1.0f));
	};
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	BoxMesh->Draw(camera, shader, boxprep);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

bool FUSIONPHYSICS::BoxBoxIntersect(CollisionBox3DAABB& Box1, CollisionBox3DAABB& Box2)
{
	return Box1.Min.x <= Box2.Max.x &&
     	   Box1.Max.x >= Box2.Min.x &&
		   Box1.Min.y <= Box2.Max.y &&
		   Box1.Max.y >= Box2.Min.y &&
		   Box1.Min.z <= Box2.Max.z &&
		   Box1.Max.z >= Box2.Min.z;
}
