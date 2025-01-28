#include "PathTracerApplication.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <unordered_set>

//#include "FusionCore/PathTracer/PathTracer.hpp"
//#include "../external/nativefiledialog/include/nfd.h"

#define SPEED 6.0f
#define CAMERA_CLOSE_PLANE 0.5f
#define CAMERA_FAR_PLANE 800.0f
const float BlendAmount = 0.04f;
const float InterBlendAmount = 0.04f;

class NewEvent : public FUSIONCORE::Event
{
public:
	NewEvent() :data(1) {};
	int data;
};

void RandomizeLights(FUSIONUTIL::DefaultShaders& Shaders, FUSIONCORE::Shader& Destination)
{
	auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::uniform_real_distribution<float> RandomFloats(-100.0f, 100.0f);
	std::uniform_real_distribution<float> RandomFloatsY(-30000.0f, 30000.0f);
	std::uniform_real_distribution<float> RandomColor(0.0f, 1.0f);
	std::uniform_real_distribution<float> RandomIntensity(40.0f, 60.0f);
	std::default_random_engine engine(seed);

	std::vector<FUSIONCORE::Light> Lights;
	float LightIntensity;

	LightIntensity = RandomIntensity(engine);
	//Lights.emplace_back(glm::vec3(-0.9f, 1.0f, 0.2f), FF_COLOR_TAN, LightIntensity, FF_DIRECTIONAL_LIGHT, LightIntensity);
	//Lights.emplace_back(glm::vec3(1.0f, 0.4f, -0.3f), FF_COLOR_BURNT_SIENNA, LightIntensity, FF_POINT_LIGHT, LightIntensity);

	/*
	for (size_t i = 0; i < 50; i++)
	{
		LightIntensity = RandomIntensity(engine);
		Lights.emplace_back(glm::vec3(RandomFloats(engine), RandomFloatsY(engine), RandomFloats(engine)), glm::vec3(RandomColor(engine), RandomColor(engine), RandomColor(engine)), LightIntensity, FF_POINT_LIGHT, LightIntensity / 30.0f);
	}
	*/

	FUSIONCORE::UploadLightsShader(Destination);
}

std::string HandleImportDialog(const std::string& Filters)
{
	nfdchar_t* OutPath = nullptr;
	std::string path;
	nfdresult_t result = NFD_OpenDialog(Filters.c_str(), NULL, &OutPath);
	if (result == NFD_OKAY)
	{
		if (OutPath != nullptr)
		{
			path = OutPath;
			for (size_t i = 0; i < path.size(); i++)
			{
				if (path.at(i) == '\\')
				{
					path.at(i) = '/';
				}
			}
		}
	}
	else if (result == NFD_CANCEL)
	{
		puts("User pressed cancel.");
	}
	else
	{
		printf("Error: %s\n", NFD_GetError());
	}
	return path;
}

class Solution {
public:
	static int BinarySearch(std::vector<int>& nums, int low, int high, const int& target)
	{
		while (low <= high) {
			int mid = low + (high - low) / 2;

			if (target == nums[mid])
				return mid;

			if (mid < target)
				low = mid + 1;
			else
				high = mid - 1;
		}
		return -1;
	}

	static void EraseMidValue(std::vector<int>& Input)
	{
		Input.erase(Input.begin() + (Input.size() / 2));
	}

	static std::vector<int> twoSum(std::vector<int>& nums, int target) 
	{
		std::sort(nums.begin(), nums.end());

		std::vector<int> CopyNums(nums.begin(), nums.end());

		size_t Mid = CopyNums.size() / 2;
		int Target;
		int result;
		while (CopyNums.size() > 0)
		{
			int MidValue = CopyNums[Mid];
			if (target < MidValue)
			{
				EraseMidValue(CopyNums);
				Mid = CopyNums.size() / 2;
				continue;
			}
			Target = target - MidValue;
			result = BinarySearch(CopyNums, 0, CopyNums.size(), Target);
			if (result == -1)
			{
				EraseMidValue(CopyNums);
				Mid = CopyNums.size() / 2;
			}
			else break;
		}
		std::vector<int> output;
		output.push_back(Mid);
		output.push_back(result);
		return output;
	}
};

int PathTracingDemo()
{
	std::vector<int> TargetArray({9,4,7,1,5});
	Solution::twoSum(TargetArray, 14);


	const int width = 1000;
	const int height = 1000;

	FUSIONCORE::Window ApplicationWindow;
	ApplicationWindow.InitializeWindow(width, height, 4, 6, false, "FusionFrame Engine");

	FUSIONUTIL::InitializeEngineBuffers();
	FUSIONCORE::InitializeCascadedShadowMapTextureArray(256, 1, 256);
	FUSIONUTIL::DefaultShaders Shaders;


	FUSIONCORE::Shader PathTraceComputeShader("Shaders/PathTracer.comp.glsl");
	FUSIONCORE::Shader RadianceBinningComputeShader("Shaders/RadianceBinning.comp.glsl");
	FUSIONCORE::Shader RadiancePrefixSumComputeShader("Shaders/RadiancePrefixSum.comp.glsl");
	FUSIONCORE::Shader RadianceGroupPrefixSumComputeShader("Shaders/RadianceGroupPrefixSum.comp.glsl");
	FUSIONCORE::Shader RadianceAggregatePrefixSumsComputeShader("Shaders/RadianceAggregatePrefixSums.comp.glsl");
	FUSIONCORE::Shader BVHvisualizeShader("Shaders/BVHvisualizeShader.vs", "Shaders/BVHvisualizeShader.fs");

	FUSIONCORE::CubeMap cubemap(*Shaders.CubeMapShader, 64, 64);
	//FUSIONCORE::ImportCubeMap("Resources/sunflowers_puresky_2k.hdr", 4096, cubemap, Shaders);
	FUSIONCORE::ImportCubeMap("Resources/boma_4k.hdr", 512, cubemap, Shaders);
	cubemap.CalculateBinRadiances(RadianceBinningComputeShader, RadiancePrefixSumComputeShader, RadianceGroupPrefixSumComputeShader, RadianceAggregatePrefixSumsComputeShader);

	const FUSIONUTIL::VideoMode mode = FUSIONUTIL::GetVideoMode(FUSIONUTIL::GetPrimaryMonitor());
	FUSIONCORE::GeometryBuffer Gbuffer(mode.width, mode.height);
	FUSIONCORE::ScreenFrameBuffer ScreenFrameBuffer(mode.width, mode.height);

	FUSIONCORE::LightIcon = std::make_unique<FUSIONCORE::Model>("Resources/LightIcon.fbx");

	FUSIONCORE::Camera3D camera3d;
	camera3d.SetPosition(glm::vec3(12.353, 13.326, 15.2838));
	camera3d.SetOrientation(glm::vec3(-0.593494, -0.648119, -0.477182));

	RandomizeLights(Shaders, PathTraceComputeShader);
	Shaders.DeferredPBRshader->use();

	FUSIONCORE::Color FogColor(FF_COLOR_CORNFLOWER_BLUE);
	FogColor.Darker(0.3f);
	FUSIONCORE::SetEnvironment(*Shaders.DeferredPBRshader, 1.0f, FogColor.GetRGB(), FogColor.GetRGB());
	//FUSIONCORE::SetEnvironmentIBL(*Shaders.DeferredPBRshader, 2.0f, glm::vec3(BackGroundColor.x, BackGroundColor.y, BackGroundColor.z));
	FUSIONCORE::UseShaderProgram(0);

	glm::ivec2 WindowSize;
	std::vector<FUSIONCORE::OmniShadowMap*> shadowMaps;
	std::function<void()> shaderPrep = []() {};

	std::vector<std::pair<FUSIONCORE::Model*, FUSIONCORE::Material*>> models;

	FUSIONCORE::PathTracer pathtracer(mode.width, mode.height, models, Shaders);

	glm::ivec2 PrevWindowSize;
	glm::ivec2 PrevWindowPos;
	bool IsFullScreen = false;

	auto window = ApplicationWindow.GetWindow();

	double DeltaTime = 0.0;
	double StartTime = 0.0;
	bool Debug = false;

	FUSIONCORE::Model* PixelModel = nullptr;
	glm::dvec2 CurrentMousePos(0.0f);
	glm::dvec2 PrevMousePos(0.0f);
	bool AllowPathTrace = false;

	double Speed = SPEED;

	std::shared_ptr<FUSIONCORE::Model> ImportedModel;
	FUSIONUTIL::InitializeImguiGLFW(ApplicationWindow.GetWindow());

	auto OnImportModel = [&]() {
		auto OutPath = HandleImportDialog("obj,fbx,gltf,glb");
		if (OutPath.empty()) return;
		models.clear();
		ImportedModel.reset();
		ImportedModel = std::make_shared<FUSIONCORE::Model>(OutPath);
		models.push_back({ ImportedModel.get(),nullptr });
		camera3d.Position = ImportedModel->GetTransformation().InitialObjectScales;
		camera3d.Orientation = glm::normalize(ImportedModel->GetTransformation().Position - camera3d.Position);
		pathtracer.ConstructBVH(models, Shaders);
	};

	while (!ApplicationWindow.ShouldClose())
	{
		StartTime = FUSIONUTIL::GetTime();
		WindowSize = ApplicationWindow.GetWindowSize();
		FUSIONUTIL::CreateFrameImguiGLFW();
		pathtracer.PathTracerDashBoard(OnImportModel);

		PathTraceComputeShader.HotReload(3000);

		PrevMousePos = CurrentMousePos;
		FUSIONUTIL::GetCursorPosition(window, CurrentMousePos.x, CurrentMousePos.y);

		static bool AllowPressF = true;
		if (FUSIONUTIL::IsKeyPressedOnce(window, FF_KEY_F, AllowPressF))
		{
			IsFullScreen = !IsFullScreen;
			if (!IsFullScreen)
			{
				FUSIONUTIL::GetWindowPosition(window, PrevWindowPos.x, PrevWindowPos.y);
				PrevWindowSize = WindowSize;
				FUSIONUTIL::SetWindowMonitor(window,FUSIONUTIL::GetPrimaryMonitor(), 0, 0, mode.width, mode.height, mode.refreshRate);
			}
			else
			{
				FUSIONUTIL::SetWindowMonitor(window, NULL, PrevWindowPos.x, PrevWindowPos.y, PrevWindowSize.x, PrevWindowSize.y, mode.refreshRate);
			}
		}

		static bool AllowPressShift = true;
		if (FUSIONUTIL::GetKey(window,FF_KEY_LEFT_SHIFT) == FF_GLFW_PRESS) Speed = SPEED * 100;
		else Speed = SPEED;

		camera3d.UpdateCameraMatrix(45.0f, (float)WindowSize.x / (float)WindowSize.y, CAMERA_CLOSE_PLANE, 10000.0f, WindowSize);
		camera3d.HandleInputs(ApplicationWindow.GetWindow(), WindowSize, FF_CAMERA_LAYOUT_FIRST_PERSON, Speed * DeltaTime);

		Gbuffer.Bind();
		Gbuffer.SetDrawModeDefault();
		FUSIONUTIL::ClearFrameBuffer(0, 0, WindowSize.x, WindowSize.y, FF_COLOR_VOID);

		static bool AllowPressR = true;
		if (FUSIONUTIL::IsKeyPressedOnce(window, FF_KEY_R, AllowPressR)) AllowPathTrace = !AllowPathTrace;
		
		if (!AllowPathTrace)
		{
			for (auto& model : models)
			{
				model.first->DrawDeferredImportedMaterial(camera3d, *Shaders.GbufferShader,[](){});
			}
			
		}

		ScreenFrameBuffer.Bind();
		FUSIONUTIL::ClearFrameBuffer(0, 0, WindowSize.x, WindowSize.y, FF_COLOR_BLACK);
		Gbuffer.DrawSceneDeferred(camera3d, *Shaders.DeferredPBRshader, [&]() {}, WindowSize, shadowMaps, cubemap, FF_COLOR_VOID, 0.3f);

		
		FUSIONUTIL::GLviewport(0, 0, WindowSize.x, WindowSize.y);
		auto gbufferSize = Gbuffer.GetFBOSize();
		FUSIONCORE::CopyDepthInfoFBOtoFBO(Gbuffer.GetFBO(), { gbufferSize.x ,gbufferSize.y }, ScreenFrameBuffer.GetFBO());
		ScreenFrameBuffer.Bind();
		if (!AllowPathTrace && pathtracer.ShouldDisplayBVH()) pathtracer.VisualizeBVH(camera3d, BVHvisualizeShader, FF_COLOR_RED);

		FUSIONUTIL::GLBindFrameBuffer(FF_GL_FRAMEBUFFER, 0);
		FUSIONUTIL::ClearFrameBuffer(0, 0, WindowSize.x, WindowSize.y, FF_COLOR_VOID);
		if (AllowPathTrace && !FUSIONUTIL::IsAnyItemActive())
		{
			//pathtracer.SetShouldRestart(!AllowPressR);
			pathtracer.Render(ApplicationWindow, PathTraceComputeShader, camera3d, &cubemap);
		}
	
		GLuint ViewportImage = !AllowPathTrace ? ScreenFrameBuffer.GetFBOimage() : pathtracer.GetTracedImage();
	
		FUSIONCORE::DrawTextureOnQuad(ViewportImage, { 0,0 }, { mode.width,mode.height }, camera3d, *Shaders.TextureOnQuadShader, 1.7f, 1.6f);
		FUSIONUTIL::RenderImguiGLFW();

		ApplicationWindow.UpdateWindow();
		DeltaTime = FUSIONUTIL::GetTime() - StartTime;
	}

	FUSIONUTIL::TerminateRenderImguiGLFW();
	FUSIONCORE::TerminateCascadedShadowMapTextureArray();

	ScreenFrameBuffer.clean();
	Gbuffer.clean();
	BVHvisualizeShader.Clear();
	PathTraceComputeShader.Clear();
	RadianceBinningComputeShader.Clear();
	RadiancePrefixSumComputeShader.Clear();
	RadianceAggregatePrefixSumsComputeShader.Clear();
	RadianceGroupPrefixSumComputeShader.Clear();


	ApplicationWindow.TerminateWindow();
	FUSIONUTIL::TerminateGLFW();
	return 0;
}
