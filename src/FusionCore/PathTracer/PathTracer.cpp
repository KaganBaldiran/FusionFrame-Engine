#include "PathTracer.hpp"
#include <glew.h>
#include <glfw3.h>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "imgui_internal.h"
#include <omp.h>

template <typename T>
using AlignedBuffer = std::vector<T, FUSIONUTIL::AlignedAllocator<T, 16>>;

inline glm::vec3 MinVec3(const glm::vec3& input0,const glm::vec3& input1)
{
	return { glm::min(input0.x, input1.x) ,glm::min(input0.y, input1.y),glm::min(input0.z, input1.z) };
}

inline glm::vec3 MaxVec3(const glm::vec3& input0, const glm::vec3& input1)
{
	return { glm::max(input0.x, input1.x) ,glm::max(input0.y, input1.y),glm::max(input0.z, input1.z) };
}

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


inline void SetTBOTextureData(FUSIONCORE::TBO& tbo,FUSIONCORE::Texture2D &texture,const GLenum& internalUsage,const GLsizeiptr& size,const void* data,const GLenum& usage)
{
	tbo.Bind();
	tbo.BufferDataFill(size, data, usage);
	
	texture.Bind(GL_TEXTURE_BUFFER);
	tbo.TexBuffer(internalUsage);
}

inline void CalculateModelBoundingBox(FUSIONCORE::Model*& Model,
	                            glm::mat4& ModelMatrix,
	                            std::vector<BVHnode>& ModelBoundingBoxes,
								glm::vec3 &min,
								glm::vec3 &max)
{
	FUSIONCORE::WorldTransform& objectTransform = Model->GetTransformation();
	BVHnode newNode;

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

	min = MinVec3(min, newNode.Min);
	max = MaxVec3(max, newNode.Max);

	objectTransform.IsTransformed = false;
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
	constexpr int TestCountPerAXis = 35;
	
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
						AlignedBuffer<glm::vec4>& TriangleNormals,
						AlignedBuffer<glm::vec2>& TriangleUVs,
						AlignedBuffer<glm::vec3> &TriangleCenters,
						AlignedBuffer<glm::vec4> &TriangleTangentsBitangents,
						std::map<unsigned int, std::pair<unsigned int, float>> &EmissiveObjectIndices,
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
					if (Indexi && !IndexSwap)
					{
						EmissiveObjectIndices[swap] = { swap,EmissiveObjectIndices[i].second };
						EmissiveObjectIndices.erase(i);
					}
					else if (IndexSwap && !Indexi)
					{
						EmissiveObjectIndices[i] = {i,EmissiveObjectIndices[swap].second };
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
					AlignedBuffer<glm::vec2> &ModelAlphas,
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
	ModelAlphas.push_back({ material->Alpha,material->IOR });
	ModelEmissives.push_back(material->Emission);
	ModelClearCoats.push_back({ material->ClearCoat , material->ClearCoatRoughness });
	return ((material->Emission.x + material->Emission.y + material->Emission.z) / 3.0f) > 0.0f || EmissiveTexture != nullptr;
}

FUSIONCORE::PathTracer::PathTracer(const unsigned int& width, const unsigned int& height, std::vector<std::pair<Model*, Material*>>& ModelsToTrace, FUSIONUTIL::DefaultShaders& Shaders)
{
	InitializeImages(width, height);
	ImageSize = { width,height };

	device = oidn::newDevice(OIDN_DEVICE_TYPE_DEFAULT);
	device.commit();

	colorBuf = device.newBuffer(ImageSize.x * ImageSize.y * 3 * sizeof(float));
	NormalBuf = device.newBuffer(ImageSize.x * ImageSize.y * 3 * sizeof(float));
	AlbedoBuf = device.newBuffer(ImageSize.x * ImageSize.y * 3 * sizeof(float));
	filter = device.newFilter("RT");

	filter.set("hdr", true); 
	filter.commit();
	
	ConstructBVH(ModelsToTrace,Shaders);

	ShouldDisplayTheEnv = false;
	EnableDenoising = true;
	TargetBounceCount = 15;
	EnvironmentLightIntensity = 1.0f;
	//bool value to turn on/off - Distance - Intensity - Secondary paths bounce count
	DoFattributes = { 0.0f,5.0f,0.01f,10 };
	IsDoFenabled = false;
	DoFbounceCount = 10;

	PreviouslyUsedCubeMap = nullptr;
	PreviousImportedHDRIcount = 0;

	RandomSeed = std::make_unique<std::uniform_real_distribution<float>>(0.01f, 1.0f);
}

void FUSIONCORE::PathTracer::PathTracerDashBoard(std::function<void()> OnImportModel, std::function<void()> OnSaveScreen, std::function<void()> AdditionalFunctionality)
{
	ImGui::Begin("Path Tracer Settings");
	ImGui::Text(("Sample: " + std::to_string(std::max(this->ProgressiveRenderedFrameCount,0)) + " / " + std::to_string(this->TargetSampleCount)).c_str());
	if (ImGui::Button("Render"))
	{
		ShouldRestart = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Import Model"))
	{
		OnImportModel();
		ShouldRestart = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Save Screen"))
	{
		OnSaveScreen();
	}
	ImGui::SameLine();
	if (ImGui::Button("Conclude Rendering"))
	{
		TargetSampleCount = ProgressiveRenderedFrameCount;
	}
	ImGui::SeparatorText("Parameters");
	if (ImGui::SliderInt("Bounce Count", &TargetBounceCount, 1, 100)) ShouldRestart = true;
	if (ImGui::SliderInt("Sample Count", &TargetSampleCount, 1, 15000)) ShouldRestart = true;
	if (ImGui::SliderFloat("Environment Light Intensity", &EnvironmentLightIntensity, 0.0f , 10.0f)) ShouldRestart = true;
	if (ImGui::Checkbox("Display BVH", &ShouldDisplayBVHv));
	if (ImGui::Checkbox("Display Environment Map", &ShouldDisplayTheEnv)) ShouldRestart = true;
	if (ImGui::Checkbox("Enable Denoising", &EnableDenoising)) ShouldRestart = true;
	if (ImGui::Checkbox("Enable DoF", &IsDoFenabled))
	{
		ShouldRestart = true;
		DoFattributes.x = (int)IsDoFenabled;
	}
	if (ImGui::SliderFloat("DoF distance", &DoFattributes.y, 0.0f, 100.0f)) ShouldRestart = true;
	if (ImGui::SliderFloat("DoF intensity", &DoFattributes.z, 0.0f, 2.0f)) ShouldRestart = true;
	if (ImGui::SliderInt("DoF Bounce Count", &DoFbounceCount, 1, 50))
	{
		ShouldRestart = true;
		DoFattributes.w = DoFbounceCount;
	}
	AdditionalFunctionality();
	ImGui::End();
}

FUSIONCORE::PathTracer::~PathTracer()
{
	glDeleteTextures(1, &this->image);
	glDeleteTextures(1, &this->NormalImage);
	glDeleteTextures(1, &this->AlbedoImage);
}

void FUSIONCORE::PathTracer::Render(Window& window,Shader& shader,Camera3D& camera,CubeMap* Cubemap,unsigned int DenoiseSampleCount)
{
	glm::vec2 WindowSize = window.GetWindowSize(); 
	shader.use();

	if (ShouldRestart || camera.IsCameraMovedf() || window.IsWindowResizedf() || shader.IsRecompiled() ||  
		PreviouslyUsedCubeMap != Cubemap || PreviousImportedHDRIcount != Cubemap->GetImportedHDRICount())
	{
		ProgressiveRenderedFrameCount = -1;
		IsDenoised = false;
		ShouldRestart = false;
		shader.setVec2("WindowSize", WindowSize);
		shader.setVec3("CameraPosition", camera.Position);
		shader.setVec4("DoFattributes", DoFattributes);
		shader.setMat4("ProjectionViewMat", camera.ProjectionViewMat);
		shader.setInt("ModelCount", ModelCount);
		shader.setInt("AlbedoCount", AlbedoCount);
		shader.setInt("NodeCount", NodeCount);
		shader.setInt("TriangleCount", TriangleCount);
		shader.setFloat("EnvironmentLightIntensity", EnvironmentLightIntensity);
		shader.setFloat("TotalEmissiveArea", TotalEmissiveArea);
		shader.setBool("ShouldDisplayTheEnv", ShouldDisplayTheEnv);
		shader.setInt("BounceCount", TargetBounceCount);
		BVHvec4Data.BindSSBO(20);
		BVHfloatData.BindSSBO(21);
		MaterialFloatValuesData.BindSSBO(17);
		MeshData.BindSSBO(18);

		PreviouslyUsedCubeMap = Cubemap;
		PreviousImportedHDRIcount = Cubemap->GetImportedHDRICount();

		if (Cubemap != nullptr)
		{
			glActiveTexture(GL_TEXTURE18);
			glBindTexture(GL_TEXTURE_CUBE_MAP, Cubemap->GetCubeMapTexture());
			shader.setInt("EnvironmentCubeMap", 18);

			glActiveTexture(GL_TEXTURE19);
			glBindTexture(GL_TEXTURE_CUBE_MAP, Cubemap->GetConvDiffCubeMap());
			shader.setInt("ConvolutedEnvironmentCubeMap", 19);
		}

		timer.Set();
		InitialTime = FUSIONUTIL::GetTime();
	}
	else if(ProgressiveRenderedFrameCount < TargetSampleCount) ProgressiveRenderedFrameCount++;
	
	shader.setInt("ProgressiveRenderedFrameCount", ProgressiveRenderedFrameCount);

	if (!IsInitialized || shader.IsRecompiled())
	{
		this->RoughnessTexture.BindTextureBuffer(0, shader.GetID(), "ModelRoughness");
		this->AlbedoTexture.BindTextureBuffer(1, shader.GetID(), "ModelAlbedos");
		this->MetallicTexture.BindTextureBuffer(2, shader.GetID(), "ModelMetallic");
		this->AlphaTexture.BindTextureBuffer(3, shader.GetID(), "ModelAlphas");
		this->EmissiveTexture.BindTextureBuffer(4, shader.GetID(), "ModelEmissives");
		if (EmissiveObjectCount > 0) this->EmissiveObjectsTexture.BindTextureBuffer(5, shader.GetID(), "EmissiveObjects");
		this->ClearCoatTexture.BindTextureBuffer(6, shader.GetID(), "ModelClearCoats");
		this->TracerTriangleUVTexture.BindTextureBuffer(7, shader.GetID(), "TriangleUVS");

		SendLightsShader(shader);
		IsInitialized = true;
	}

	if (ProgressiveRenderedFrameCount < TargetSampleCount)
	{
		float Time = glfwGetTime();

		std::default_random_engine engine(Time);

		shader.setFloat("Time", Time);
		shader.setFloat("CameraPlaneDistance", camera.FarPlane - camera.NearPlane);
		shader.setFloat("RandomSeed", (*RandomSeed)(engine));
		shader.setInt("TargetSampleCount", TargetSampleCount);

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
		if (!IsDenoised && EnableDenoising)
		{
			IsDenoised = true;

			std::vector<float> TempBuffer(ImageSize.x * ImageSize.y * 3);
			glBindTexture(GL_TEXTURE_2D, image);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, TempBuffer.data());
			colorBuf.write(0, ImageSize.x * ImageSize.y * 3 * sizeof(float), TempBuffer.data());

			glBindTexture(GL_TEXTURE_2D, NormalImage);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, TempBuffer.data());
			NormalBuf.write(0, ImageSize.x * ImageSize.y * 3 * sizeof(float), TempBuffer.data());

			glBindTexture(GL_TEXTURE_2D, AlbedoImage);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, TempBuffer.data());
			AlbedoBuf.write(0, ImageSize.x * ImageSize.y * 3 * sizeof(float), TempBuffer.data());

			Denoise(colorBuf.getData(), NormalBuf.getData(), AlbedoBuf.getData(),colorBuf.getData());

			glBindTexture(GL_TEXTURE_2D, image);
			colorBuf.read(0, ImageSize.x * ImageSize.y * 3 * sizeof(float), TempBuffer.data());
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ImageSize.x, ImageSize.y, GL_RGB, GL_FLOAT, TempBuffer.data());  
			
			LOG_INF("----------------- Path tracing meta data -----------------" << "\n"
				<<
				"Render time (seconds): " <<  FUSIONUTIL::GetTime() - InitialTime << "\n"
				<<
				"Sample count: " << TargetSampleCount << "\n"
				<<
				"BVH node count: " << this->NodeCount << "\n"
			);
		}
	}
}

void FUSIONCORE::PathTracer::ConstructBVH(std::vector<std::pair<Model*, Material*>>& ModelsToTrace, FUSIONUTIL::DefaultShaders& Shaders)
{
	IsInitialized = false;
	ProgressiveRenderedFrameCount = -1;
	TargetSampleCount = 100;
	ShouldRestart = true;
	IsDenoised = false;
	ShouldDisplayBVHv = false;

	if (ModelsToTrace.empty())
	{
		return;
	}
	std::cout << "\033[2K\rConstructing the BVH... Progress: " << 0 << "%" << std::flush;

	bool IsFirstTime = TopDownBVHnodes.size() == 0;

	timer.Set();
	this->TopDownBVHnodes.clear();
	this->BottomUpBVHNodes.clear();

	AlignedBuffer<glm::mat4> ModelMatrixes;
	AlignedBuffer<glm::vec3> TriangleCenters;
	AlignedBuffer<glm::vec4> TrianglePositions;
	AlignedBuffer<glm::vec4> TriangleNormals;
	AlignedBuffer<glm::vec4> TriangleTangentsBitangents;
	AlignedBuffer<glm::vec2> TriangleUVs;

	int TextureMapTypeCount = 6;

	//Albedo-Normal-Roughness-Metallic-Alpha-Emissive
	AlignedBuffer<GLuint64> TextureHandles;

	AlignedBuffer<glm::vec4> ModelAlbedos;
	AlignedBuffer<float> ModelRoughness;
	AlignedBuffer<float> ModelMetallic;
	AlignedBuffer<glm::vec2> ModelAlphas;
	AlignedBuffer<glm::vec4> ModelEmissives;
	AlignedBuffer<glm::vec2> ModelClearCoats;
	std::map<unsigned int, std::pair<unsigned int,float>> EmissiveObjectIndices;
	int MaterialIndex = -1;

	std::vector<std::pair<glm::vec3, glm::vec3>> TriangleMinMax;
	std::vector<BVHnode> BoundingBoxes;

	ModelCount = ModelsToTrace.size();
	ModelMatrixes.reserve(ModelCount);
	ModelAlbedos.reserve(ModelCount);
	ModelRoughness.reserve(ModelCount);
	ModelAlphas.reserve(ModelCount);
	ModelEmissives.reserve(ModelCount);
	ModelClearCoats.reserve(ModelCount);
	TextureHandles.reserve(ModelCount * TextureMapTypeCount);

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
	TotalEmissiveArea = 0.0f;

	for (int i = 0; i < ModelCount; i++)
	{
		auto& ModelWithMaterial = ModelsToTrace[i];
		auto& model = ModelWithMaterial.first;
		auto& material = ModelWithMaterial.second;

		if (material != nullptr)
		{
			MaterialIndex++;
			material->MakeMaterialMipmapBindlessResident(4, *Shaders.TextureMipmapRenderShader);
			ProcessMaterial(material, TextureHandles, ModelAlbedos, ModelRoughness, ModelMetallic, ModelAlphas, ModelEmissives, ModelClearCoats);
		}

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
				MeshMaterial->MakeMaterialMipmapBindlessResident(4, *Shaders.TextureMipmapRenderShader);
				IsMaterialEmissive = ProcessMaterial(MeshMaterial, TextureHandles, ModelAlbedos, ModelRoughness, ModelMetallic, ModelAlphas, ModelEmissives, ModelClearCoats);
			}

			for (size_t y = 0; y < indices.size(); y += 3)
			{
				auto& index0 = indices[y];
				auto& index1 = indices[y + 1];
				auto& index2 = indices[y + 2];
				auto& Vertex0 = vertices[index0];
				auto& Vertex1 = vertices[index1];
				auto& Vertex2 = vertices[index2];

				TempPosition = glm::vec3(ModelMatrix * glm::vec4(Vertex0->Position, 1.0f));
				CompareVec3MinMax(TriangleMin, TriangleMax, TempPosition);
				TriangleCenter += TempPosition;

				TriangleNormals.push_back(glm::vec4(glm::normalize(NormalMatrix * Vertex0->Normal),0.0f));
				TriangleTangentsBitangents.push_back(glm::vec4(glm::normalize(NormalMatrix * Vertex0->Tangent), 0.0f));
				TriangleTangentsBitangents.push_back(glm::vec4(glm::normalize(NormalMatrix * Vertex0->Bitangent), 0.0f));
				TrianglePositions.push_back(glm::vec4(TempPosition, MaterialIndex));
				TriangleUVs.push_back(Vertex0->TexCoords);


				TempPosition = glm::vec3(ModelMatrix * glm::vec4(Vertex1->Position, 1.0f));
				CompareVec3MinMax(TriangleMin, TriangleMax, TempPosition);
				TriangleCenter += TempPosition;

				TriangleNormals.push_back(glm::vec4(glm::normalize(NormalMatrix * Vertex1->Normal), 0.0f));
				TriangleTangentsBitangents.push_back(glm::vec4(glm::normalize(NormalMatrix * Vertex1->Tangent), 0.0f));
				TriangleTangentsBitangents.push_back(glm::vec4(glm::normalize(NormalMatrix * Vertex1->Bitangent), 0.0f));
				TrianglePositions.push_back(glm::vec4(TempPosition, MaterialIndex));
				TriangleUVs.push_back(Vertex1->TexCoords);


				TempPosition = glm::vec3(ModelMatrix * glm::vec4(Vertex2->Position, 1.0f));
				CompareVec3MinMax(TriangleMin, TriangleMax, TempPosition);
				TriangleCenter += TempPosition;

				TriangleNormals.push_back(glm::vec4(glm::normalize(NormalMatrix * Vertex2->Normal), 0.0f));
				TriangleTangentsBitangents.push_back(glm::vec4(glm::normalize(NormalMatrix * Vertex2->Tangent), 0.0f));
				TriangleTangentsBitangents.push_back(glm::vec4(glm::normalize(NormalMatrix * Vertex2->Bitangent), 0.0f));
				TrianglePositions.push_back(glm::vec4(TempPosition, MaterialIndex));
				TriangleUVs.push_back(Vertex2->TexCoords);

				if (IsMaterialEmissive)
				{
					glm::vec4& VertexPos0 = TrianglePositions.at(TrianglePositions.size() - 1);
					glm::vec3 Edge0 = glm::vec3(TrianglePositions.at(TrianglePositions.size() - 2)) - glm::vec3(VertexPos0);
					glm::vec3 Edge1 = glm::vec3(TrianglePositions.at(TrianglePositions.size() - 3)) - glm::vec3(VertexPos0);
					float TriangleArea = 0.5f * glm::length(glm::cross(Edge0, Edge1));
					TotalEmissiveArea += TriangleArea;
					EmissiveObjectIndices[CurrentVertexIndex] = { CurrentVertexIndex,TriangleArea };
					//LOG(TriangleArea << " " << TotalEmissiveArea << " " << Vec3<float>(Edge0) << " " << Vec3<float>(Edge1));
				}

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

	std::cout << "\033[2K\rConstructing the BVH... Progress: " << 40 << "%" << std::flush;

	this->EmissiveObjectCount = EmissiveObjectIndices.size();
	ConstructBottomUpBVH(BottomUpBVHNodes, BoundingBoxes, min, max);

	TopDownBVHnodes.insert(TopDownBVHnodes.end(), BottomUpBVHNodes.begin(), BottomUpBVHNodes.end());
	for (int i = 0; i < BottomUpBVHNodes.size(); i++)
	{
		auto& node = TopDownBVHnodes[i];
		if (node.ChildIndex < 0)
		{
			ConstructTopDownBVH(TopDownBVHnodes, i, TrianglePositions, TriangleNormals,
				TriangleUVs, TriangleCenters, TriangleTangentsBitangents,
				EmissiveObjectIndices, TriangleMinMax,27);
		}
		std::cout << "\033[2K\rConstructing the BVH... Progress: " << 40.0f + ((30.0f / BottomUpBVHNodes.size()) * (i + 1)) << "%" << std::flush;
	}
	ModelNodeCount = BoundingBoxes.size();

	std::cout << "\033[2K\rConstructing the BVH... Progress: " << 70 << "%" << std::flush;

	NodeCount = TopDownBVHnodes.size();
	AlignedBuffer<glm::vec4> Bounds;
	AlignedBuffer<int> ChildIndicesTriIndicesCounts;

	std::vector<glm::vec2> EmissiveIndices;

	EmissiveIndices.reserve(EmissiveObjectIndices.size());
	for (const auto& index : EmissiveObjectIndices) {
		EmissiveIndices.push_back({ index.second.first,index.second.second });
	}

	size_t sizeOfBounds = 2 * NodeCount * sizeof(glm::vec4);
	size_t sizeOfChildIndicesTriIndicesCounts = 3 * NodeCount * sizeof(int);

	Bounds.resize(NodeCount * 2);
	ChildIndicesTriIndicesCounts.resize(3 * NodeCount);
	for (int i = 0; i < NodeCount; i++) {
		auto& node = TopDownBVHnodes[i];

		Bounds[i] = glm::vec4(node.Min,0.0f);
		Bounds[i + NodeCount] = glm::vec4(node.Max,0.0f);
		ChildIndicesTriIndicesCounts[i] = node.ChildIndex;
		ChildIndicesTriIndicesCounts[NodeCount + i] = node.TriangleIndex;
		ChildIndicesTriIndicesCounts[2 * NodeCount + i] = node.TriangleCount;
	}

	BVHvec4Data.Bind();
	BVHvec4Data.BufferDataFill(GL_SHADER_STORAGE_BUFFER, sizeOfBounds, Bounds.data(), GL_STREAM_DRAW);
	BVHvec4Data.BindSSBO(20);
	BVHvec4Data.Unbind();

	BVHfloatData.Bind();
	BVHfloatData.BufferDataFill(GL_SHADER_STORAGE_BUFFER, sizeOfChildIndicesTriIndicesCounts, ChildIndicesTriIndicesCounts.data(), GL_STREAM_DRAW);
	BVHfloatData.BindSSBO(21);
	BVHfloatData.Unbind();

	AlignedBuffer<char> CombinedMeshvec4data;
	size_t sizeOfTriangleNormals = TriangleNormals.size() * sizeof(glm::vec4);
	size_t sizeOfTriangleTangentBitangentNormals = TriangleTangentsBitangents.size() * sizeof(glm::vec4);
	size_t sizeOfTrianglePositions = TrianglePositions.size() * sizeof(glm::vec4);
    size_t TotalMeshVec4BufferSize = sizeOfTriangleNormals + sizeOfTriangleTangentBitangentNormals + sizeOfTrianglePositions;

	CombinedMeshvec4data.resize(TotalMeshVec4BufferSize);
	std::memcpy(CombinedMeshvec4data.data(),TriangleNormals.data(), sizeOfTriangleNormals);
	std::memcpy(CombinedMeshvec4data.data() + sizeOfTriangleNormals, TriangleTangentsBitangents.data(), sizeOfTriangleTangentBitangentNormals);
	std::memcpy(CombinedMeshvec4data.data() + sizeOfTriangleNormals + sizeOfTriangleTangentBitangentNormals, TrianglePositions.data(), sizeOfTrianglePositions);

	MeshData.Bind();
	MeshData.BufferDataFill(GL_SHADER_STORAGE_BUFFER, TotalMeshVec4BufferSize, CombinedMeshvec4data.data(), GL_STREAM_DRAW);
	MeshData.BindSSBO(18);
	MeshData.Unbind();

	std::cout << "\033[2K\rConstructing the BVH... Progress: " << 85 << "%" << std::flush;

	SetTBOTextureData(ClearCoatData,
		ClearCoatTexture,
		GL_RG32F,
		ModelClearCoats.size() * sizeof(glm::vec2),
		ModelClearCoats.data(),
		GL_STATIC_DRAW);

	SetTBOTextureData(TracerTriangleUVdata,
		TracerTriangleUVTexture,
		GL_RG32F,
		TriangleUVs.size() * sizeof(glm::vec2),
		TriangleUVs.data(),
		GL_STATIC_DRAW);

	SetTBOTextureData(AlphaData,
		AlphaTexture,
		GL_RG32F,
		ModelAlphas.size() * sizeof(glm::vec2),
		ModelAlphas.data(),
		GL_STATIC_DRAW);

	SetTBOTextureData(RoughnessData,
		RoughnessTexture,
		GL_R32F,
		ModelRoughness.size() * sizeof(float),
		ModelRoughness.data(),
		GL_STATIC_DRAW);

	SetTBOTextureData(MetallicData,
		MetallicTexture,
		GL_R32F,
		ModelMetallic.size() * sizeof(float),
		ModelMetallic.data(),
		GL_STATIC_DRAW);


	SetTBOTextureData(AlbedoData,
		AlbedoTexture,
		GL_RGBA32F,
		ModelAlbedos.size() * sizeof(glm::vec4),
		ModelAlbedos.data(),
		GL_STATIC_DRAW);

	SetTBOTextureData(EmissiveData,
		EmissiveTexture,
		GL_RGBA32F,
		ModelEmissives.size() * sizeof(glm::vec4),
		ModelEmissives.data(),
		GL_STATIC_DRAW);

	if (EmissiveObjectCount > 0)
	{
		SetTBOTextureData(EmissiveObjectsData,
			EmissiveObjectsTexture,
			GL_RG32F,
			EmissiveIndices.size() * sizeof(glm::vec2),
			EmissiveIndices.data(),
			GL_STATIC_DRAW);
	}

	glBindBuffer(GL_TEXTURE_BUFFER, 0);

	std::cout << "\033[2K\rConstructing the BVH... Progress: " << 95 << "%" << std::flush;

	ModelTextureHandlesData.Bind();
	ModelTextureHandlesData.BufferDataFill(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint64) * TextureHandles.size(), TextureHandles.data(), GL_STREAM_DRAW);
	ModelTextureHandlesData.BindSSBO(9);
	ModelTextureHandlesData.Unbind();

	TriangleCount = TrianglePositions.size();

	std::cout << "\033[2K\rConstructed the BVH" << std::flush;
	std::cout << std::endl;
}

void FUSIONCORE::PathTracer::Denoise(void* ColorBuffer, void* NormalBuffer, void* AlbedoBuffer, void* outputBuffer)
{
	filter.setImage("color", ColorBuffer, oidn::Format::Float3, ImageSize.x, ImageSize.y);
	filter.setImage("albedo", AlbedoBuffer, oidn::Format::Float3, ImageSize.x, ImageSize.y);
	filter.setImage("normal", NormalBuffer, oidn::Format::Float3, ImageSize.x, ImageSize.y);
	filter.setImage("output", outputBuffer, oidn::Format::Float3, ImageSize.x, ImageSize.y); 
	filter.commit();

	filter.execute();

	const char* errorMessage;
	if (device.getError(errorMessage) != oidn::Error::None)
		std::cout << "Error: " << errorMessage << std::endl;	
}

void FUSIONCORE::PathTracer::InitializeImages(const unsigned int& width, const unsigned int& height)
{
	glGenTextures(1, &image);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, image, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);


	glGenTextures(1, &NormalImage);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, NormalImage);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(1, NormalImage, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);


	glGenTextures(1, &AlbedoImage);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, AlbedoImage);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(2, AlbedoImage, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
}

void FUSIONCORE::PathTracer::VisualizeBVH(FUSIONCORE::Camera3D& Camera, FUSIONCORE::Shader& Shader, glm::vec3 NodeColor)
{
	static bool initialized = false;
	auto UnitBoxBuffer = GetUnitBoxBuffer();
	
	Shader.use();
	UnitBoxBuffer->BindVAO();

	Shader.setMat4("ProjView", Camera.ProjectionViewMat);
	Shader.setVec3("LightColor", NodeColor);
	Shader.setInt("NodeCount", NodeCount);
	BVHvec4Data.BindSSBO(20);

	glDrawElementsInstanced(GL_LINES, 32, GL_UNSIGNED_INT, 0, TopDownBVHnodes.size());

	BindVAONull();
	UseShaderProgram(0);
}

