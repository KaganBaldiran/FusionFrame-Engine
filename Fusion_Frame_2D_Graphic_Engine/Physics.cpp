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
		shader.setMat4("model", this->GetTransformation().ModelMatrix);
	};
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	BoxMesh->Draw(camera, shader, boxprep);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void FUSIONPHYSICS::CollisionBox3DAABB::Update()
{

	auto& lastScales = this->Parent->GetTransformation().LastScales;
	auto& lastRotations = this->Parent->GetTransformation().LastRotations;
	auto& lastTransforms = this->Parent->GetTransformation().LastTransforms;

	
	/*glm::vec4 tempMin(Min.x,
		              Min.y,
		              Min.z,
		              1.0f);

	glm::vec4 tempMax(Max.x,
		              Max.y,
		              Max.z,
		              1.0f);

	for (size_t i = 0; i < lastScales.size(); i++)
	{
		tempMin = glm::scale(lastScales[i].Scale) * tempMin;
	}
	for (size_t i = 0; i < lastRotations.size(); i++)
	{
		tempMin = glm::translate(this->Parent->GetTransformation().Position) * tempMin;
		tempMin = glm::rotate(lastRotations[i].Degree, lastRotations[i].Vector) * tempMin;
		tempMin = glm::translate(-this->Parent->GetTransformation().Position) * tempMin;

		tempMax = glm::translate(this->Parent->GetTransformation().Position) * tempMax;
		tempMax = glm::rotate(lastRotations[i].Degree, lastRotations[i].Vector) * tempMax;
		tempMax = glm::translate(-this->Parent->GetTransformation().Position) * tempMax;
	}
	for (size_t i = 0; i < lastTransforms.size(); i++)
	{
		tempMin = glm::translate(lastTransforms[i].Transformation) * tempMin;
		tempMax = glm::translate(lastTransforms[i].Transformation) * tempMax;

	}

	Min = glm::vec3(tempMin.x, tempMin.y, tempMin.z);
	Max = glm::vec3(tempMax.x, tempMax.y, tempMax.z);*/
	Min = Max = BoxVertices[0].Position;
	for (auto& vertex : BoxVertices)
	{
		glm::vec4 tempVertex = glm::vec4(vertex.Position, 1.0f);

		for (size_t i = 0; i < lastScales.size(); i++)
		{
			tempVertex = glm::scale(lastScales[i].Scale) * tempVertex;
		}

		for (size_t i = 0; i < lastRotations.size(); i++)
		{
			// Apply rotation around the object's local origin
			tempVertex = glm::rotate(lastRotations[i].Degree, lastRotations[i].Vector) * tempVertex;
		}

		for (size_t i = 0; i < lastTransforms.size(); i++)
		{
			tempVertex = glm::translate(lastTransforms[i].Transformation) * tempVertex;
		}

		vertex.Position = glm::vec3(tempVertex.x, tempVertex.y, tempVertex.z);

		// Update Min and Max based on the updated vertex
		Min = glm::min(Min, vertex.Position);
		Max = glm::max(Max, vertex.Position);
	}

	for (size_t i = 0; i < lastScales.size(); i++)
	{
		this->transformation.Scale(lastScales[i].Scale);
	}
	for (size_t i = 0; i < lastRotations.size(); i++)
	{
		this->transformation.Translate(this->Parent->GetTransformation().Position);
		this->transformation.Rotate(lastRotations[i].Vector, lastRotations[i].Degree);
		this->transformation.Translate(-this->Parent->GetTransformation().Position);
	}
	for (size_t i = 0; i < lastTransforms.size(); i++)
	{
		this->transformation.Translate(lastTransforms[i].Transformation);
	}


	UpdateChildren();
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
