#include "Physics.hpp"
#include <time.h>
#include <random>
#include <chrono>

std::pair<glm::vec3, glm::vec3> FindMinMax(std::vector<FUSIONOPENGL::Vertex> BoxVertices, glm::mat4 ModelMat4)
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

std::vector<FUSIONOPENGL::Vertex> FindPointsOnDirection(const glm::vec3& direction, const std::vector<FUSIONOPENGL::Vertex>& Vertices)
{
	std::vector<FUSIONOPENGL::Vertex> PointsOnDirection;

	for (const FUSIONOPENGL::Vertex& vertex : Vertices) {
		if (SameDirection(vertex.Position, direction)) {
			PointsOnDirection.push_back(vertex);
		}
	}

	return PointsOnDirection;
}

FUSIONPHYSICS::CollisionBox3DAABB::CollisionBox3DAABB(FUSIONOPENGL::WorldTransform& transformation, glm::vec3 BoxSizeCoeff)
{
	this->ModelOriginPoint = transformation.Position;
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

	for (size_t i = 0; i < BoxNormals.size(); i++)
	{
		auto face = FindPointsOnDirection(BoxNormals[i], BoxVertices);
		if (face.size() == 4)
		{
			Faces.emplace_back(face);
			Faces.back().FindNormal();

			auto Normal = Faces.back().GetNormal();
			glm::vec4 transformedOrigin = this->GetTransformation().GetModelMat4() * glm::vec4(*this->GetTransformation().OriginPoint, 1.0f);

			auto Difference = glm::vec3(transformedOrigin.x, transformedOrigin.y, transformedOrigin.z) - face[0].Position;

			if (Difference.x < 0)
			{
				Normal.x *= -1;
			}
			if (Difference.y < 0)
			{
				Normal.y *= -1;
			}
			if (Difference.z < 0)
			{
				Normal.z *= -1;
			}

			Faces.back().SetNormal(Normal);
			//LOG(Vec3<float>(Normal));
		}

	}

	auto MinMax = FindMinMax(BoxVertices, this->GetTransformation().GetModelMat4());

	Min = MinMax.first;
	Min = MinMax.second;

	std::vector<FUSIONOPENGL::Texture2D> textures;
	BoxMesh = std::make_unique<FUSIONOPENGL::Mesh3D>(BoxVertices, BoxIndices, textures);
}

FUSIONPHYSICS::CollisionBox3DAABB::CollisionBox3DAABB(glm::vec3 Size, glm::vec3 BoxSizeCoeff)
{
	this->ModelOriginPoint = FF_ORIGIN;
	FUSIONOPENGL::Vertex vertex;

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

	for (size_t i = 0; i < BoxNormals.size(); i++)
	{
		auto face = FindPointsOnDirection(BoxNormals[i], BoxVertices);
		if (face.size() == 4)
		{
			Faces.emplace_back(face);
			Faces.back().FindNormal();

			auto Normal = Faces.back().GetNormal();
			glm::vec4 transformedOrigin = this->GetTransformation().GetModelMat4() * glm::vec4(this->ModelOriginPoint, 1.0f);

			auto Difference = glm::vec3(transformedOrigin.x, transformedOrigin.y, transformedOrigin.z) - face[0].Position;

			if (Difference.x < 0)
			{
				Normal.x *= -1;
			}
			if (Difference.y < 0)
			{
				Normal.y *= -1;
			}
			if (Difference.z < 0)
			{
				Normal.z *= -1;
			}

			Faces.back().SetNormal(Normal);
			LocalBoxNormals.push_back(Normal);
		}

	}

	auto MinMax = FindMinMax(BoxVertices, this->GetTransformation().GetModelMat4());

	Min = MinMax.first;
	Min = MinMax.second;

	std::vector<FUSIONOPENGL::Texture2D> textures;
	BoxMesh = std::make_unique<FUSIONOPENGL::Mesh3D>(BoxVertices, BoxIndices, textures);
}

void FUSIONPHYSICS::CollisionBox::DrawBoxMesh(FUSIONOPENGL::Camera3D& camera, FUSIONOPENGL::Shader& shader)
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

void FUSIONPHYSICS::CollisionBox3DAABB::UpdateAttributes()
{
	LocalBoxNormals.clear();
	for (size_t i = 0; i < Faces.size(); i++)
	{
		Faces[i].FindNormal(this->GetTransformation().GetModelMat4());
		auto Normal = Faces[i].GetNormal();
		glm::vec4 transformedOrigin = this->GetTransformation().GetModelMat4() * glm::vec4(this->ModelOriginPoint, 1.0f);

		auto Difference = glm::vec3(transformedOrigin.x, transformedOrigin.y, transformedOrigin.z) - Faces[i].GetVertices()[0].Position;

		if (Difference.x < 0)
		{
			Normal.x *= -1;
		}
		if (Difference.y < 0)
		{
			Normal.y *= -1;
		}
		if (Difference.z < 0)
		{
			Normal.z *= -1;
		}

		Faces.back().SetNormal(Normal);
		LocalBoxNormals.push_back(Normal);
		//LOG("Normal: " << Vec3<float>(Normal));
	}

	auto MinMax = FindMinMax(BoxVertices, this->GetTransformation().GetModelMat4());

	Min = MinMax.first;
	Min = MinMax.second;
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
	for (size_t i = 0; i < Faces.size(); i++)
	{
		Faces[i].FindNormal(this->GetTransformation().GetModelMat4());
		auto Normal = Faces[i].GetNormal();
		glm::vec4 transformedOrigin = this->GetTransformation().GetModelMat4() * glm::vec4(*this->GetTransformation().OriginPoint, 1.0f);

		auto Difference = glm::vec3(transformedOrigin.x, transformedOrigin.y, transformedOrigin.z) - Faces[i].GetVertices()[0].Position;

		if (Difference.x < 0)
		{
			Normal.x *= -1;
		}
		if (Difference.y < 0)
		{
			Normal.y *= -1;
		}
		if (Difference.z < 0)
		{
			Normal.z *= -1;
		}

		Faces.back().SetNormal(Normal);
		LocalBoxNormals.push_back(Normal);
	}

	auto MinMax = FindMinMax(BoxVertices, this->GetTransformation().GetModelMat4());

	Min = MinMax.first;
	Min = MinMax.second;

	UpdateChildren();
}


std::pair<int, glm::vec3> FUSIONPHYSICS::CheckCollisionDirection(glm::vec3 targetVector, glm::mat4 Entity1ModelMatrix)
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
		float dotProduct = glm::dot(glm::normalize(glm::vec3(transformed.x, transformed.y, transformed.z)), glm::normalize(targetVector));

		if (dotProduct > max)
		{
			max = dotProduct;
			Chosen = i;
			//ChosenAxis = glm::normalize(glm::vec3(transformed.x, transformed.y, transformed.z));
			ChosenAxis = Directions[i];
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
		Box1.GetTransformation().GetModelMat4());

	return{ Box1.Min.x <= Box2.Max.x &&
		   Box1.Max.x >= Box2.Min.x &&
		   Box1.Min.y <= Box2.Max.y &&
		   Box1.Max.y >= Box2.Min.y &&
		   Box1.Min.z <= Box2.Max.z &&
		   Box1.Max.z >= Box2.Min.z , direction.first };
}

float FUSIONPHYSICS::FindMinSeparation(CollisionBox& Box1, CollisionBox& Box2, glm::vec3 Axis)
{
	if (Axis == glm::vec3(0.0f))
		return true;
	float maxProjectionA = FLT_MIN;
	float minProjectionA = FLT_MAX;
	for (size_t i = 0; i < Box1.GetVertices().size(); i++)
	{
		glm::vec4 vertexWorldPositon = glm::vec4(FUSIONOPENGL::TranslateVertex(Box1.GetTransformation().GetModelMat4(), Box1.GetVertices()[i].Position), 1.0f);
		float projection = vertexWorldPositon.x * Axis.x + vertexWorldPositon.y * Axis.y + vertexWorldPositon.z * Axis.z;
		maxProjectionA = glm::max(maxProjectionA, projection);
		minProjectionA = glm::min(minProjectionA, projection);
	}

	float maxProjectionB = FLT_MIN;
	float minProjectionB = FLT_MAX;
	for (size_t i = 0; i < Box2.GetVertices().size(); i++)
	{
		glm::vec4 vertexWorldPositon = glm::vec4(FUSIONOPENGL::TranslateVertex(Box2.GetTransformation().GetModelMat4(), Box2.GetVertices()[i].Position), 1.0f);
		float projection = vertexWorldPositon.x * Axis.x + vertexWorldPositon.y * Axis.y + vertexWorldPositon.z * Axis.z;
		maxProjectionB = glm::max(maxProjectionB, projection);
		minProjectionB = glm::min(minProjectionB, projection);
	}

	return (minProjectionA <= maxProjectionB && minProjectionA >= minProjectionB) || (minProjectionB <= maxProjectionA && minProjectionB >= minProjectionA);
}

bool FUSIONPHYSICS::IsCollidingSAT(CollisionBox3DAABB& Box1, CollisionBox3DAABB& Box2)
{
	glm::vec3 listOfAxisToCheck[15];

	listOfAxisToCheck[0] = Box1.GetLocalNormals()[2];
	listOfAxisToCheck[1] = Box1.GetLocalNormals()[0];
	listOfAxisToCheck[2] = Box1.GetLocalNormals()[5];
	listOfAxisToCheck[3] = Box1.GetLocalNormals()[2];
	listOfAxisToCheck[4] = Box1.GetLocalNormals()[0];
	listOfAxisToCheck[5] = Box1.GetLocalNormals()[5];

	listOfAxisToCheck[6] = listOfAxisToCheck[0] * listOfAxisToCheck[3];
	listOfAxisToCheck[7] = listOfAxisToCheck[0] * listOfAxisToCheck[4];
	listOfAxisToCheck[8] = listOfAxisToCheck[0] * listOfAxisToCheck[5];
	listOfAxisToCheck[9] = listOfAxisToCheck[1] * listOfAxisToCheck[3];
	listOfAxisToCheck[10] = listOfAxisToCheck[1] * listOfAxisToCheck[4];
	listOfAxisToCheck[11] = listOfAxisToCheck[1] * listOfAxisToCheck[5];
	listOfAxisToCheck[12] = listOfAxisToCheck[2] * listOfAxisToCheck[3];
	listOfAxisToCheck[13] = listOfAxisToCheck[2] * listOfAxisToCheck[4];
	listOfAxisToCheck[14] = listOfAxisToCheck[2] * listOfAxisToCheck[5];

	for (size_t i = 0; i < 15; i++)
	{
		if (!FindMinSeparation(Box1, Box2, listOfAxisToCheck[i]))
		{
			return false;
			break;
		}
	}

	listOfAxisToCheck[0] = Box2.GetLocalNormals()[2];
	listOfAxisToCheck[1] = Box2.GetLocalNormals()[0];
	listOfAxisToCheck[2] = Box2.GetLocalNormals()[5];
	listOfAxisToCheck[3] = Box2.GetLocalNormals()[2];
	listOfAxisToCheck[4] = Box2.GetLocalNormals()[0];
	listOfAxisToCheck[5] = Box2.GetLocalNormals()[5];

	listOfAxisToCheck[6] = listOfAxisToCheck[0] * listOfAxisToCheck[3];
	listOfAxisToCheck[7] = listOfAxisToCheck[0] * listOfAxisToCheck[4];
	listOfAxisToCheck[8] = listOfAxisToCheck[0] * listOfAxisToCheck[5];
	listOfAxisToCheck[9] = listOfAxisToCheck[1] * listOfAxisToCheck[3];
	listOfAxisToCheck[10] = listOfAxisToCheck[1] * listOfAxisToCheck[4];
	listOfAxisToCheck[11] = listOfAxisToCheck[1] * listOfAxisToCheck[5];
	listOfAxisToCheck[12] = listOfAxisToCheck[2] * listOfAxisToCheck[3];
	listOfAxisToCheck[13] = listOfAxisToCheck[2] * listOfAxisToCheck[4];
	listOfAxisToCheck[14] = listOfAxisToCheck[2] * listOfAxisToCheck[5];

	for (size_t i = 0; i < 15; i++)
	{
		if (!FindMinSeparation(Box1, Box2, listOfAxisToCheck[i]))
		{
			return false;
			break;
		}
	}

	return true;
}

bool FUSIONPHYSICS::IsCollidingSAT(CollisionBox& Plane, CollisionBox3DAABB& Box)
{
	glm::vec3 listOfAxisToCheck[15];

	listOfAxisToCheck[0] = Box.GetLocalNormals()[2];
	listOfAxisToCheck[1] = Box.GetLocalNormals()[0];
	listOfAxisToCheck[2] = Box.GetLocalNormals()[5];
	listOfAxisToCheck[3] = Box.GetLocalNormals()[2];
	listOfAxisToCheck[4] = Box.GetLocalNormals()[0];
	listOfAxisToCheck[5] = Box.GetLocalNormals()[5];

	listOfAxisToCheck[6] = listOfAxisToCheck[0] * listOfAxisToCheck[3];
	listOfAxisToCheck[7] = listOfAxisToCheck[0] * listOfAxisToCheck[4];
	listOfAxisToCheck[8] = listOfAxisToCheck[0] * listOfAxisToCheck[5];
	listOfAxisToCheck[9] = listOfAxisToCheck[1] * listOfAxisToCheck[3];
	listOfAxisToCheck[10] = listOfAxisToCheck[1] * listOfAxisToCheck[4];
	listOfAxisToCheck[11] = listOfAxisToCheck[1] * listOfAxisToCheck[5];
	listOfAxisToCheck[12] = listOfAxisToCheck[2] * listOfAxisToCheck[3];
	listOfAxisToCheck[13] = listOfAxisToCheck[2] * listOfAxisToCheck[4];
	listOfAxisToCheck[14] = listOfAxisToCheck[2] * listOfAxisToCheck[5];

	for (size_t i = 0; i < 15; i++)
	{
		if (!FindMinSeparation(Plane, Box, listOfAxisToCheck[i]))
		{
			return false;
			break;
		}
	}

	for (size_t i = 0; i < Plane.GetLocalNormals().size(); i++)
	{
		if (!FindMinSeparation(Plane, Box, Plane.GetLocalNormals()[i]))
		{
			return false;
			break;
		}
	}

	return true;
}

bool FUSIONPHYSICS::IsCollidingSAT(CollisionBox& Box1, CollisionBox& Box2)
{
	for (size_t i = 0; i < Box1.GetLocalNormals().size(); i++)
	{
		if (!FindMinSeparation(Box1, Box2, Box1.GetLocalNormals()[i]))
		{
			return false;
			break;
		}
	}

	for (size_t i = 0; i < Box1.GetLocalNormals().size(); i++)
	{
		if (!FindMinSeparation(Box1, Box2, Box2.GetLocalNormals()[i]))
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
	FUSIONOPENGL::Vertex vertex;

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

	FUSIONOPENGL::Face<4> face;

	face.SetVertices(BoxVertices);
	face.FindNormal(this->GetTransformation().GetModelMat4());

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
		auto Vertex1 = FUSIONOPENGL::TranslateVertex(this->GetTransformation().GetModelMat4(), BoxVertices[EdgeIndices[i]].Position);
		auto Vertex2 = FUSIONOPENGL::TranslateVertex(this->GetTransformation().GetModelMat4(), BoxVertices[EdgeIndices[i + 1]].Position);
		glm::vec3 Edge = Vertex1 - Vertex2;
		glm::vec3 EdgeMidPoint = (Vertex1 + Vertex2) / glm::vec3(2.0f);
		glm::vec3 EdgeNormal = glm::cross(LocalBoxNormals[0], Edge);
		EdgeNormal = glm::normalize(EdgeNormal);

		//LOG("Edge Normal: " << Vec3<float>(EdgeNormal));


		/*auto Difference = glm::vec3(transformedOrigin.x, transformedOrigin.y, transformedOrigin.z) - EdgeMidPoint;

		if (Difference.x < 0)
		{
			if (EdgeNormal.x != 0.0f)
			{
				EdgeNormal.x *= -1;
			}
		}
		if (Difference.z < 0)
		{
			if (EdgeNormal.z != 0.0f)
			{
				EdgeNormal.z *= -1;
			}
		}*/


		LocalBoxNormals.push_back(EdgeNormal);
	}

	auto MinMax = FindMinMax(BoxVertices, this->GetTransformation().GetModelMat4());

	Min = MinMax.first;
	Min = MinMax.second;

	std::vector<FUSIONOPENGL::Texture2D> textures;
	BoxMesh = std::make_unique<FUSIONOPENGL::Mesh3D>(BoxVertices, BoxIndices, textures);
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

	Faces[0].FindNormal(this->GetTransformation().GetModelMat4());
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
		auto Vertex1 = FUSIONOPENGL::TranslateVertex(this->GetTransformation().GetModelMat4(), BoxVertices[EdgeIndices[i]].Position);
		auto Vertex2 = FUSIONOPENGL::TranslateVertex(this->GetTransformation().GetModelMat4(), BoxVertices[EdgeIndices[i + 1]].Position);
		glm::vec3 Edge = Vertex1 - Vertex2;
		glm::vec3 EdgeMidPoint = (Vertex1 + Vertex2) / glm::vec3(2.0f);
		glm::vec3 EdgeNormal = glm::cross(LocalBoxNormals[0], Edge);
		EdgeNormal = glm::normalize(EdgeNormal);

		//LOG("Edge Normal: " << Vec3<float>(EdgeNormal));


		/*auto Difference = glm::vec3(transformedOrigin.x, transformedOrigin.y, transformedOrigin.z) - EdgeMidPoint;

		if (Difference.x < 0)
		{
			if (EdgeNormal.x != 0.0f)
			{
				EdgeNormal.x *= -1;
			}
		}
		if (Difference.z < 0)
		{
			if (EdgeNormal.z != 0.0f)
			{
				EdgeNormal.z *= -1;
			}
		}*/


		LocalBoxNormals.push_back(EdgeNormal);
	}

	auto MinMax = FindMinMax(BoxVertices, this->GetTransformation().GetModelMat4());

	Min = MinMax.first;
	Min = MinMax.second;

	UpdateChildren();
}

void FUSIONPHYSICS::CollisionBoxPlane::UpdateAttributes()
{
	LocalBoxNormals.clear();
	glm::vec4 transformedOrigin = this->GetTransformation().GetModelMat4() * glm::vec4(this->ModelOriginPoint, 1.0f);

	Faces[0].FindNormal(this->GetTransformation().GetModelMat4());
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
		auto Vertex1 = FUSIONOPENGL::TranslateVertex(this->GetTransformation().GetModelMat4(), BoxVertices[EdgeIndices[i]].Position);
		auto Vertex2 = FUSIONOPENGL::TranslateVertex(this->GetTransformation().GetModelMat4(), BoxVertices[EdgeIndices[i + 1]].Position);
		glm::vec3 Edge = Vertex1 - Vertex2;
		glm::vec3 EdgeMidPoint = (Vertex1 + Vertex2) / glm::vec3(2.0f);
		glm::vec3 EdgeNormal = glm::cross(LocalBoxNormals[0], Edge);
		EdgeNormal = glm::normalize(EdgeNormal);

		//LOG("Edge Normal: " << Vec3<float>(EdgeNormal));


		/*auto Difference = glm::vec3(transformedOrigin.x, transformedOrigin.y, transformedOrigin.z) - EdgeMidPoint;

		if (Difference.x < 0)
		{
			if (EdgeNormal.x != 0.0f)
			{
				EdgeNormal.x *= -1;
			}
		}
		if (Difference.z < 0)
		{
			if (EdgeNormal.z != 0.0f)
			{
				EdgeNormal.z *= -1;
			}
		}*/


		LocalBoxNormals.push_back(EdgeNormal);
	}

	auto MinMax = FindMinMax(BoxVertices, this->GetTransformation().GetModelMat4());

	Min = MinMax.first;
	Min = MinMax.second;
}
