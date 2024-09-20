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

class Node
{
public:
	glm::vec3 Min;
	glm::vec3 Max;
	glm::vec3 Origin;
	int ChildIndex;
	int Depth;

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

	Node() : Min(glm::vec3(std::numeric_limits<float>::max())),
		Max(glm::vec3(std::numeric_limits<float>::lowest())),
		ChildIndex(-1),
		Depth(0)
	{}

	virtual bool IsLeaf() const;
};

class BottomUpNode : public Node
{
public:
	FUSIONCORE::Model* model;

	BottomUpNode() : model(nullptr)
	{}

	bool IsLeaf() const override
	{
		return model != nullptr && ChildIndex == -1;
	}
};

class TopDownNode : public Node
{
public:
	int TriangleIndex;
	int TriangleCount;

	TopDownNode() : TriangleCount(0),
		     TriangleIndex(std::numeric_limits<int>::max())
	{}

	bool IsLeaf() const override
	{
		return TriangleCount > 0 && ChildIndex == -1;
	}
};

void SetTBObindlessTextureData(FUSIONCORE::TBO& tbo,FUSIONCORE::Texture2D &texture,GLenum internalUsage,GLsizeiptr size,const void* data,GLenum usage)
{
	tbo.Bind();
	tbo.BufferDataFill(size, data, usage);
	
	texture.Bind(GL_TEXTURE_BUFFER);
	tbo.TexBuffer(internalUsage);
	texture.MakeBindless();
    texture.MakeResident();

	glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

void CalculateModelBoundingBox(FUSIONCORE::Model*& Model, 
	                            glm::mat4& ModelMatrix,
	                            std::vector<std::pair<BottomUpNode,unsigned int>>& ModelBoundingBoxes,
								glm::vec3 &min,
								glm::vec3 &max,
	                            int Iterator)
{
	FUSIONCORE::WorldTransform& objectTransform = Model->GetTransformation();
	BottomUpNode newNode;

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
		ModelBoundingBoxes.push_back({ newNode , Iterator });
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


FUSIONCORE::WorldTransform NodeToWorldTransform(TopDownNode& Node)
{
	FUSIONCORE::WorldTransform newTranform;
	newTranform.InitialObjectScales = Node.Max - Node.Min;
	newTranform.OriginPoint = &Node.Origin;
	return newTranform;
}

void FUSIONCORE::PathTracer::VisualizeBVH(FUSIONCORE::Camera3D& Camera, FUSIONCORE::Shader& Shader, glm::vec3 NodeColor)
{
	for(auto& node : this->BVHnodes)
	{
		if (node.TriangleCount <= 0) continue;

		auto NodeTranform = NodeToWorldTransform(node);
		FUSIONPHYSICS::CollisionBoxAABB HeadNodeBox(NodeTranform, glm::vec3(1.0f));
		HeadNodeBox.SetMeshColor(FF_COLOR_RED);
		HeadNodeBox.DrawBoxMesh(Camera, Shader);
	}
}

void ConstructTopDownBVH(std::vector<TopDownNode> &BVHNodes,
                        std::vector<FUSIONCORE::Model*>& ModelsToTrace,
						std::vector<std::pair<BottomUpNode, unsigned int>>& ModelBoundingBoxes,
						AlignedBuffer<glm::vec4>& TrianglePositions,
						AlignedBuffer<glm::vec3> &TriangleCenters,
	                    const glm::vec3 &InitialMin,
	                    const glm::vec3 &InitialMax,
	                    int MaxDepth)
{

	std::deque<int> NodesToProcess;
	//NodesToProcess.insert(NodesToProcess.end(), BoundingBoxes.begin(), BoundingBoxes.end());
	auto& InitialNode = BVHNodes.emplace_back();
	InitialNode.Min = InitialMin;
	InitialNode.Max = InitialMax;
	InitialNode.TriangleCount = int(TrianglePositions.size() / 3);
	InitialNode.TriangleIndex = 0;
	InitialNode.Origin = (InitialNode.Max + InitialNode.Min) * 0.5f;
	NodesToProcess.push_back(0);

	while (!NodesToProcess.empty())
	{
		auto &node = BVHNodes[NodesToProcess.front()];
		NodesToProcess.pop_front();

		node.Depth++;
		if (node.Depth >= MaxDepth || node.TriangleCount <= 5)
		{
			continue;
		}

		glm::vec3 AxisSizes = node.Max - node.Min;
		glm::vec3 AxisCenters = (node.Max + node.Min) * 0.5f;
		int SplitAxis = AxisSizes.x > glm::max(AxisSizes.y, AxisSizes.z) ? 0 : AxisSizes.y > AxisSizes.z ? 1 : 2;

		float SplitPosition = AxisCenters[SplitAxis];

		TopDownNode Child0;
		TopDownNode Child1;
	
		Child0.Depth = node.Depth;
		Child1.Depth = node.Depth;
		Child0.TriangleIndex = node.TriangleIndex;
		Child1.TriangleIndex = node.TriangleIndex;

		for (int i = node.TriangleIndex; i < node.TriangleIndex + node.TriangleCount ; i++)
		{
			glm::vec3 TriangleCenter = TriangleCenters[i];

			bool IsChild0 = (SplitPosition > TriangleCenter[SplitAxis]);
			TopDownNode* ChosenChild = IsChild0 ? &Child0 : &Child1;
			
			ChosenChild->CompareVec3MinMax(TrianglePositions[i * 3]);
			ChosenChild->CompareVec3MinMax(TrianglePositions[i * 3 + 1]);
			ChosenChild->CompareVec3MinMax(TrianglePositions[i * 3 + 2]);
			ChosenChild->TriangleCount++;

			if (IsChild0)
			{
				int swap = ChosenChild->TriangleIndex + ChosenChild->TriangleCount - 1;
				std::swap(TrianglePositions[i * 3], TrianglePositions[swap * 3]);
				std::swap(TrianglePositions[i * 3 + 1], TrianglePositions[swap * 3 + 1]);
				std::swap(TrianglePositions[i * 3 + 2], TrianglePositions[swap * 3 + 2]);
				std::swap(TriangleCenters[i], TriangleCenters[swap]);
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

	AlignedBuffer<glm::mat4> ModelMatrixes;
	AlignedBuffer<glm::vec4> TrianglePositions;
	AlignedBuffer<glm::vec3> TriangleCenters;

	std::vector<std::pair<BottomUpNode, unsigned int>> BoundingBoxes;

	size_t ModelCount = ModelsToTrace.size();

	ModelMatrixes.reserve(ModelCount);
	TriangleCenters.reserve(ModelCount);
	glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());
	
	glm::mat4 ModelMatrix(1.0f);
    
	FUSIONUTIL::Timer timer;
	timer.Set();
	
    //#pragma omp parallel for 
	for (int i = 0; i < ModelCount; i++)
	{
		glm::vec3 TriangleCenter = glm::vec3(0.0f);
		glm::vec3 TempPosition;

		auto& model = ModelsToTrace[i];
		glm::mat4 ModelMatrix = model->GetTransformation().GetModelMat4();

        //#pragma omp critical
		{
			ModelMatrixes.push_back(ModelMatrix);  
		}

		size_t TriangleCount = 0;

		for (auto& mesh : model->Meshes)
		{
			auto& faces = mesh.GetFaces();
			auto& vertices = mesh.GetVertices();

            //#pragma omp critical
			{
				TrianglePositions.reserve((faces.size() * 3) + TrianglePositions.size());
			}

			for (auto& face : faces)
			{
				auto& indices = face->Indices;
				if (indices.size() < 3) {
					continue;
				}

				TempPosition = glm::vec3(ModelMatrix * glm::vec4(vertices[indices[0]]->Position, 1.0f));
				TriangleCenter += TempPosition;
                //#pragma omp critical
				{
					TrianglePositions.push_back(glm::vec4(TempPosition, i));
				}

				TempPosition = glm::vec3(ModelMatrix * glm::vec4(vertices[indices[1]]->Position, 1.0f));
				TriangleCenter += TempPosition;
               // #pragma omp critical
				{
					TrianglePositions.push_back(glm::vec4(TempPosition, i));
				}

				TempPosition = glm::vec3(ModelMatrix * glm::vec4(vertices[indices[2]]->Position, 1.0f));
				TriangleCenter += TempPosition;
               // #pragma omp critical
				{
					TrianglePositions.push_back(glm::vec4(TempPosition, i));
				}

				TriangleCenter /= 3;

               // #pragma omp critical
				{
					TriangleCenters.push_back(TriangleCenter);
				}

				TriangleCenter = glm::vec3(0.0f);
			}
		}
        //#pragma omp critical
		{
			CalculateModelBoundingBox(model, ModelMatrix, BoundingBoxes, min, max, i);
		}
	}

	ConstructTopDownBVH(BVHnodes, ModelsToTrace, BoundingBoxes, TrianglePositions, TriangleCenters, min, max, 17);

	NodeCount = BVHnodes.size();
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
		auto& node = BVHnodes[i];

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

	
	TracerTriangleDataBuffer.Bind();
	TracerTriangleDataBuffer.BufferDataFill(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * TrianglePositions.size(), TrianglePositions.data(), GL_STREAM_DRAW);
	TracerTriangleDataBuffer.BindSSBO(7);
	TracerTriangleDataBuffer.Unbind();

	TriangleCount = TrianglePositions.size();
	IsInitialized = false;

	LOG("Process took: " << timer.GetMiliseconds());
	/*shader.use();
	shader.setInt("TriangleCount", TriangleCount);

	GLuint workGroupSizeX = 32;
	GLuint workGroupSizeY = 32;

	GLuint numGroupsX = (TrianglePositions.size() + workGroupSizeX - 1) / workGroupSizeX;

	glDispatchCompute(numGroupsX,1,1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);*/
}

FUSIONCORE::PathTracer::~PathTracer()
{
	glDeleteTextures(1, &this->image);
	this->ChildIndexTexture.Clear();
	this->MaxBoundTexture.Clear();
	this->MinBoundTexture.Clear();
	this->TriangleCountTexture.Clear();
	this->TriangleIndexTexture.Clear();
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

	if (!IsInitialized)
	{
		this->MinBoundTexture.SendBindlessHandle(shader.GetID(), "MinBounds");
		this->MaxBoundTexture.SendBindlessHandle(shader.GetID(), "MaxBounds");
		this->ChildIndexTexture.SendBindlessHandle(shader.GetID(), "ChildIndicies");
		this->TriangleIndexTexture.SendBindlessHandle(shader.GetID(), "TriangleIndicies");
		this->TriangleCountTexture.SendBindlessHandle(shader.GetID(), "TriangleCounts");
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


