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

	float Xsize = (transformation.InitialObjectScales.x / 2.0f) * BoxSizeCoeff.x;
	float Ysize = (transformation.InitialObjectScales.y / 2.0f) * BoxSizeCoeff.y;
	float Zsize = (transformation.InitialObjectScales.z / 2.0f) * BoxSizeCoeff.z;

	auto OriginPosition = FF_ORIGIN;

    //Upper Part
	vertex.Position.x = OriginPosition.x + Xsize;
	vertex.Position.y = OriginPosition.y + Ysize;
	vertex.Position.z = OriginPosition.z + Zsize;

	BoxVertices.push_back(vertex);

	vertex.Position.x = OriginPosition.x + Xsize;
	vertex.Position.y = OriginPosition.y + Ysize;
	vertex.Position.z = OriginPosition.z - Zsize;

	BoxVertices.push_back(vertex);

	vertex.Position.x = OriginPosition.x - Xsize;
	vertex.Position.y = OriginPosition.y + Ysize;
	vertex.Position.z = OriginPosition.z + Zsize;

	BoxVertices.push_back(vertex);

	vertex.Position.x = OriginPosition.x - Xsize;
	vertex.Position.y = OriginPosition.y + Ysize;
	vertex.Position.z = OriginPosition.z - Zsize;

	BoxVertices.push_back(vertex);



	//Bottom Part
	vertex.Position.x = OriginPosition.x + Xsize;
	vertex.Position.y = OriginPosition.y - Ysize;
	vertex.Position.z = OriginPosition.z + Zsize;

	BoxVertices.push_back(vertex);

	vertex.Position.x = OriginPosition.x + Xsize;
	vertex.Position.y = OriginPosition.y - Ysize;
	vertex.Position.z = OriginPosition.z - Zsize;

	BoxVertices.push_back(vertex);

	vertex.Position.x = OriginPosition.x - Xsize;
	vertex.Position.y = OriginPosition.y - Ysize;
	vertex.Position.z = OriginPosition.z + Zsize;

	BoxVertices.push_back(vertex);

	vertex.Position.x = OriginPosition.x - Xsize;
	vertex.Position.y = OriginPosition.y - Ysize;
	vertex.Position.z = OriginPosition.z - Zsize;

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

	this->GetTransformation().TranslationMatrix = transformation.TranslationMatrix;
	this->GetTransformation().ScalingMatrix = transformation.ScalingMatrix;
	this->GetTransformation().RotationMatrix = transformation.RotationMatrix;

	float maxX = -std::numeric_limits<float>::infinity();
	float maxY = -std::numeric_limits<float>::infinity();
	float maxZ = -std::numeric_limits<float>::infinity();
	float minX = std::numeric_limits<float>::infinity();
	float minY = std::numeric_limits<float>::infinity();
	float minZ = std::numeric_limits<float>::infinity();

	auto& VertexArray = this->BoxVertices;

	for (unsigned int k = 0; k < VertexArray.size(); k++) {

		glm::vec4 transformed = this->GetTransformation().GetModelMat4() * glm::vec4(VertexArray[k].Position, 1.0f);

		maxX = std::max(maxX, transformed.x);
		maxY = std::max(maxY, transformed.y);
		maxZ = std::max(maxZ, transformed.z);
		minX = std::min(minX, transformed.x);
		minY = std::min(minY, transformed.y);
		minZ = std::min(minZ, transformed.z);
	}

	Min.x = minX;
	Min.y = minY;
	Min.z = minZ;

	Max.x = maxX;
	Max.y = maxY;
	Max.z = maxZ;

	std::vector<FUSIONOPENGL::Texture2D> textures;
	BoxMesh = std::make_unique<FUSIONOPENGL::Mesh3D>(BoxVertices, BoxIndices, textures);
}

void FUSIONPHYSICS::CollisionBox3DAABB::DrawBoxMesh(FUSIONOPENGL::Camera3D& camera, Shader& shader)
{
	std::function<void()> boxprep = [&]() 
	{
		shader.setVec3("LightColor", MeshColor);
		shader.setMat4("model", this->GetTransformation().GetModelMat4());
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

	for (size_t i = 0; i < lastScales.size(); i++)
	{
		this->transformation.Scale(lastScales[i].Scale);
	}
	for (size_t i = 0; i < lastRotations.size(); i++)
	{
		this->transformation.Rotate(lastRotations[i].Vector, lastRotations[i].Degree);
	}
	for (size_t i = 0; i < lastTransforms.size(); i++)
	{
		this->transformation.Translate(lastTransforms[i].Transformation);
	}

	float maxX = -std::numeric_limits<float>::infinity();
	float maxY = -std::numeric_limits<float>::infinity();
	float maxZ = -std::numeric_limits<float>::infinity();
	float minX = std::numeric_limits<float>::infinity();
	float minY = std::numeric_limits<float>::infinity();
	float minZ = std::numeric_limits<float>::infinity();

	auto& VertexArray = this->BoxVertices;

	for (unsigned int k = 0; k < VertexArray.size(); k++) {

		glm::vec4 transformed = this->GetTransformation().GetModelMat4() * glm::vec4(VertexArray[k].Position, 1.0f);

		maxX = std::max(maxX, transformed.x);
		maxY = std::max(maxY, transformed.y);
		maxZ = std::max(maxZ, transformed.z);
		minX = std::min(minX, transformed.x);
		minY = std::min(minY, transformed.y);
		minZ = std::min(minZ, transformed.z);
	}

	Min.x = minX;
	Min.y = minY;
	Min.z = minZ;

	Max.x = maxX;
	Max.y = maxY;
	Max.z = maxZ;

	UpdateChildren();
}


std::pair<int,glm::vec3> FUSIONPHYSICS::CheckCollisionDirection(glm::vec3 targetVector , glm::mat4 Entity1ModelMatrix)
{
	glm::vec3 Directions[] = {
		glm::vec3(0.0f, 1.0f,0.0f),	    // Up
		glm::vec3(0.0f, -1.0f,0.0f),	// Down
		glm::vec3(1.0f, 0.0f , 0.0f),	// Right
		glm::vec3(-1.0f, 0.0f , 0.0f),	// Left
		glm::vec3(0.0f, 0.0 , 1.0f),	// Forward
		glm::vec3(0.0f, 0.0 , -1.0f)	// Backward
	};

	float max = 0.0f;
	int Chosen = -1;
	glm::vec3 ChosenAxis = glm::vec3(0.0f);

	glm::vec4 transformed;
	for (size_t i = 0; i < 6; i++)
	{
		transformed = Entity1ModelMatrix * glm::vec4(Directions[i], 1.0f);
		float dotProduct = glm::dot(glm::vec3(transformed.x, transformed.y, transformed.z),targetVector);

		if (dotProduct > max)
		{
			max = dotProduct;
			Chosen = i;
			ChosenAxis = Directions[i];
		}
	}

	return { Chosen,ChosenAxis };
}

std::pair<bool,int> FUSIONPHYSICS::BoxBoxIntersect(CollisionBox3DAABB& Box1, CollisionBox3DAABB& Box2)
{
	auto direction = CheckCollisionDirection(Box1.GetTransformation().Position - Box2.GetTransformation().Position,
		                                     Box1.GetTransformation().GetModelMat4());

	LOG("BOX 1 min: " << Vec3<float>(Box1.Min) << "  max : " << Vec3<float>(Box1.Max));
	LOG("BOX 2 min: " << Vec3<float>(Box2.Min) << "  max : " << Vec3<float>(Box2.Max));

	return{ Box1.Min.x <= Box2.Max.x &&
		   Box1.Max.x >= Box2.Min.x &&
		   Box1.Min.y <= Box2.Max.y &&
		   Box1.Max.y >= Box2.Min.y &&
		   Box1.Min.z <= Box2.Max.z &&
		   Box1.Max.z >= Box2.Min.z , direction.first };
}
