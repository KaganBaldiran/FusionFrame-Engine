#include "Physics.hpp"
#include <time.h>
#include <random>
#include <chrono>
#include <unordered_set>

std::pair<glm::vec3, glm::vec3> FindMinMax(std::vector<FUSIONCORE::Vertex> BoxVertices, glm::mat4 ModelMat4)
{
	float maxX = -std::numeric_limits<float>::infinity();
	float maxY = -std::numeric_limits<float>::infinity();
	float maxZ = -std::numeric_limits<float>::infinity();
	float minX = std::numeric_limits<float>::infinity();
	float minY = std::numeric_limits<float>::infinity();
	float minZ = std::numeric_limits<float>::infinity();

	auto& VertexArray = BoxVertices;

	for (unsigned int k = 0; k < VertexArray.size(); k++) {

		glm::vec4 transformed = ModelMat4 * glm::vec4(VertexArray[k].Position, 1.0f);

		maxX = std::max(maxX, transformed.x);
		maxY = std::max(maxY, transformed.y);
		maxZ = std::max(maxZ, transformed.z);
		minX = std::min(minX, transformed.x);
		minY = std::min(minY, transformed.y);
		minZ = std::min(minZ, transformed.z);
	}

	return { {minX,minY,minZ} ,{maxX,maxY,maxZ} };
}

bool SameDirection(const glm::vec3 direction1, const glm::vec3 direction2)
{
	return glm::dot(direction1, direction2) > 0;
}

std::vector<FUSIONCORE::Vertex> FindPointsOnDirection(const glm::vec3& direction, const std::vector<FUSIONCORE::Vertex>& Vertices)
{
	std::vector<FUSIONCORE::Vertex> PointsOnDirection;

	for (const FUSIONCORE::Vertex& vertex : Vertices) {
		if (SameDirection(vertex.Position, direction)) {
			PointsOnDirection.push_back(vertex);
		}
	}

	return PointsOnDirection;
}

FUSIONPHYSICS::CollisionBox3DAABB::CollisionBox3DAABB(FUSIONCORE::WorldTransform& transformation, glm::vec3 BoxSizeCoeff)
{
	this->ModelOriginPoint = transformation.Position;
	FUSIONCORE::Vertex vertex;

	auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::uniform_real_distribution<float> RandomFloats(0.0f, 1.0f);
	std::default_random_engine engine(seed);

	MeshColor.x = RandomFloats(engine);
	MeshColor.y = RandomFloats(engine);
	MeshColor.z = RandomFloats(engine);

	float Xsize = (transformation.InitialObjectScales.x / 2.0f) * BoxSizeCoeff.x;
	float Ysize = (transformation.InitialObjectScales.y / 2.0f) * BoxSizeCoeff.y;
	float Zsize = (transformation.InitialObjectScales.z / 2.0f) * BoxSizeCoeff.z;

	auto OriginPosition = *transformation.OriginPoint;

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

	const glm::vec3 Normals[] = {
		{1.0f,0.0f,0.0f},
		{-1.0f,0.0f,0.0f},
		{0.0f,1.0f,0.0f},
		{0.0f,-1.0f,0.0f},
		{0.0f,0.0f,1.0f},
		{0.0f,0.0f,-1.0f}
	};
	BoxNormals.assign(Normals, Normals + 6);

	this->GetTransformation().TranslationMatrix = transformation.TranslationMatrix;
	this->GetTransformation().ScalingMatrix = transformation.ScalingMatrix;
	this->GetTransformation().RotationMatrix = transformation.RotationMatrix;
	this->GetTransformation().OriginPoint = transformation.OriginPoint;

	LocalBoxNormals.clear();
	auto rotation = glm::toQuat(GetTransformation().RotationMatrix);
	for (size_t i = 0; i < this->BoxNormals.size(); i++)
	{
		glm::vec3 Normal = glm::rotate(rotation, BoxNormals[i]);
		LocalBoxNormals.push_back(glm::normalize(Normal));
	}

	for (size_t i = 0; i < 6; i++)
	{
		if (i != 2 && i != 3)
		{
			LocalEdgeNormals.push_back(BoxNormals[2] + BoxNormals[i]);
			LocalEdgeNormals.push_back(BoxNormals[3] + BoxNormals[i]);
		}
	}

	LocalEdgeNormals.push_back(BoxNormals[4] + BoxNormals[0]);
	LocalEdgeNormals.push_back(BoxNormals[4] + BoxNormals[1]);
	LocalEdgeNormals.push_back(BoxNormals[5] + BoxNormals[0]);
	LocalEdgeNormals.push_back(BoxNormals[5] + BoxNormals[1]);

	auto MinMax = FindMinMax(BoxVertices, this->GetTransformation().GetModelMat4());

	Min = MinMax.first;
	Max = MinMax.second;

	std::vector<FUSIONCORE::Texture2D> textures;

	std::vector<std::shared_ptr<FUSIONCORE::Face>> MeshFaces;
	for (size_t i = 0; i < (sizeof(indices) / sizeof(indices[0])); i += 3)
	{
		FUSIONCORE::Face newFace;
		newFace.Indices.push_back(indices[i]);
		newFace.Indices.push_back(indices[i + 1]);
		newFace.Indices.push_back(indices[i + 2]);
		MeshFaces.push_back(std::make_shared < FUSIONCORE::Face>(newFace));
	}

	std::vector<std::shared_ptr<FUSIONCORE::Vertex>> sharedPtrVertices;
	sharedPtrVertices.reserve(this->BoxVertices.size());

	for (const auto& vertex : BoxVertices) {
	
		sharedPtrVertices.push_back(std::make_shared<FUSIONCORE::Vertex>(vertex));
	}

	BoxMesh = std::make_unique<FUSIONCORE::Mesh>(sharedPtrVertices, BoxIndices, MeshFaces,textures);
}


FUSIONPHYSICS::CollisionBox3DAABB::CollisionBox3DAABB(glm::vec3 Size, glm::vec3 BoxSizeCoeff)
{
	this->ModelOriginPoint = FF_ORIGIN;
	FUSIONCORE::Vertex vertex;

	auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::uniform_real_distribution<float> RandomFloats(0.0f, 1.0f);
	std::default_random_engine engine(seed);

	MeshColor.x = RandomFloats(engine);
	MeshColor.y = RandomFloats(engine);
	MeshColor.z = RandomFloats(engine);

	float Xsize = (Size.x / 2.0f) * BoxSizeCoeff.x;
	float Ysize = (Size.y / 2.0f) * BoxSizeCoeff.y;
	float Zsize = (Size.z / 2.0f) * BoxSizeCoeff.z;

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

	const glm::vec3 Normals[] = {
		{1.0f,0.0f,0.0f},
		{-1.0f,0.0f,0.0f},
		{0.0f,1.0f,0.0f},
		{0.0f,-1.0f,0.0f},
		{0.0f,0.0f,1.0f},
		{0.0f,0.0f,-1.0f}
	};
	BoxNormals.assign(Normals, Normals + 6);

	LocalBoxNormals.clear();
	auto rotation = glm::toQuat(GetTransformation().RotationMatrix);
	for (size_t i = 0; i < this->BoxNormals.size(); i++)
	{
		glm::vec3 Normal = glm::rotate(rotation, BoxNormals[i]);
		LocalBoxNormals.push_back(glm::normalize(Normal));
	}

	auto MinMax = FindMinMax(BoxVertices, this->GetTransformation().GetModelMat4());

	Min = MinMax.first;
	Max = MinMax.second;

	std::vector<FUSIONCORE::Texture2D> textures;
	std::vector<std::shared_ptr<FUSIONCORE::Face>> MeshFaces;
	for (size_t i = 0; i < (sizeof(indices) / sizeof(indices[0])); i += 3)
	{
		FUSIONCORE::Face newFace;
		newFace.Indices.push_back(indices[i]);
		newFace.Indices.push_back(indices[i + 1]);
		newFace.Indices.push_back(indices[i + 2]);
		MeshFaces.push_back(std::make_shared<FUSIONCORE::Face>(newFace));
	}

	std::vector<std::shared_ptr<FUSIONCORE::Vertex>> sharedPtrVertices;
	sharedPtrVertices.reserve(this->BoxVertices.size());

	for (const auto& vertex : BoxVertices) {

		sharedPtrVertices.push_back(std::make_shared<FUSIONCORE::Vertex>(vertex));
	}

	BoxMesh = std::make_unique<FUSIONCORE::Mesh>(sharedPtrVertices, BoxIndices, MeshFaces, textures);
}

FUSIONPHYSICS::CollisionBox::CollisionBox(FUSIONCORE::Mesh &InputMesh, FUSIONCORE::WorldTransform transformation)
{
	auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::uniform_real_distribution<float> RandomFloats(0.0f, 1.0f);
	std::default_random_engine engine(seed);

	MeshColor.x = RandomFloats(engine);
	MeshColor.y = RandomFloats(engine);
	MeshColor.z = RandomFloats(engine);

	this->BoxMesh = std::make_unique<FUSIONCORE::Mesh>(InputMesh);

	this->GetTransformation().TranslationMatrix = transformation.TranslationMatrix;
	this->GetTransformation().ScalingMatrix = transformation.ScalingMatrix;
	this->GetTransformation().RotationMatrix = transformation.RotationMatrix;
	this->GetTransformation().OriginPoint = transformation.OriginPoint;

	auto& MeshVertices = BoxMesh->GetVertices();
	this->BoxNormals.reserve(MeshVertices.size());
	this->BoxVertices.reserve(MeshVertices.size());
	for (size_t i = 0; i < MeshVertices.size(); i++)
	{
		this->BoxNormals.push_back(MeshVertices[i]->Normal);
		this->BoxVertices.push_back(*MeshVertices[i]);
	}

	std::unordered_set<glm::vec3, FUSIONCORE::Vec3Hash> uniqueNormals(BoxNormals.begin(), BoxNormals.end());
	BoxNormals.assign(uniqueNormals.begin(), uniqueNormals.end());

	std::unordered_set<FUSIONCORE::Vertex , FUSIONCORE::VertexHash> uniqueVertices(BoxVertices.begin(), BoxVertices.end());
	BoxVertices.assign(uniqueVertices.begin(), uniqueVertices.end());
	
	auto rotation = glm::toQuat(GetTransformation().RotationMatrix);
	for (size_t i = 0; i < this->BoxNormals.size(); i++)
	{
		glm::vec3 Normal = glm::rotate(rotation, BoxNormals[i]);
		LocalBoxNormals.push_back(glm::normalize(Normal));
	}

	auto MinMax = FindMinMax(BoxVertices, this->GetTransformation().GetModelMat4());
	Min = MinMax.first;
	Max = MinMax.second;
}

void FUSIONPHYSICS::CollisionBox::DrawBoxMesh(FUSIONCORE::Camera3D& camera, FUSIONCORE::Shader& shader)
{
	std::function<void()> boxprep = [&]()
		{
			shader.setVec3("LightColor", MeshColor);
			shader.setMat4("model", GetTransformation().GetModelMat4());
		};
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	BoxMesh->Draw(camera, shader, boxprep);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void FUSIONPHYSICS::CollisionBox::Clean()
{
	this->BoxMesh->Clean();
}

void FUSIONPHYSICS::CollisionBox::Update()
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

	LocalBoxNormals.clear();
	auto rotation = glm::toQuat(GetTransformation().RotationMatrix);
	for (size_t i = 0; i < this->BoxNormals.size(); i++)
	{
		glm::vec3 Normal = glm::rotate(rotation, BoxNormals[i]);
		LocalBoxNormals.push_back(glm::normalize(Normal));
	}

	auto MinMax = FindMinMax(BoxVertices, this->GetTransformation().GetModelMat4());
	Min = MinMax.first;
	Max = MinMax.second;
}

void FUSIONPHYSICS::CollisionBox::UpdateAttributes()
{
	LocalBoxNormals.clear();
	auto rotation = glm::toQuat(GetTransformation().RotationMatrix);
	for (size_t i = 0; i < this->BoxNormals.size(); i++)
	{
		glm::vec3 Normal = glm::rotate(rotation, BoxNormals[i]);
		LocalBoxNormals.push_back(glm::normalize(Normal));
	}

	auto MinMax = FindMinMax(BoxVertices, this->GetTransformation().GetModelMat4());
	Min = MinMax.first;
	Max = MinMax.second;
}

void FUSIONPHYSICS::CollisionBox3DAABB::UpdateAttributes()
{
	LocalBoxNormals.clear();
	auto rotation = glm::toQuat(GetTransformation().RotationMatrix);
	for (size_t i = 0; i < this->BoxNormals.size(); i++)
	{
		glm::vec3 Normal = glm::rotate(rotation, BoxNormals[i]);
		LocalBoxNormals.push_back(glm::normalize(Normal));
	}

	LocalEdgeNormals.clear();
	for (size_t i = 0; i < 6; i++)
	{
		if (i != 2 && i != 3)
		{
			LocalEdgeNormals.push_back(LocalBoxNormals[2] + LocalBoxNormals[i]);
			LocalEdgeNormals.push_back(LocalBoxNormals[3] + LocalBoxNormals[i]);
		}
	}

	LocalEdgeNormals.push_back(LocalBoxNormals[4] + LocalBoxNormals[0]);
	LocalEdgeNormals.push_back(LocalBoxNormals[4] + LocalBoxNormals[1]);
	LocalEdgeNormals.push_back(LocalBoxNormals[5] + LocalBoxNormals[0]);
	LocalEdgeNormals.push_back(LocalBoxNormals[5] + LocalBoxNormals[1]);

	auto MinMax = FindMinMax(BoxVertices, this->GetTransformation().GetModelMat4());

	Min = MinMax.first;
	Max = MinMax.second;
}

float FUSIONPHYSICS::CollisionBox3DAABB::ProjectOntoAxis(const glm::vec3& axis)
{

	glm::vec4 transformed0 = this->GetTransformation().GetModelMat4() * glm::vec4(BoxVertices[0].Position, 1.0f);
	float minProjection = glm::dot({ transformed0.x, transformed0.y, transformed0.z }, axis);

	for (const auto& vertex : BoxVertices)
	{
		glm::vec4 transformed = this->GetTransformation().GetModelMat4() * glm::vec4(vertex.Position, 1.0f);
		float projection = glm::dot({ transformed.x, transformed.y, transformed.z }, axis);

		if (projection < minProjection)
			minProjection = projection;
	}

	return minProjection;
}

void FUSIONPHYSICS::CollisionBox3DAABB::Clear()
{
	this->GetBoxMesh()->Clean();
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

	LocalBoxNormals.clear();
	auto rotation = glm::toQuat(GetTransformation().RotationMatrix);
	for (size_t i = 0; i < this->BoxNormals.size(); i++)
	{
		glm::vec3 Normal = glm::rotate(rotation, BoxNormals[i]);
		LocalBoxNormals.push_back(glm::normalize(Normal));
	}

	LocalEdgeNormals.clear();
	for (size_t i = 0; i < 6; i++)
	{
		if (i != 2 && i != 3)
		{
			LocalEdgeNormals.push_back(LocalBoxNormals[2] + LocalBoxNormals[i]);
			LocalEdgeNormals.push_back(LocalBoxNormals[3] + LocalBoxNormals[i]);
		}
	}

	LocalEdgeNormals.push_back(LocalBoxNormals[4] + LocalBoxNormals[0]);
	LocalEdgeNormals.push_back(LocalBoxNormals[4] + LocalBoxNormals[1]);
	LocalEdgeNormals.push_back(LocalBoxNormals[5] + LocalBoxNormals[0]);
	LocalEdgeNormals.push_back(LocalBoxNormals[5] + LocalBoxNormals[1]);

	auto MinMax = FindMinMax(BoxVertices, this->GetTransformation().GetModelMat4());

	Min = MinMax.first;
	Max = MinMax.second;

	UpdateChildren();
}


std::pair<int, glm::vec3> FUSIONPHYSICS::CheckCollisionDirection(glm::vec3 targetVector, FUSIONCORE::WorldTransform &Entity1Transformation)
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

	glm::vec3 transformed;
	auto rotation = glm::toQuat(Entity1Transformation.RotationMatrix);
	for (size_t i = 0; i < 6; i++)
	{
		transformed = glm::rotate(rotation, Directions[i]);
		float dotProduct = glm::dot(glm::normalize(transformed), glm::normalize(targetVector));

		if (dotProduct > max)
		{
			max = dotProduct;
			Chosen = i;
			ChosenAxis = transformed;
		}
	}

	return { Chosen,ChosenAxis };
}

std::pair<int, glm::vec3> FUSIONPHYSICS::CheckCollisionDirection(glm::vec3 targetVector, std::vector<glm::vec3> Normals)
{
	float max = 0.0f;
	int Chosen = -1;
	glm::vec3 ChosenAxis = glm::vec3(0.0f);

	glm::vec4 transformed;
	for (size_t i = 0; i < Normals.size(); i++)
	{
		float dotProduct = glm::dot(glm::normalize(Normals[i]), glm::normalize(targetVector));

		if (dotProduct > max)
		{
			max = dotProduct;
			Chosen = i;
			ChosenAxis = glm::normalize(Normals[i]);
		}
	}

	return { Chosen,ChosenAxis };
}

std::pair<bool, int> FUSIONPHYSICS::BoxBoxIntersect(CollisionBox3DAABB& Box1, CollisionBox3DAABB& Box2)
{
	auto direction = CheckCollisionDirection(Box1.GetTransformation().Position - Box2.GetTransformation().Position,
		Box1.GetTransformation());

	return{ Box1.Min.x <= Box2.Max.x &&
		   Box1.Max.x >= Box2.Min.x &&
		   Box1.Min.y <= Box2.Max.y &&
		   Box1.Max.y >= Box2.Min.y &&
		   Box1.Min.z <= Box2.Max.z &&
		   Box1.Max.z >= Box2.Min.z , direction.first };
}

bool FUSIONPHYSICS::FindMinSeparation(CollisionBox& Box1, CollisionBox& Box2, glm::vec3 Axis)
{
	if (Axis == glm::vec3(0.0f))
	{
		return true;
	}

	float maxProjectionA = FLT_MIN;
	float minProjectionA = FLT_MAX;
	for (size_t i = 0; i < Box1.GetVertices().size(); i++)
	{
		glm::vec4 vertexWorldPositon = glm::vec4(FUSIONCORE::TranslateVertex(Box1.GetTransformation().GetModelMat4(), Box1.GetVertices()[i].Position), 1.0f);
		float projection = glm::dot({ vertexWorldPositon.x,vertexWorldPositon.y,vertexWorldPositon.z }, Axis);
		maxProjectionA = glm::max(maxProjectionA, projection);
		minProjectionA = glm::min(minProjectionA, projection);
	}

	float maxProjectionB = FLT_MIN;
	float minProjectionB = FLT_MAX;
	for (size_t i = 0; i < Box2.GetVertices().size(); i++)
	{
		glm::vec4 vertexWorldPositon = glm::vec4(FUSIONCORE::TranslateVertex(Box2.GetTransformation().GetModelMat4(), Box2.GetVertices()[i].Position), 1.0f);
		float projection = glm::dot({ vertexWorldPositon.x,vertexWorldPositon.y,vertexWorldPositon.z }, Axis);
		maxProjectionB = glm::max(maxProjectionB, projection);
		minProjectionB = glm::min(minProjectionB, projection);
	}

	return (minProjectionA <= maxProjectionB && maxProjectionA >= minProjectionB) ||
		   (minProjectionB <= maxProjectionA && maxProjectionB >= minProjectionA);
}


bool FUSIONPHYSICS::IsCollidingSAT(CollisionBox3DAABB& Box1, CollisionBox3DAABB& Box2)
{
	auto& localNormalsBox1 = Box1.GetLocalNormals();
	auto& LocalEdgeNormalsBox1 = Box1.GetLocalEdgeNormals();
	auto LocalNormalBox1Size = localNormalsBox1.size();
	auto LocalEdgeBox1Size = LocalEdgeNormalsBox1.size();

	std::vector<glm::vec3> listOfAxisToCheck;
	listOfAxisToCheck.reserve(LocalNormalBox1Size + LocalEdgeBox1Size);

	for (size_t i = 0; i < LocalNormalBox1Size; i++)
	{
		listOfAxisToCheck[i] = localNormalsBox1[i];
	}

	for (size_t i = 0; i < LocalEdgeBox1Size; i++)
	{
		listOfAxisToCheck[LocalNormalBox1Size + i] = LocalEdgeNormalsBox1[i];
	}

	for (size_t i = 0; i < LocalNormalBox1Size + LocalEdgeBox1Size; i++)
	{
		if (!FindMinSeparation(Box1, Box2, listOfAxisToCheck[i]))
		{
			return false;
		}
	}

	auto& LocalEdgeNormalsBox2 = Box2.GetLocalEdgeNormals();
	auto LocalEdgeBox2Size = LocalEdgeNormalsBox2.size();
	auto& localNormalsBox2 = Box2.GetLocalNormals();
	auto LocalNormalBox2Size = localNormalsBox2.size();

	for (size_t i = 0; i < LocalNormalBox2Size; i++)
	{
		listOfAxisToCheck[i] = localNormalsBox2[i];
	}

	for (size_t i = 0; i < LocalEdgeBox2Size; i++)
	{
		listOfAxisToCheck[LocalNormalBox2Size + i] = LocalEdgeNormalsBox2[i];
	}

	for (size_t i = 0; i < LocalNormalBox2Size + LocalEdgeBox2Size; i++)
	{
		if (!FindMinSeparation(Box1, Box2, listOfAxisToCheck[i]))
		{
			return false;
		}
	}

	return true;
}

bool FUSIONPHYSICS::IsCollidingSAT(CollisionBox& Plane, CollisionBox3DAABB& Box)
{
	auto& localNormalsBox1 = Box.GetLocalNormals();
	auto& LocalEdgeNormalsBox1 = Box.GetLocalEdgeNormals();
	auto LocalNormalBox1Size = localNormalsBox1.size();
	auto LocalEdgeBox1Size = LocalEdgeNormalsBox1.size();

	std::vector<glm::vec3> listOfAxisToCheck;
	listOfAxisToCheck.reserve(LocalNormalBox1Size + LocalEdgeBox1Size);

	for (size_t i = 0; i < LocalNormalBox1Size; i++)
	{
		listOfAxisToCheck[i] = localNormalsBox1[i];
	}

	for (size_t i = 0; i < LocalEdgeBox1Size; i++)
	{
		listOfAxisToCheck[LocalNormalBox1Size + i] = LocalEdgeNormalsBox1[i];
	}

	for (size_t i = 0; i < LocalNormalBox1Size + LocalEdgeBox1Size; i++)
	{
		if (!FindMinSeparation(Plane, Box, listOfAxisToCheck[i]))
		{
			return false;
			break;
		}
	}

	auto& PlaneLocalNormals = Plane.GetLocalNormals();
	for (size_t i = 0; i < PlaneLocalNormals.size(); i++)
	{
		if (!FindMinSeparation(Plane, Box, PlaneLocalNormals[i]))
		{
			return false;
			break;
		}
	}

	return true;
}

bool FUSIONPHYSICS::IsCollidingSAT(CollisionBox& Box1, CollisionBox& Box2)
{
	auto& Box1LocalNormals = Box1.GetLocalNormals();
	for (size_t i = 0; i < Box1LocalNormals.size(); i++)
	{
		if (!FindMinSeparation(Box1, Box2, Box1LocalNormals[i]))
		{
			return false;
			break;
		}
	}

	auto& Box2LocalNormals = Box2.GetLocalNormals();
	for (size_t i = 0; i < Box2LocalNormals.size(); i++)
	{
		if (!FindMinSeparation(Box1, Box2, Box2LocalNormals[i]))
		{
			return false;
			break;
		}
	}

	return true;
}

FUSIONPHYSICS::CollisionBoxPlane::CollisionBoxPlane(glm::vec3 Size, glm::vec3 BoxSizeCoeff)
{
	this->ModelOriginPoint = FF_ORIGIN;
	FUSIONCORE::Vertex vertex;

	auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::uniform_real_distribution<float> RandomFloats(0.0f, 1.0f);
	std::default_random_engine engine(seed);

	MeshColor.x = RandomFloats(engine);
	MeshColor.y = RandomFloats(engine);
	MeshColor.z = RandomFloats(engine);

	float Xsize = (Size.x / 2.0f) * BoxSizeCoeff.x;
	float Ysize = (Size.y / 2.0f) * BoxSizeCoeff.y;
	float Zsize = (Size.z / 2.0f) * BoxSizeCoeff.z;

	auto OriginPosition = FF_ORIGIN;

	//Upper Part
	vertex.Position.x = OriginPosition.x + Xsize;
	vertex.Position.y = OriginPosition.y;
	vertex.Position.z = OriginPosition.z + Zsize;

	BoxVertices.push_back(vertex);

	vertex.Position.x = OriginPosition.x + Xsize;
	vertex.Position.y = OriginPosition.y;
	vertex.Position.z = OriginPosition.z - Zsize;

	BoxVertices.push_back(vertex);

	vertex.Position.x = OriginPosition.x - Xsize;
	vertex.Position.y = OriginPosition.y;
	vertex.Position.z = OriginPosition.z - Zsize;

	BoxVertices.push_back(vertex);

	vertex.Position.x = OriginPosition.x - Xsize;
	vertex.Position.y = OriginPosition.y;
	vertex.Position.z = OriginPosition.z + Zsize;

	BoxVertices.push_back(vertex);

	const GLuint indices[] = {
		// Upper part
		0, 1, 2,0,
		2, 3, 0,2
	};

	BoxIndices.assign(indices, indices + sizeof(indices) / sizeof(indices[0]));

	const glm::vec3 Normals[] = {
		{0.0f,1.0f,0.0f},
		{0.0f,-1.0f,0.0f}
	};
	BoxNormals.assign(Normals, Normals + sizeof(Normals) / sizeof(Normals[0]));

	glm::vec4 transformedOrigin = this->GetTransformation().GetModelMat4() * glm::vec4(this->ModelOriginPoint, 1.0f);

	FUSIONCORE::Face face;

	face.Vertices.assign(BoxVertices.begin() , BoxVertices.end());
	face.Normal = FindNormal(this->GetTransformation().GetModelMat4(), face.Vertices);

	auto faceNormal = face.GetNormal();

	LocalBoxNormals.push_back(faceNormal);
	LocalBoxNormals.push_back({ faceNormal.x , -faceNormal.y , faceNormal.z });

	Faces.push_back(face);

	const GLuint EdgeIndices[5] = {
		0, 1,
		2, 3,
		0
	};

	for (size_t i = 0; i < 4; i++)
	{
		auto Vertex1 = FUSIONCORE::TranslateVertex(this->GetTransformation().GetModelMat4(), BoxVertices[EdgeIndices[i]].Position);
		auto Vertex2 = FUSIONCORE::TranslateVertex(this->GetTransformation().GetModelMat4(), BoxVertices[EdgeIndices[i + 1]].Position);
		glm::vec3 Edge = Vertex1 - Vertex2;
		glm::vec3 EdgeMidPoint = (Vertex1 + Vertex2) / glm::vec3(2.0f);
		glm::vec3 EdgeNormal = glm::cross(LocalBoxNormals[0], Edge);
		EdgeNormal = glm::normalize(EdgeNormal);
		LocalBoxNormals.push_back(EdgeNormal);
	}

	auto MinMax = FindMinMax(BoxVertices, this->GetTransformation().GetModelMat4());

	Min = MinMax.first;
	Max = MinMax.second;

	std::vector<FUSIONCORE::Texture2D> textures;
	std::vector<std::shared_ptr<FUSIONCORE::Face>> MeshFaces;
	for (size_t i = 0; i < (sizeof(indices) / sizeof(indices[0])); i += 4)
	{
		FUSIONCORE::Face newFace;
		newFace.Indices.push_back(indices[i]);
		newFace.Indices.push_back(indices[i + 1]);
		newFace.Indices.push_back(indices[i + 2]);
		MeshFaces.push_back(std::make_shared<FUSIONCORE::Face>(newFace));
	}

	std::vector<std::shared_ptr<FUSIONCORE::Vertex>> sharedPtrVertices;
	sharedPtrVertices.reserve(this->BoxVertices.size());

	for (const auto& vertex : BoxVertices) {

		sharedPtrVertices.push_back(std::make_shared<FUSIONCORE::Vertex>(vertex));
	}

	BoxMesh = std::make_unique<FUSIONCORE::Mesh>(sharedPtrVertices, BoxIndices, MeshFaces, textures);
}

void FUSIONPHYSICS::CollisionBoxPlane::Clear()
{
	this->GetBoxMesh()->Clean();
}

void FUSIONPHYSICS::CollisionBoxPlane::Update()
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

	LocalBoxNormals.clear();
	glm::vec4 transformedOrigin = this->GetTransformation().GetModelMat4() * glm::vec4(this->ModelOriginPoint, 1.0f);

	Faces[0].Normal = FindNormal(this->GetTransformation().GetModelMat4() , Faces[0].Vertices);
	auto faceNormal = Faces[0].GetNormal();

	LocalBoxNormals.push_back(faceNormal);
	LocalBoxNormals.push_back({ faceNormal.x , -faceNormal.y , faceNormal.z });

	const GLuint EdgeIndices[5] = {
		0, 1,
		2, 3,
		0
	};

	for (size_t i = 0; i < 4; i++)
	{
		auto Vertex1 = FUSIONCORE::TranslateVertex(this->GetTransformation().GetModelMat4(), BoxVertices[EdgeIndices[i]].Position);
		auto Vertex2 = FUSIONCORE::TranslateVertex(this->GetTransformation().GetModelMat4(), BoxVertices[EdgeIndices[i + 1]].Position);
		glm::vec3 Edge = Vertex1 - Vertex2;
		glm::vec3 EdgeMidPoint = (Vertex1 + Vertex2) / glm::vec3(2.0f);
		glm::vec3 EdgeNormal = glm::cross(LocalBoxNormals[0], Edge);
		EdgeNormal = glm::normalize(EdgeNormal);

		LocalBoxNormals.push_back(EdgeNormal);
	}

	auto MinMax = FindMinMax(BoxVertices, this->GetTransformation().GetModelMat4());

	Min = MinMax.first;
	Max = MinMax.second;

	UpdateChildren();
}

void FUSIONPHYSICS::CollisionBoxPlane::UpdateAttributes()
{
	LocalBoxNormals.clear();
	glm::vec4 transformedOrigin = this->GetTransformation().GetModelMat4() * glm::vec4(this->ModelOriginPoint, 1.0f);

	Faces[0].Normal = FindNormal(this->GetTransformation().GetModelMat4(), Faces[0].Vertices);
	auto faceNormal = Faces[0].GetNormal();

	LocalBoxNormals.push_back(faceNormal);
	LocalBoxNormals.push_back({ faceNormal.x , -faceNormal.y , faceNormal.z });

	const GLuint EdgeIndices[5] = {
		0, 1,
		2, 3,
		0
	};

	for (size_t i = 0; i < 4; i++)
	{
		auto Vertex1 = FUSIONCORE::TranslateVertex(this->GetTransformation().GetModelMat4(), BoxVertices[EdgeIndices[i]].Position);
		auto Vertex2 = FUSIONCORE::TranslateVertex(this->GetTransformation().GetModelMat4(), BoxVertices[EdgeIndices[i + 1]].Position);
		glm::vec3 Edge = Vertex1 - Vertex2;
		glm::vec3 EdgeMidPoint = (Vertex1 + Vertex2) / glm::vec3(2.0f);
		glm::vec3 EdgeNormal = glm::cross(LocalBoxNormals[0], Edge);
		EdgeNormal = glm::normalize(EdgeNormal);
		LocalBoxNormals.push_back(EdgeNormal);
	}

	auto MinMax = FindMinMax(BoxVertices, this->GetTransformation().GetModelMat4());

	Min = MinMax.first;
	Max = MinMax.second;
}
