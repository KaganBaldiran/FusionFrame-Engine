#define _GNU_SOURCE
#include "PathTracer.hpp"
#include <glew.h>
#include <glfw3.h>
#include "../../FusionUtility/Log.h"
#include <vector>
#include <cstdio>
#include <cstdlib>
#include "../../FusionUtility/Hashing.hpp"
#include <omp.h>
#include "../../FusionUtility/StopWatch.h"
#include <algorithm>
#include "../../FusionCore/Color.hpp"

template <typename T>
using AlignedBuffer = std::vector<T, FUSIONUTIL::AlignedAllocator<T,16>>;

template<typename T>
void CompareVec3MinMax(T&& Min, T&& Max,T&& v)
{
	Min.x = glm::min(v.x, Min.x);
	Min.y = glm::min(v.y, Min.y);
	Min.z = glm::min(v.z, Min.z);

	Max.x = glm::max(v.x, Max.x);
	Max.y = glm::max(v.y, Max.y);
	Max.z = glm::max(v.z, Max.z);
}

class BVHnode
{
public:
	glm::vec3 Min;
	glm::vec3 Max;
	glm::vec3 Origin;
	int ChildIndex;
	int Depth;
	int TriangleIndex;
	int TriangleCount;

	template<typename T>
	void CompareVec3MinMax(T&& v)
	{
		Min.x = glm::min(v.x, Min.x);
		Min.y = glm::min(v.y, Min.y);
		Min.z = glm::min(v.z, Min.z);

		Max.x = glm::max(v.x, Max.x);
		Max.y = glm::max(v.y, Max.y);
		Max.z = glm::max(v.z, Max.z);
	}

	template<typename T>
	void CompareVec3MinMax(T&& min, T&& max)
	{
		Min.x = glm::min(min.x, Min.x);
		Min.y = glm::min(min.y, Min.y);
		Min.z = glm::min(min.z, Min.z);

		Max.x = glm::max(max.x, Max.x);
		Max.y = glm::max(max.y, Max.y);
		Max.z = glm::max(max.z, Max.z);
	}

	BVHnode() : Min(glm::vec3(std::numeric_limits<float>::max())),
		Max(glm::vec3(std::numeric_limits<float>::lowest())),
		ChildIndex(-1),
		Depth(0),
		TriangleCount(0),
		TriangleIndex(std::numeric_limits<int>::max())
	{}

	bool IsLeaf()
	{
		return TriangleCount > 0 && ChildIndex == -1;
	}
};


inline void SetTBObindlessTextureData(FUSIONCORE::TBO& tbo,FUSIONCORE::Texture2D &texture,GLenum internalUsage,GLsizeiptr size,const void* data,GLenum usage)
{
	tbo.Bind();
	tbo.BufferDataFill(size, data, usage);
	
	texture.Bind(GL_TEXTURE_BUFFER);
	tbo.TexBuffer(internalUsage);
	texture.MakeBindless();
    texture.MakeResident();

	glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

inline void CalculateModelBoundingBox(FUSIONCORE::Model*& Model,
	                            glm::mat4& ModelMatrix,
	                            std::vector<BVHnode>& ModelBoundingBoxes,
								glm::vec3 &min,
								glm::vec3 &max)
{
	FUSIONCORE::WorldTransform& objectTransform = Model->GetTransformation();
	BVHnode newNode;

	//if (BoundingBoxes.size() <= i + 1 || !BoundingBoxes[i].Object->IsSameObject(BoundingBox.Object) || objectTransform.IsTransformed)
	//if (objectTransform.IsTransformed)
	{
		auto& ModelScales = objectTransform.InitialObjectScales;
		auto& ModelPosition = *objectTransform.OriginPoint;
		auto HalfScales = (ModelScales * 0.5f);

		for (size_t x = 0; x < 2; ++x)
		{
			for (size_t y = 0; y < 2; ++y)
			{
				for (size_t z = 0; z < 2; ++z)
				{
					glm::vec3 Vertex(ModelPosition.x + ((2.0f * x - 1.0f) * HalfScales.x),
						ModelPosition.y + ((2.0f * y - 1.0f) * HalfScales.y),
						ModelPosition.z + ((2.0f * z - 1.0f) * HalfScales.z));

					Vertex = glm::vec3(ModelMatrix * glm::vec4(Vertex, 1.0f));
					newNode.CompareVec3MinMax(Vertex);
				}
			}
		}

		newNode.Origin = (newNode.Min + newNode.Max) * 0.5f;
		ModelBoundingBoxes.push_back(newNode);
		//}
		min.x = glm::min(min.x, newNode.Min.x);
		min.y = glm::min(min.y, newNode.Min.y);
		min.z = glm::min(min.z, newNode.Min.z);

		max.x = glm::max(max.x, newNode.Max.x);
		max.y = glm::max(max.y, newNode.Max.y);
		max.z = glm::max(max.z, newNode.Max.z);

		objectTransform.IsTransformed = false;
	}
}

template<typename VEC3>
inline float ComputeBoundingBoxArea(VEC3&& Min, VEC3&& Max)
{
	glm::vec3 Dimension = Max - Min;
	return 2.0f * (Dimension.x * Dimension.y + Dimension.x * Dimension.z + Dimension.z * Dimension.y);
}

template<typename FLOAT,typename INT>
inline float ComputeSAHcost(FLOAT&& LeftNodeArea, FLOAT&& RightNodeArea, FLOAT&& ParentNodeArea,
	                 INT&& NumLeft, INT&& NumRight, FLOAT&& TraversalCost = 1.0f, FLOAT&& IntersectionCost = 4.0f)
{
	return TraversalCost + (LeftNodeArea / ParentNodeArea) * NumLeft * IntersectionCost + (RightNodeArea / ParentNodeArea) * NumRight * IntersectionCost;
}

template<typename INT, typename FLOAT>
inline float EstimateSplitTopDown(BVHnode& Node, INT&& Axis, FLOAT&& SplitPosition,
					AlignedBuffer<glm::vec3>& TriangleCenters,
					std::vector<std::pair<glm::vec3, glm::vec3>>& TriangleMinMax)
{
	BVHnode Child0;
	BVHnode Child1;
	int NumberIn0 = 0;
	int NumberIn1 = 0;

	for (int i = Node.TriangleIndex; i < Node.TriangleIndex + Node.TriangleCount; i++)
	{
		auto& MinMax = TriangleMinMax[i];
		if (TriangleCenters[i][Axis] < SplitPosition)
		{
			Child0.CompareVec3MinMax(MinMax.first, MinMax.second);
			NumberIn0++;
		}
		else
		{
			Child1.CompareVec3MinMax(MinMax.first, MinMax.second);
			NumberIn1++;
		}
	}

	return ComputeSAHcost(ComputeBoundingBoxArea(Child0.Min, Child0.Max), ComputeBoundingBoxArea(Child1.Min, Child1.Max),
		                  ComputeBoundingBoxArea(Node.Min, Node.Max), NumberIn0, NumberIn1);
}

template<typename INT,typename FLOAT>
inline float EstimateSplitBottomUp(BVHnode& Node, INT&& Axis, FLOAT&& SplitPosition,
	                               std::vector<BVHnode> &BoundingBoxes)
{
	BVHnode Child0;
	BVHnode Child1;
	int NumberIn0 = 0;
	int NumberIn1 = 0;

	for (int i = Node.TriangleIndex; i < Node.TriangleIndex + Node.TriangleCount; i++)
	{
		auto& box = BoundingBoxes[i];
		if (box.Origin[Axis] < SplitPosition)
		{
			Child0.CompareVec3MinMax(box.Min, box.Max);
			NumberIn0++;
		}
		else
		{
			Child1.CompareVec3MinMax(box.Min, box.Max);
			NumberIn1++;
		}
	}

	return ComputeSAHcost(ComputeBoundingBoxArea(Child0.Min, Child0.Max), ComputeBoundingBoxArea(Child1.Min, Child1.Max),
		ComputeBoundingBoxArea(Node.Min, Node.Max), NumberIn0, NumberIn1);
}

inline void SplitTopDown(int &BestAxis,float& BestPos,float& BestCost,BVHnode& Node,
	              AlignedBuffer<glm::vec3>& TriangleCenters,
	              std::vector<std::pair<glm::vec3, glm::vec3>>& TriangleMinMax)
{
	constexpr int TestCountPerAXis = 6;
	
	float BoundMin = 0;
	float BoundMax = 0;
	float SplitFract = 0.0f;
	float Position = 0.0f;
	float Cost = 0.0f;

	for (size_t i = 0; i < 3; i++)
	{
		BoundMin = Node.Min[i];
		BoundMax = Node.Max[i];
		for (size_t y = 0; y < TestCountPerAXis; y++)
		{
			SplitFract = static_cast<float>(i + 1) / static_cast<float>(TestCountPerAXis + 1);

			Position = BoundMin + (BoundMax - BoundMin) * SplitFract;
			Cost = EstimateSplitTopDown(Node, i, Position, TriangleCenters, TriangleMinMax);

			if (Cost <= BestCost)
			{
				BestCost = Cost;
				BestPos = Position;
				BestAxis = i;
			}
		}
	}
}

inline void SplitBottomUp(int& BestAxis, float& BestPos, float& BestCost, BVHnode& Node,
	                      std::vector<BVHnode>& BoundingBoxes)
{
	constexpr int TestCountPerAXis = 6;

	float BoundMin = 0;
	float BoundMax = 0;
	float SplitFract = 0.0f;
	float Position = 0.0f;
	float Cost = 0.0f;

	for (size_t i = 0; i < 3; i++)
	{
		BoundMin = Node.Min[i];
		BoundMax = Node.Max[i];
		for (size_t y = 0; y < TestCountPerAXis; y++)
		{
			SplitFract = static_cast<float>(i + 1) / static_cast<float>(TestCountPerAXis + 1);

			Position = BoundMin + (BoundMax - BoundMin) * SplitFract;
			Cost = EstimateSplitBottomUp(Node, i, Position, BoundingBoxes);

			if (Cost <= BestCost)
			{
				BestCost = Cost;
				BestPos = Position;
				BestAxis = i;
			}
		}
	}
}

inline void ConstructTopDownBVH(std::vector<BVHnode> &BVHNodes,
						int ModelBoundingBoxIndex,
						AlignedBuffer<glm::vec4>& TrianglePositions,
						AlignedBuffer<glm::vec3> &TriangleCenters,
						std::vector<std::pair<glm::vec3, glm::vec3>> &TriangleMinMax,
	                    int MaxDepth)
{
	std::deque<int> NodesToProcess;
	NodesToProcess.push_back(ModelBoundingBoxIndex);

	while (!NodesToProcess.empty())
	{
		auto &node = BVHNodes[NodesToProcess.front()];
		NodesToProcess.pop_front();

		node.Depth++;
		if (node.Depth >= MaxDepth || node.TriangleCount <= 3)
		{
			continue;
		}
		
		float SplitPosition = 0;
		int SplitAxis = 0;
		float Cost = std::numeric_limits<float>::max();

		SplitTopDown(SplitAxis, SplitPosition, Cost, node, TriangleCenters, TriangleMinMax);

		BVHnode Child0;
		BVHnode Child1;
	
		Child0.Depth = node.Depth;
		Child1.Depth = node.Depth;
		Child0.TriangleIndex = node.TriangleIndex;
		Child1.TriangleIndex = node.TriangleIndex;

		for (int i = node.TriangleIndex; i < node.TriangleIndex + node.TriangleCount ; i++)
		{
			glm::vec3 TriangleCenter = TriangleCenters[i];

			bool IsChild0 = (SplitPosition > TriangleCenter[SplitAxis]);
			BVHnode* ChosenChild = IsChild0 ? &Child0 : &Child1;
			
			auto& PreCalculatedMinMax = TriangleMinMax[i];
			ChosenChild->CompareVec3MinMax(PreCalculatedMinMax.first, PreCalculatedMinMax.second);
			ChosenChild->TriangleCount++;

			if (IsChild0)
			{
				int swap = ChosenChild->TriangleIndex + ChosenChild->TriangleCount - 1;
				std::swap(TrianglePositions[i * 3], TrianglePositions[swap * 3]);
				std::swap(TrianglePositions[i * 3 + 1], TrianglePositions[swap * 3 + 1]);
				std::swap(TrianglePositions[i * 3 + 2], TrianglePositions[swap * 3 + 2]);
				std::swap(TriangleCenters[i], TriangleCenters[swap]);
				std::swap(TriangleMinMax[i], TriangleMinMax[swap]);
				Child1.TriangleIndex++;
			}
		}

		node.TriangleIndex = -1;
		node.TriangleCount = 0;

		node.ChildIndex = BVHNodes.size();

		Child0.Origin = (Child0.Max + Child0.Min) * 0.5f;
		BVHNodes.push_back(std::move(Child0));
		NodesToProcess.push_back(BVHNodes.size() - 1);
		
		Child1.Origin = (Child1.Max + Child1.Min) * 0.5f;
		BVHNodes.push_back(std::move(Child1));
		NodesToProcess.push_back(BVHNodes.size() - 1);
	}
}

inline void ConstructBottomUpBVH(std::vector<BVHnode>& BVHNodes,
								std::vector<BVHnode>& BoundingBoxes,
								const glm::vec3& InitialMin,
								const glm::vec3& InitialMax)
{
	std::deque<int> NodesToProcess;
	auto& InitialNode = BVHNodes.emplace_back();
	InitialNode.Min = InitialMin;
	InitialNode.Max = InitialMax;
	InitialNode.TriangleCount = int(BoundingBoxes.size());
	InitialNode.TriangleIndex = 0;
	InitialNode.Origin = (InitialNode.Max + InitialNode.Min) * 0.5f;
	NodesToProcess.push_back(0);

	while (!NodesToProcess.empty())
	{
		auto& node = BVHNodes[NodesToProcess.front()];
		NodesToProcess.pop_front();

		if (node.TriangleCount <= 1)
		{
			auto& child = BoundingBoxes[node.TriangleIndex];

			node.TriangleCount = child.TriangleCount;
			node.TriangleIndex = child.TriangleIndex;
			continue;
		}

		float SplitPosition = 0;
		int SplitAxis = 0;
		float Cost = std::numeric_limits<float>::max();

		SplitBottomUp(SplitAxis, SplitPosition, Cost, node, BoundingBoxes);

		BVHnode Child0;
		BVHnode Child1;

		Child0.TriangleIndex = node.TriangleIndex;
		Child1.TriangleIndex = node.TriangleIndex;

		for (int i = node.TriangleIndex; i < node.TriangleIndex + node.TriangleCount; i++)
		{
			auto& box = BoundingBoxes[i];

			bool IsChild0 = (SplitPosition > box.Origin[SplitAxis]);
			BVHnode* ChosenChild = IsChild0 ? &Child0 : &Child1;

			ChosenChild->CompareVec3MinMax(box.Min, box.Max);
			ChosenChild->TriangleCount++;

			if (IsChild0)
			{
				int swap = ChosenChild->TriangleIndex + ChosenChild->TriangleCount - 1;
				std::swap(BoundingBoxes[i], BoundingBoxes[swap]);
				Child1.TriangleIndex++;
			}
		}

		node.TriangleIndex = -1;
		node.TriangleCount = 0;

		node.ChildIndex = BVHNodes.size();

		Child0.Origin = (Child0.Max + Child0.Min) * 0.5f;
		BVHNodes.push_back(std::move(Child0));
		NodesToProcess.push_back(BVHNodes.size() - 1);

		Child1.Origin = (Child1.Max + Child1.Min) * 0.5f;
		BVHNodes.push_back(std::move(Child1));
		NodesToProcess.push_back(BVHNodes.size() - 1);
	}
}

FUSIONCORE::PathTracer::PathTracer(unsigned int width,unsigned int height,std::vector<Model*>& ModelsToTrace,Shader& shader)
{
	glGenTextures(1, &image);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, image, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	if (ModelsToTrace.empty())
	{
		return;
	}
	FUSIONUTIL::Timer timer;
	timer.Set();

	AlignedBuffer<glm::mat4> ModelMatrixes;
	AlignedBuffer<glm::vec4> TrianglePositions;
	std::vector<glm::vec3> TriangleNormals;
	AlignedBuffer<glm::vec3> TriangleCenters;

	std::vector<std::pair<glm::vec3, glm::vec3>> TriangleMinMax;
	std::vector<BVHnode> BoundingBoxes;

	size_t ModelCount = ModelsToTrace.size();
	ModelMatrixes.reserve(ModelCount);
	
	glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());

	glm::vec3 TriangleMin = min;
	glm::vec3 TriangleMax = max;

	glm::vec3 TriangleCenter = glm::vec3(0.0f);
	glm::mat4 ModelMatrix(1.0f);
	glm::mat3 NormalMatrix(1.0f);
	glm::vec3 TempPosition;
	float inv_three = 1.0f / 3.0f;
	int CurrentModelTriangleCount = 0;

	int ReserveCount = 0;
	for (auto& model : ModelsToTrace)
	{
		for (auto& mesh : model->Meshes)
		{
			ReserveCount += mesh.GetFaces().size();
		}
	}

	TrianglePositions.reserve(ReserveCount * 3);
	TriangleNormals.reserve(ReserveCount * 3);
	TriangleCenters.reserve(ReserveCount);
	TriangleMinMax.reserve(ReserveCount);
	
	for (int i = 0; i < ModelCount; i++)
	{
		auto& model = ModelsToTrace[i];
		ModelMatrix = model->GetTransformation().GetModelMat4();
		NormalMatrix = glm::transpose(glm::inverse(glm::mat3(ModelMatrix)));

		ModelMatrixes.push_back(ModelMatrix);
		
		for (auto& mesh : model->Meshes)
		{
			auto& faces = mesh.GetFaces();
			auto& vertices = mesh.GetVertices();
			CurrentModelTriangleCount += faces.size();
			
			for (auto& face : faces)
			{
				auto& indices = face->Indices;
				if (indices.size() < 3) {
					continue;
				}

				auto& Vertex0 = vertices[indices[0]];

				TempPosition = glm::vec3(ModelMatrix * glm::vec4(Vertex0->Position, 1.0f));
				CompareVec3MinMax(TriangleMin, TriangleMax, TempPosition);
				TriangleCenter += TempPosition;
				
				TriangleNormals.push_back(NormalMatrix * Vertex0->Normal);
				TrianglePositions.push_back(glm::vec4(TempPosition, i));
				
				auto& Vertex1 = vertices[indices[1]];

				TempPosition = glm::vec3(ModelMatrix * glm::vec4(Vertex1->Position, 1.0f));
				CompareVec3MinMax(TriangleMin, TriangleMax, TempPosition);
				TriangleCenter += TempPosition;
				
				TriangleNormals.push_back(NormalMatrix * Vertex1->Normal);
				TrianglePositions.push_back(glm::vec4(TempPosition, i));
				
				auto& Vertex2 = vertices[indices[2]];

				TempPosition = glm::vec3(ModelMatrix * glm::vec4(Vertex2->Position, 1.0f));
				CompareVec3MinMax(TriangleMin, TriangleMax, TempPosition);
				TriangleCenter += TempPosition;
				
				TriangleNormals.push_back(NormalMatrix * Vertex2->Normal);
				TrianglePositions.push_back(glm::vec4(TempPosition, i));
				
				TriangleCenter *= inv_three;

				TriangleCenters.push_back(TriangleCenter);
				
				TriangleMinMax.push_back({ TriangleMin,TriangleMax });

				TriangleMin = glm::vec3(std::numeric_limits<float>::max());
				TriangleMax = glm::vec3(std::numeric_limits<float>::lowest());
				TriangleCenter = glm::vec3(0.0f);
			}
		}
		
		CalculateModelBoundingBox(model, ModelMatrix, BoundingBoxes, min, max);
		
		auto& CurrentBoundingBox = BoundingBoxes.back();
		CurrentBoundingBox.TriangleCount = CurrentModelTriangleCount;
		CurrentBoundingBox.TriangleIndex = TriangleCenters.size() - CurrentModelTriangleCount;
		CurrentModelTriangleCount = 0;
	}

	ConstructBottomUpBVH(BottomUpBVHNodes, BoundingBoxes, min, max);

	TopDownBVHnodes.insert(TopDownBVHnodes.end(), BottomUpBVHNodes.begin(), BottomUpBVHNodes.end());
	for (int i = 0; i < BottomUpBVHNodes.size(); i++)
	{
		auto& node = TopDownBVHnodes[i];
		if (node.ChildIndex < 0)
		{
		   ConstructTopDownBVH(TopDownBVHnodes, i, TrianglePositions, TriangleCenters, TriangleMinMax, 27);
		}
	}
	ModelNodeCount = BoundingBoxes.size();
	
	NodeCount = TopDownBVHnodes.size();
	std::vector<glm::vec3> MinBounds;
	std::vector<glm::vec3> MaxBounds;
	std::vector<float> ChildIndicies;
	std::vector<float> TriangleIndicies;
	std::vector<float> TriangleCounts;

	size_t sizeOfMinBounds = NodeCount * sizeof(glm::vec3);
	size_t sizeOfMaxBounds = NodeCount * sizeof(glm::vec3);
	size_t sizeOfChildIndices = NodeCount * sizeof(float);
	size_t sizeOfTriangleIndices = NodeCount * sizeof(float);
	size_t sizeOfTriangleCounts = NodeCount * sizeof(float);

	MinBounds.reserve(NodeCount);
	MaxBounds.reserve(NodeCount);
	ChildIndicies.reserve(NodeCount);
	TriangleIndicies.reserve(NodeCount);
	TriangleCounts.reserve(NodeCount);
	for (int i = 0; i < NodeCount; i++) {
		auto& node = TopDownBVHnodes[i];

		MinBounds.push_back(node.Min);
		MaxBounds.push_back(node.Max);
		ChildIndicies.push_back(node.ChildIndex);
		TriangleIndicies.push_back(node.TriangleIndex);
		TriangleCounts.push_back(node.TriangleCount);
	}

	SetTBObindlessTextureData(MinBoundData, 
		MinBoundTexture, 
		GL_RGB32F,
		sizeOfMinBounds, 
		MinBounds.data(), 
		GL_STATIC_DRAW);

	SetTBObindlessTextureData(MaxBoundData,
		MaxBoundTexture,
		GL_RGB32F,
		sizeOfMaxBounds,
		MaxBounds.data(),
		GL_STATIC_DRAW);

	SetTBObindlessTextureData(ChildIndexData,
		ChildIndexTexture,
		GL_R32F,
		sizeOfChildIndices,
		ChildIndicies.data(),
		GL_STATIC_DRAW);

	SetTBObindlessTextureData(TriangleCountData,
		TriangleCountTexture,
		GL_R32F,
		sizeOfTriangleCounts,
		TriangleCounts.data(),
		GL_STATIC_DRAW);

	SetTBObindlessTextureData(TriangleIndexData,
		TriangleIndexTexture,
		GL_R32F,
		sizeOfTriangleIndices,
		TriangleIndicies.data(),
		GL_STATIC_DRAW);


	SetTBObindlessTextureData(TriangleNormalsData,
		TriangleNormalsTexture,
		GL_RGB32F,
		TriangleNormals.size() * sizeof(glm::vec3),
		TriangleNormals.data(),
		GL_STATIC_DRAW);

	TracerTriangleDataBuffer.Bind();
	TracerTriangleDataBuffer.BufferDataFill(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * TrianglePositions.size(), TrianglePositions.data(), GL_STREAM_DRAW);
	TracerTriangleDataBuffer.BindSSBO(7);
	TracerTriangleDataBuffer.Unbind();

	TriangleCount = TrianglePositions.size();
	IsInitialized = false;

	LOG("Process took: " << timer.GetMiliseconds());
}

FUSIONCORE::PathTracer::~PathTracer()
{
	glDeleteTextures(1, &this->image);
	this->ChildIndexTexture.Clear();
	this->MaxBoundTexture.Clear();
	this->MinBoundTexture.Clear();
	this->TriangleCountTexture.Clear();
	this->TriangleIndexTexture.Clear();
	this->TriangleNormalsTexture.Clear();
}

void FUSIONCORE::PathTracer::Render(glm::vec2 WindowSize,Shader& shader,Camera3D& camera)
{
	shader.use();
	shader.setVec2("WindowSize", WindowSize);
	shader.setVec3("CameraPosition", camera.Position);
	shader.setMat4("ProjectionViewMat",camera.ProjectionViewMat);
	shader.setFloat("Time",glfwGetTime());
	shader.setInt("TriangleCount", TriangleCount);
	shader.setInt("NodeCount", NodeCount);
	shader.setInt("ModelNodeCount", ModelNodeCount);

	if (!IsInitialized)
	{
		this->MinBoundTexture.SendBindlessHandle(shader.GetID(), "MinBounds");
		this->MaxBoundTexture.SendBindlessHandle(shader.GetID(), "MaxBounds");
		this->ChildIndexTexture.SendBindlessHandle(shader.GetID(), "ChildIndicies");
		this->TriangleIndexTexture.SendBindlessHandle(shader.GetID(), "TriangleIndicies");
		this->TriangleCountTexture.SendBindlessHandle(shader.GetID(), "TriangleCounts");
		this->TriangleNormalsTexture.SendBindlessHandle(shader.GetID(), "TriangleNormals");
		IsInitialized = true;
	}

	GLuint workGroupSizeX = 32;
	GLuint workGroupSizeY = 32;

	GLuint numGroupsX = (WindowSize.x + workGroupSizeX - 1) / workGroupSizeX;
	GLuint numGroupsY = (WindowSize.y  + workGroupSizeY - 1) / workGroupSizeY;

	glDispatchCompute(numGroupsX, numGroupsY,1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
	UseShaderProgram(0);
}

inline FUSIONCORE::WorldTransform NodeToWorldTransform(BVHnode& Node)
{
	FUSIONCORE::WorldTransform newTranform;
	newTranform.InitialObjectScales = Node.Max - Node.Min;
	newTranform.OriginPoint = &Node.Origin;
	return newTranform;
}

void FUSIONCORE::PathTracer::VisualizeBVH(FUSIONCORE::Camera3D& Camera, FUSIONCORE::Shader& Shader, glm::vec3 NodeColor)
{
	for(auto& node : this->TopDownBVHnodes)
	{
		//if (node.TriangleCount <= 0) continue;

		auto NodeTranform = NodeToWorldTransform(node);
		FUSIONPHYSICS::CollisionBoxAABB HeadNodeBox(NodeTranform, glm::vec3(1.0f));
		HeadNodeBox.SetMeshColor(FF_COLOR_RED);
		HeadNodeBox.DrawBoxMesh(Camera, Shader);
	}
}

