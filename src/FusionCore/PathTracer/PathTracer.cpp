#include "PathTracer.hpp"
#include <glew.h>
#include <glfw3.h>
#include "../../FusionUtility/Log.h"
#include <vector>
#include <cstdio>
#include <cstdlib>
#include "../../FusionUtility/StopWatch.h"
#include <algorithm>
#include "../../FusionCore/Color.hpp"
#include "../../FusionCore/Light.hpp"
#include "../../FusionCore/Decal.hpp"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "imgui_internal.h"
#include <omp.h>

template <typename T>
using AlignedBuffer = std::vector<T, FUSIONUTIL::AlignedAllocator<T, 16>>;

inline void CompareVec3MinMax(glm::vec3& Min,glm::vec3& Max, const glm::vec3& v)
{
	Min.x = glm::min(v.x, Min.x);
	Min.y = glm::min(v.y, Min.y);
	Min.z = glm::min(v.z, Min.z);

	Max.x = glm::max(v.x, Max.x);
	Max.y = glm::max(v.y, Max.y);
	Max.z = glm::max(v.z, Max.z);
}

template<typename T>
inline void SwapThreeItems(T& dest,const int& i,const int& swap)
{
	std::swap(dest[i * 3], dest[swap * 3]);
	std::swap(dest[i * 3 + 1], dest[swap * 3 + 1]);
	std::swap(dest[i * 3 + 2], dest[swap * 3 + 2]);
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

	void CompareVec3MinMax(const glm::vec3& v)
	{
		Min.x = glm::min(v.x, Min.x);
		Min.y = glm::min(v.y, Min.y);
		Min.z = glm::min(v.z, Min.z);

		Max.x = glm::max(v.x, Max.x);
		Max.y = glm::max(v.y, Max.y);
		Max.z = glm::max(v.z, Max.z);
	}

	void CompareVec3MinMax(const glm::vec3& min, const glm::vec3& max)
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

inline float ComputeBoundingBoxArea(const glm::vec3& Min, const glm::vec3& Max)
{
	glm::vec3 Dimension = Max - Min;
	return 2.0f * (Dimension.x * Dimension.y + Dimension.x * Dimension.z + Dimension.z * Dimension.y);
}

inline float ComputeSAHcost(const float& LeftNodeArea, const float& RightNodeArea, const float& ParentNodeArea,
	const int& NumLeft, const int& NumRight,
	const float& TraversalCost = 1.0f, const float& IntersectionCost = 4.0f)
{
	return TraversalCost + (LeftNodeArea / ParentNodeArea) * NumLeft * IntersectionCost +
		(RightNodeArea / ParentNodeArea) * NumRight * IntersectionCost;
}

inline float EstimateSplitTopDown(BVHnode& Node, const int& Axis, const float& SplitPosition,
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


inline float EstimateSplitBottomUp(BVHnode& Node, const int& Axis, const float& SplitPosition,
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
	constexpr int TestCountPerAXis = 25;
	
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
			SplitFract = static_cast<float>(y + 1) / static_cast<float>(TestCountPerAXis + 1);

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
	constexpr int TestCountPerAXis = 15;

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
			SplitFract = static_cast<float>(y + 1) / static_cast<float>(TestCountPerAXis + 1);

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
						AlignedBuffer<glm::vec3>& TriangleNormals,
						AlignedBuffer<glm::vec2>& TriangleUVs,
						AlignedBuffer<glm::vec3> &TriangleCenters,
						AlignedBuffer<glm::vec3> &TriangleTangentsBitangents,
						std::map<unsigned int, unsigned int> &EmissiveObjectIndices,
						std::vector<std::pair<glm::vec3, glm::vec3>> &TriangleMinMax,
	                    int MaxDepth)
{
	std::deque<int> NodesToProcess;
	NodesToProcess.push_back(ModelBoundingBoxIndex);

	float ParentCost = std::numeric_limits<int>::max();

	while (!NodesToProcess.empty())
	{
		auto &node = BVHNodes[NodesToProcess.back()];
		NodesToProcess.pop_back();

		node.Depth++;
		if (node.Depth >= MaxDepth || node.TriangleCount <= 1)
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

				SwapThreeItems(TrianglePositions, i, swap);
				SwapThreeItems(TriangleNormals, i, swap);
				SwapThreeItems(TriangleUVs, i, swap);
				SwapThreeItems(TriangleTangentsBitangents, i * 2, swap * 2);
				SwapThreeItems(TriangleTangentsBitangents, i * 2 + 1, swap * 2 + 1);

				std::swap(TriangleCenters[i], TriangleCenters[swap]);
				std::swap(TriangleMinMax[i], TriangleMinMax[swap]);

				auto Indexi = EmissiveObjectIndices.find(i) != EmissiveObjectIndices.end();
				auto IndexSwap = EmissiveObjectIndices.find(swap) != EmissiveObjectIndices.end();
				if (Indexi || IndexSwap)
				{
					//LOG(swap << " " << i);
					if (Indexi && IndexSwap)
					{
						
					}
					else if (Indexi)
					{
						EmissiveObjectIndices[swap] = swap;
						EmissiveObjectIndices.erase(i);
					}
					else if (IndexSwap)
					{
						EmissiveObjectIndices[i] = i;
						EmissiveObjectIndices.erase(swap);
					}
					
				}
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

		ParentCost = Cost;
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

bool ProcessMaterial(FUSIONCORE::Material*& material,
					AlignedBuffer<GLuint64> &TextureHandles,
					AlignedBuffer<glm::vec4> &ModelAlbedos,
					AlignedBuffer<float> &ModelRoughness,
					AlignedBuffer<float> &ModelMetallic,
					AlignedBuffer<float> &ModelAlphas,
					AlignedBuffer<glm::vec4>& ModelEmissives,
	                AlignedBuffer<glm::vec2>& ModelClearCoats)
{
	auto AlbedoTexture = material->GetTextureMap(FF_TEXTURE_DIFFUSE);
	auto NormalTexture = material->GetTextureMap(FF_TEXTURE_NORMAL);
	auto RoughnessTexture = material->GetTextureMap(FF_TEXTURE_SPECULAR);
	auto MetallicTexture = material->GetTextureMap(FF_TEXTURE_METALLIC);
	auto AlphaTexture = material->GetTextureMap(FF_TEXTURE_ALPHA);
	auto EmissiveTexture = material->GetTextureMap(FF_TEXTURE_EMISSIVE);
	TextureHandles.push_back(AlbedoTexture != nullptr ? AlbedoTexture->GetTextureHandle() : 0);
	TextureHandles.push_back(NormalTexture != nullptr ? NormalTexture->GetTextureHandle() : 0);
	TextureHandles.push_back(RoughnessTexture != nullptr ? RoughnessTexture->GetTextureHandle() : 0);
	TextureHandles.push_back(MetallicTexture != nullptr ? MetallicTexture->GetTextureHandle() : 0);
	TextureHandles.push_back(AlphaTexture != nullptr ? AlphaTexture->GetTextureHandle() : 0);
	TextureHandles.push_back(EmissiveTexture != nullptr ? EmissiveTexture->GetTextureHandle() : 0);
	ModelAlbedos.push_back(material->Albedo);
	ModelRoughness.push_back(material->Roughness);
	ModelMetallic.push_back(material->Metallic);
	ModelAlphas.push_back(material->Alpha);
	ModelEmissives.push_back(material->Emission);
	ModelClearCoats.push_back({ material->ClearCoat , material->ClearCoatRoughness });
	return ((material->Emission.x + material->Emission.y + material->Emission.z) / 3.0f) > 0.0f || EmissiveTexture != nullptr;
}

FUSIONCORE::PathTracer::PathTracer(unsigned int width,unsigned int height, std::vector<std::pair<Model*, Material*>>& ModelsToTrace)
{
	glGenTextures(1, &image);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, image, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glGenBuffers(1, &pbo);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);

	glBufferData(GL_PIXEL_PACK_BUFFER, width * height * 3 * sizeof(float), NULL, GL_STREAM_DRAW);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	glGenQueries(1, &queryObject);

	ImageSize = { width,height };


	device = oidn::newDevice(oidn::DeviceType::CUDA);
	device.commit();

	colorBuf = device.newBuffer(ImageSize.x * ImageSize.y * 3 * sizeof(float));
	filter = device.newFilter("RT");

	filter.set("hdr", true); 
	filter.commit();

	if (ModelsToTrace.empty())
	{
		return;
	}
	timer.Set();

	AlignedBuffer<glm::mat4> ModelMatrixes;
	AlignedBuffer<glm::vec3> TriangleCenters;
	AlignedBuffer<glm::vec4> TrianglePositions;
	AlignedBuffer<glm::vec3> TriangleNormals;
	AlignedBuffer<glm::vec3> TriangleTangentsBitangents;
	AlignedBuffer<glm::vec2> TriangleUVs;

	int TextureMapTypeCount = 6;

	//Albedo-Normal-Roughness-Metallic-Alpha-Emissive
	AlignedBuffer<GLuint64> TextureHandles;

	AlignedBuffer<glm::vec4> ModelAlbedos;
	AlignedBuffer<float> ModelRoughness;
	AlignedBuffer<float> ModelMetallic;
	AlignedBuffer<float> ModelAlphas;
	AlignedBuffer<glm::vec4> ModelEmissives;
	AlignedBuffer<glm::vec2> ModelClearCoats;
	std::map<unsigned int,unsigned int> EmissiveObjectIndices;
	int MaterialIndex = -1;

	std::vector<std::pair<glm::vec3, glm::vec3>> TriangleMinMax;
	std::vector<BVHnode> BoundingBoxes;

	ModelCount = ModelsToTrace.size();
	ModelMatrixes.reserve(ModelCount);
	ModelAlbedos.reserve(ModelCount);
	ModelRoughness.reserve(ModelCount);
	ModelAlphas.reserve(ModelCount);
	ModelEmissives.reserve(ModelCount);
	TextureHandles.reserve(ModelCount * TextureMapTypeCount);
	Models.reserve(ModelCount);
	
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
		for (auto& mesh : model.first->Meshes)
		{
			ReserveCount += mesh.GetIndices().size();
		}
	}

	TrianglePositions.reserve(ReserveCount);
	TriangleNormals.reserve(ReserveCount);
	TriangleTangentsBitangents.reserve(ReserveCount * 2);
	TriangleUVs.reserve(ReserveCount);
	TriangleCenters.reserve(ReserveCount / 3);
	TriangleMinMax.reserve(ReserveCount / 3);
	
	int CurrentVertexIndex = 0;

	for (int i = 0; i < ModelCount; i++)
	{
		auto& ModelWithMaterial = ModelsToTrace[i];
		auto& model = ModelWithMaterial.first;
		auto& material = ModelWithMaterial.second;
		
		if (material != nullptr)
		{
			MaterialIndex++;
			ProcessMaterial(material, TextureHandles, ModelAlbedos, ModelRoughness, ModelMetallic, ModelAlphas, ModelEmissives, ModelClearCoats);
		}

		Models.push_back(model);

		ModelMatrix = model->GetTransformation().GetModelMat4();
		NormalMatrix = glm::transpose(glm::inverse(glm::mat3(ModelMatrix)));

		ModelMatrixes.push_back(ModelMatrix);
		
		for (int MeshIndex = 0; MeshIndex < model->Meshes.size(); MeshIndex++)
		{
			auto& mesh = model->Meshes[MeshIndex];
			auto& indices = mesh.GetIndices();
			auto& vertices = mesh.GetVertices();
			int MeshTriangleCount = indices.size() / 3;
			CurrentModelTriangleCount += MeshTriangleCount;
			bool IsMaterialEmissive = false;

			if (material == nullptr)
			{
				MaterialIndex++;
				auto MeshMaterial = &mesh.ImportedMaterial;
				MeshMaterial->MakeMaterialBindlessResident();
				IsMaterialEmissive = ProcessMaterial(MeshMaterial, TextureHandles, ModelAlbedos, ModelRoughness, ModelMetallic, ModelAlphas, ModelEmissives,ModelClearCoats);
				//LOG_PARAMETERS(IsMaterialEmissive);
			}

			for (size_t y = 0; y < indices.size(); y += 3)
			{
				auto& index0 = indices[y];
				auto& index1 = indices[y + 1];
				auto& index2 = indices[y + 2];
				auto& Vertex0 = vertices[index0];

				if (IsMaterialEmissive)
				{
					EmissiveObjectIndices[CurrentVertexIndex] = CurrentVertexIndex;
				}

				TempPosition = glm::vec3(ModelMatrix * glm::vec4(Vertex0->Position, 1.0f));
				CompareVec3MinMax(TriangleMin, TriangleMax, TempPosition);
				TriangleCenter += TempPosition;
				
				TriangleNormals.push_back(glm::normalize(NormalMatrix * Vertex0->Normal));
				TriangleTangentsBitangents.push_back(glm::normalize(NormalMatrix * Vertex0->Tangent));
				TriangleTangentsBitangents.push_back(glm::normalize(NormalMatrix * Vertex0->Bitangent));
				TrianglePositions.push_back(glm::vec4(TempPosition, MaterialIndex));
				TriangleUVs.push_back(Vertex0->TexCoords);

				auto& Vertex1 = vertices[index1];

				TempPosition = glm::vec3(ModelMatrix * glm::vec4(Vertex1->Position, 1.0f));
				CompareVec3MinMax(TriangleMin, TriangleMax, TempPosition);
				TriangleCenter += TempPosition;
				
				TriangleNormals.push_back(glm::normalize(NormalMatrix * Vertex1->Normal));
				TriangleTangentsBitangents.push_back(glm::normalize(NormalMatrix * Vertex1->Tangent));
				TriangleTangentsBitangents.push_back(glm::normalize(NormalMatrix * Vertex1->Bitangent));
				TrianglePositions.push_back(glm::vec4(TempPosition, MaterialIndex));
				TriangleUVs.push_back(Vertex1->TexCoords);

				auto& Vertex2 = vertices[index2];

				TempPosition = glm::vec3(ModelMatrix * glm::vec4(Vertex2->Position, 1.0f));
				CompareVec3MinMax(TriangleMin, TriangleMax, TempPosition);
				TriangleCenter += TempPosition;
				
				TriangleNormals.push_back(glm::normalize(NormalMatrix * Vertex2->Normal));
				TriangleTangentsBitangents.push_back(glm::normalize(NormalMatrix * Vertex2->Tangent));
				TriangleTangentsBitangents.push_back(glm::normalize(NormalMatrix * Vertex2->Bitangent));
				TrianglePositions.push_back(glm::vec4(TempPosition, MaterialIndex));
				TriangleUVs.push_back(Vertex2->TexCoords);

				TriangleCenter *= inv_three;

				TriangleCenters.push_back(TriangleCenter);
				TriangleMinMax.push_back({ TriangleMin,TriangleMax });

				TriangleMin = glm::vec3(std::numeric_limits<float>::max());
				TriangleMax = glm::vec3(std::numeric_limits<float>::lowest());
				TriangleCenter = glm::vec3(0.0f);

				CurrentVertexIndex += 1;
			}
		}
		
		CalculateModelBoundingBox(model, ModelMatrix, BoundingBoxes, min, max);
		
		auto& CurrentBoundingBox = BoundingBoxes.back();
		CurrentBoundingBox.TriangleCount = CurrentModelTriangleCount;
		CurrentBoundingBox.TriangleIndex = TriangleCenters.size() - CurrentModelTriangleCount;
		CurrentModelTriangleCount = 0;
	}

	this->EmissiveObjectCount = EmissiveObjectIndices.size();

	ConstructBottomUpBVH(BottomUpBVHNodes, BoundingBoxes, min, max);

	TopDownBVHnodes.insert(TopDownBVHnodes.end(), BottomUpBVHNodes.begin(), BottomUpBVHNodes.end());
	for (int i = 0; i < BottomUpBVHNodes.size(); i++)
	{
		auto& node = TopDownBVHnodes[i];
		if (node.ChildIndex < 0)
		{
		   ConstructTopDownBVH(TopDownBVHnodes, i, TrianglePositions, TriangleNormals,
			                   TriangleUVs,TriangleCenters,TriangleTangentsBitangents,
			                   EmissiveObjectIndices,TriangleMinMax, 27);
		}
	}
	ModelNodeCount = BoundingBoxes.size();
	
	NodeCount = TopDownBVHnodes.size();
	std::vector<glm::vec3> MinBounds;
	std::vector<glm::vec3> MaxBounds;
	std::vector<float> ChildIndicies;
	std::vector<float> TriangleIndicies;
	std::vector<float> TriangleCounts;

	std::vector<float> EmissiveIndices;

	EmissiveIndices.reserve(EmissiveObjectIndices.size());
	for (const auto& index : EmissiveObjectIndices) {
		EmissiveIndices.push_back(index.second);
	}

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
	
	SetTBObindlessTextureData(AlbedoData,
		AlbedoTexture,
		GL_RGBA32F,
		ModelAlbedos.size() * sizeof(glm::vec4),
		ModelAlbedos.data(),
		GL_STATIC_DRAW);

	SetTBObindlessTextureData(RoughnessData,
		RoughnessTexture,
		GL_R32F,
		ModelRoughness.size() * sizeof(float),
		ModelRoughness.data(),
		GL_STATIC_DRAW);

	SetTBObindlessTextureData(MetallicData,
		MetallicTexture,
		GL_R32F,
		ModelMetallic.size() * sizeof(float),
		ModelMetallic.data(),
		GL_STATIC_DRAW);

	SetTBObindlessTextureData(AlphaData,
		AlphaTexture,
		GL_R32F,
		ModelAlphas.size() * sizeof(float),
		ModelAlphas.data(),
		GL_STATIC_DRAW);

	SetTBObindlessTextureData(EmissiveData,
		EmissiveTexture,
		GL_RGBA32F,
		ModelEmissives.size() * sizeof(glm::vec4),
		ModelEmissives.data(),
		GL_STATIC_DRAW);

	if (EmissiveObjectCount > 0)
	{
		SetTBObindlessTextureData(EmissiveObjectsData,
			EmissiveObjectsTexture,
			GL_R32F,
			EmissiveIndices.size() * sizeof(float),
			EmissiveIndices.data(),
			GL_STATIC_DRAW);
	}

	SetTBObindlessTextureData(ClearCoatData,
		ClearCoatTexture,
		GL_RG32F,
		ModelClearCoats.size() * sizeof(glm::vec2),
		ModelClearCoats.data(),
		GL_STATIC_DRAW);

	SetTBObindlessTextureData(TracerTriangleUVdata,
		TracerTriangleUVTexture,
		GL_RG32F,
		TriangleUVs.size() * sizeof(glm::vec2),
		TriangleUVs.data(),
		GL_STATIC_DRAW);

	SetTBObindlessTextureData(TracerTriangleNormalData,
		TracerTriangleNormalsTexture,
		GL_RGB32F,
		TriangleNormals.size() * sizeof(glm::vec3),
		TriangleNormals.data(),
		GL_STATIC_DRAW);

	SetTBObindlessTextureData(TracerTriangleTangentBitangentData,
		TracerTriangleTangentBitangentTexture,
		GL_RGB32F,
		TriangleTangentsBitangents.size() * sizeof(glm::vec3),
		TriangleTangentsBitangents.data(),
		GL_STATIC_DRAW);

	SetTBObindlessTextureData(TracerTrianglePositionsData,
		TracerTrianglePositionsTexture,
		GL_RGBA32F,
		TrianglePositions.size() * sizeof(glm::vec4),
		TrianglePositions.data(),
		GL_STATIC_DRAW);

	glBindBuffer(GL_TEXTURE_BUFFER, 0);

	ModelMatricesData.Bind();
	ModelMatricesData.BufferDataFill(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4) * ModelMatrixes.size(), ModelMatrixes.data(), GL_STREAM_DRAW);
	ModelMatricesData.BindSSBO(8);
	ModelMatricesData.Unbind();

	ModelTextureHandlesData.Bind();
	ModelTextureHandlesData.BufferDataFill(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint64) * TextureHandles.size(), TextureHandles.data(), GL_STREAM_DRAW);
	ModelTextureHandlesData.BindSSBO(9);
	ModelTextureHandlesData.Unbind();

	TriangleCount = TrianglePositions.size();
	IsInitialized = false;
	ProgressiveRenderedFrameCount = -1;

	LOG("Process took: " << timer.GetMiliseconds());
}

GLint FUSIONCORE::PathTracer::IsPathTracingDone()
{
	GLint PathTracingDone = 0;
	glGetQueryObjectiv(queryObject, GL_QUERY_RESULT_AVAILABLE, &PathTracingDone);
	LOG_PARAMETERS(PathTracingDone);
	return PathTracingDone;
}

void FUSIONCORE::PathTracer::PathTracerDashBoard()
{
	ImGui::Begin("Path Tracer Settings");
	ImGui::Text("Hello");
	if (ImGui::Button("Render"))
	{
		ShouldPathTrace = true;
	}
	ImGui::End();
}

FUSIONCORE::PathTracer::~PathTracer()
{
	glDeleteTextures(1, &this->image);
	glDeleteQueries(1, &queryObject);
}

void FUSIONCORE::PathTracer::Render(Window& window,Shader& shader,Camera3D& camera,CubeMap* Cubemap,unsigned int DenoiseSampleCount)
{
	glm::vec2 WindowSize = window.GetWindowSize();
	static bool IsDenoised = false;
	shader.use();

	if (camera.IsCameraMovedf() || window.IsWindowResizedf() || shader.IsRecompiled())
	{
		ProgressiveRenderedFrameCount = -1;
		IsDenoised = false;
		shader.setVec2("WindowSize", WindowSize);
		shader.setVec3("CameraPosition", camera.Position);
		shader.setMat4("ProjectionViewMat", camera.ProjectionViewMat);
		shader.setInt("ModelCount", ModelCount);
		timer.Set();
	}
	else ProgressiveRenderedFrameCount++;
	
	shader.setInt("ProgressiveRenderedFrameCount", ProgressiveRenderedFrameCount);

	if (!IsInitialized || shader.IsRecompiled())
	{
		this->MinBoundTexture.SendBindlessHandle(shader.GetID(), "MinBounds");
		this->MaxBoundTexture.SendBindlessHandle(shader.GetID(), "MaxBounds");
		this->ChildIndexTexture.SendBindlessHandle(shader.GetID(), "ChildIndicies");
		this->TriangleIndexTexture.SendBindlessHandle(shader.GetID(), "TriangleIndicies");
		this->TriangleCountTexture.SendBindlessHandle(shader.GetID(), "TriangleCounts");
		this->AlbedoTexture.SendBindlessHandle(shader.GetID(), "ModelAlbedos");
		this->RoughnessTexture.SendBindlessHandle(shader.GetID(), "ModelRoughness");
		this->MetallicTexture.SendBindlessHandle(shader.GetID(), "ModelMetallic");
		this->AlphaTexture.SendBindlessHandle(shader.GetID(), "ModelAlphas");
		this->EmissiveTexture.SendBindlessHandle(shader.GetID(), "ModelEmissives");
		if (EmissiveObjectCount > 0) this->EmissiveObjectsTexture.SendBindlessHandle(shader.GetID(), "EmissiveObjects");
		this->ClearCoatTexture.SendBindlessHandle(shader.GetID(), "ModelClearCoats");
		this->TracerTriangleUVTexture.SendBindlessHandle(shader.GetID(), "TriangleUVS");
		this->TracerTriangleNormalsTexture.SendBindlessHandle(shader.GetID(), "TriangleNormals");
		this->TracerTrianglePositionsTexture.SendBindlessHandle(shader.GetID(), "TrianglePositions");
		this->TracerTriangleTangentBitangentTexture.SendBindlessHandle(shader.GetID(), "TriangleTangentsBitangents");
		SendLightsShader(shader);
		IsInitialized = true;
	}
	
	const int SampleCount = 1000;

	if (ProgressiveRenderedFrameCount <= SampleCount)
	{
		float Time = glfwGetTime();
		std::uniform_real_distribution<float> RandomSeed(0.01f, 1.0f);
		std::default_random_engine engine(Time);

		shader.setFloat("Time", Time);
		shader.setFloat("CameraPlaneDistance", camera.FarPlane - camera.NearPlane);
		shader.setFloat("RandomSeed", RandomSeed(engine));

		if (Cubemap != nullptr)
		{
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_CUBE_MAP, Cubemap->GetCubeMapTexture());
			shader.setInt("EnvironmentCubeMap", 3);
		}

		GLuint workGroupSizeX = 32;
		GLuint workGroupSizeY = 32;

		GLuint numGroupsX = (WindowSize.x + workGroupSizeX - 1) / workGroupSizeX;
		GLuint numGroupsY = (WindowSize.y + workGroupSizeY - 1) / workGroupSizeY;

		glDispatchCompute(numGroupsX, numGroupsY, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
		UseShaderProgram(0);
	}
	else
	{

		if (!IsDenoised)
		{
			/*glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
			glBindTexture(GL_TEXTURE_2D, image);

			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, 0);

			float* pixels = (float*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

			std::vector<float> outputBuffer(ImageSize.x * ImageSize.y * 4);*/

			IsDenoised = true;
			ShouldPathTrace = false;

			std::vector<float> outputBuffer(ImageSize.x * ImageSize.y * 3);
			glBindTexture(GL_TEXTURE_2D, image);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, outputBuffer.data());
			colorBuf.write(0, ImageSize.x * ImageSize.y * 3 * sizeof(float), outputBuffer.data());

			Denoise(colorBuf.getData(), colorBuf.getData());
			glBindTexture(GL_TEXTURE_2D, image);

			colorBuf.read(0, ImageSize.x * ImageSize.y * 3 * sizeof(float), outputBuffer.data());
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ImageSize.x, ImageSize.y, GL_RGB, GL_FLOAT, outputBuffer.data());  

			/*glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);

			float* pixels2 = (float*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_READ_ONLY);
			memcpy(pixels2, outputBuffer.data(), ImageSize.x * ImageSize.y * 4);

			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ImageSize.x, ImageSize.y, GL_RGBA, GL_FLOAT, 0);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);*/
			LOG_INF("----------------- Path tracing meta data -----------------" << "\n"
				<<
				"Render time (seconds): " << timer.GetSeconds() << "\n" 
				<<
				"Sample count: " << SampleCount << "\n"
				<<
				"BVH node count: " << this->NodeCount << "\n"
				<<
				"Model count: " << this->Models.size() << "\n"
			);
		}
	}
}

void FUSIONCORE::PathTracer::Denoise(void* ColorBuffer, void* outputBuffer)
{
	filter.setImage("color", ColorBuffer, oidn::Format::Float3, ImageSize.x, ImageSize.y); 
	filter.setImage("output", outputBuffer, oidn::Format::Float3, ImageSize.x, ImageSize.y); 
	filter.commit();

	filter.execute();

	const char* errorMessage;
	if (device.getError(errorMessage) != oidn::Error::None)
		std::cout << "Error: " << errorMessage << std::endl;	
}

void FUSIONCORE::PathTracer::VisualizeBVH(FUSIONCORE::Camera3D& Camera, FUSIONCORE::Shader& Shader, glm::vec3 NodeColor)
{
	static bool initialized = false;
	auto UnitBoxBuffer = GetUnitBoxBuffer();
	
	Shader.use();
	UnitBoxBuffer->BindVAO();

	Shader.setMat4("ProjView", Camera.ProjectionViewMat);
	Shader.setVec3("LightColor", NodeColor);
	if (!initialized)
	{
		this->MinBoundTexture.SendBindlessHandle(Shader.GetID(), "MinBounds");
		this->MaxBoundTexture.SendBindlessHandle(Shader.GetID(), "MaxBounds");
		this->TriangleCountTexture.SendBindlessHandle(Shader.GetID(), "TriangleCounts");
		initialized = true;
	}

	glDrawElementsInstanced(GL_LINES, 32, GL_UNSIGNED_INT, 0, TopDownBVHnodes.size());

	BindVAONull();
	UseShaderProgram(0);
}

