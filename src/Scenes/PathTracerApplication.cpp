#include "PathTracerApplication.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <unordered_set>

#include "../external/nativefiledialog/include/nfd.h"

void PathTracingDemo::RandomizeLights(FUSIONUTIL::DefaultShaders& Shaders, FUSIONCORE::Shader& Destination)
{
	auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::uniform_real_distribution<float> RandomFloats(-100.0f, 100.0f);
	std::uniform_real_distribution<float> RandomFloatsY(-30000.0f, 30000.0f);
	std::uniform_real_distribution<float> RandomColor(0.0f, 1.0f);
	std::uniform_real_distribution<float> RandomIntensity(40.0f, 60.0f);
	std::default_random_engine engine(seed);

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
	else if (result == NFD_CANCEL) {}
	else
	{
		printf("Error: %s\n", NFD_GetError());
	}
	return path;
}

std::string HandleSaveDialog(const std::string& Filters)
{
	nfdchar_t* OutPath = nullptr;
	std::string path;
	nfdresult_t result = NFD_SaveDialog(Filters.c_str(),NULL, &OutPath);
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
	else if (result == NFD_CANCEL) {}
	else
	{
		printf("Error: %s\n", NFD_GetError());
	}
	return path;
}

void DecomposeMatrix(const glm::mat4& Input, glm::vec3& Position, glm::quat& Rotation, glm::vec3& Scale)
{
	Position = Input[3];
	for (int i = 0; i < 3; i++)
	{
		Scale[i] = glm::length(glm::vec3(Input[i]));
	}
	const glm::mat3 rotationMatrix(
		glm::vec3(Input[0]) / Scale[0],
		glm::vec3(Input[1]) / Scale[1],
		glm::vec3(Input[2]) / Scale[2]);
	Rotation = glm::quat_cast(rotationMatrix);
}

PathTracingDemo::~PathTracingDemo()
{
	FUSIONUTIL::TerminateRenderImguiGLFW();
	FUSIONCORE::TerminateCascadedShadowMapTextureArray();

	ScreenFrameBuffer->clean();
	Gbuffer->clean();
	BVHvisualizeShader->Clear();
	PathTraceComputeShader->Clear();
	RadianceBinningComputeShader->Clear();
	RadiancePrefixSumComputeShader->Clear();
	RadianceAggregatePrefixSumsComputeShader->Clear();
	RadianceGroupPrefixSumComputeShader->Clear();

	ApplicationWindow.TerminateWindow();
	FUSIONUTIL::TerminateGLFW();
}

PathTracingDemo::PathTracingDemo()
{
	ApplicationWindow.InitializeWindow(width, height, 4, 6, false, "Path Tracer");
	Shaders = std::make_shared<FUSIONUTIL::DefaultShaders>();
	FUSIONCORE::InitializeCascadedShadowMapTextureArray(256, 1, 256);

	PathTraceComputeShader = std::make_shared<FUSIONCORE::Shader>("Shaders/PathTracer.comp.glsl");
	RadianceBinningComputeShader = std::make_shared<FUSIONCORE::Shader>("Shaders/RadianceBinning.comp.glsl");
	RadiancePrefixSumComputeShader = std::make_shared<FUSIONCORE::Shader>("Shaders/RadiancePrefixSum.comp.glsl");
	RadianceGroupPrefixSumComputeShader = std::make_shared<FUSIONCORE::Shader>("Shaders/RadianceGroupPrefixSum.comp.glsl");
	RadianceAggregatePrefixSumsComputeShader = std::make_shared<FUSIONCORE::Shader>("Shaders/RadianceAggregatePrefixSums.comp.glsl");
	BVHvisualizeShader = std::make_shared<FUSIONCORE::Shader>("Shaders/BVHvisualizeShader.vs", "Shaders/BVHvisualizeShader.fs");

	HDRIsize = 1024;

	cubemap = std::make_shared<FUSIONCORE::CubeMap>(*Shaders->CubeMapShader, 64, 64);
	//FUSIONCORE::ImportCubeMap("Resources/sunflowers_puresky_2k.hdr", 4096, cubemap, Shaders);
	FUSIONCORE::ImportCubeMap("Resources/boma_4k.hdr", HDRIsize, *cubemap, *Shaders);
	cubemap->CalculateBinRadiances(*RadianceBinningComputeShader, *RadiancePrefixSumComputeShader, *RadianceGroupPrefixSumComputeShader, *RadianceAggregatePrefixSumsComputeShader);

	mode = FUSIONUTIL::GetVideoMode(FUSIONUTIL::GetPrimaryMonitor());
	Gbuffer = std::make_shared<FUSIONCORE::GeometryBuffer>(mode.width, mode.height);
	ScreenFrameBuffer = std::make_shared<FUSIONCORE::ScreenFrameBuffer>(mode.width, mode.height);

	FUSIONCORE::LightIcon = std::make_unique<FUSIONCORE::Model>("Resources/LightIcon.fbx");

	camera3d = std::make_shared<FUSIONCORE::Camera3D>();

	camera3d->SetPosition(glm::vec3(12.353, 13.326, 15.2838));
	camera3d->SetOrientation(glm::vec3(-0.593494, -0.648119, -0.477182));

	RandomizeLights(*Shaders, *PathTraceComputeShader);
	Shaders->DeferredPBRshader->use();

	FUSIONCORE::Color FogColor(FF_COLOR_CORNFLOWER_BLUE);
	FogColor.Darker(0.3f);
	FUSIONCORE::SetEnvironment(*Shaders->DeferredPBRshader, 1.0f, FogColor.GetRGB(), FogColor.GetRGB());
	FUSIONCORE::UseShaderProgram(0);

	shaderPrep = []() {};

	IsFullScreen = false;

	DeltaTime = 0.0;
	StartTime = 0.0;
	Debug = false;

	pathtracer = std::make_shared<FUSIONCORE::PathTracer>(mode.width, mode.height, models, *Shaders);
	CurrentMousePos = { 0.0,0.0 };
	PrevMousePos = { 0.0,0.0 };
	AllowPathTrace = false;

	Speed = SPEED;

	SelectedOperation = ImGuizmo::OPERATION::TRANSLATE;
	IsSceneAltered = false;

	SelectedModel = nullptr;
	FOV = 45.0f;

	FUSIONUTIL::InitializeImguiGLFW(ApplicationWindow.GetWindow());

	OnSaveScreen = [&]() {
		auto OutPath = HandleSaveDialog("png");
		if (OutPath.empty()) return;
		FUSIONCORE::SaveFrameBufferImage(WindowSize.x, WindowSize.y, OutPath.c_str(), FF_ATTACHMENT_COLOR_ATTACHMENT0);
	};

	OnImportModel = [&]() {
		auto OutPath = HandleImportDialog("obj,fbx,gltf,glb");
		if (OutPath.empty()) return;
		ImportedModel.reset();
		try
		{
			ImportedModel = std::make_shared<FUSIONCORE::Model>(OutPath);
		}
		catch (const std::exception& e)
		{
			LOG_ERR(e.what());
			return;
		}
		if (!ImportedModel)
		{
			LOG_ERR("An unexpected error occurred while importing the model!");
			return;
		}
		models.clear();
		SelectedModel = nullptr;
		models.push_back({ ImportedModel.get(),nullptr });
		camera3d->Position = ImportedModel->GetTransformation().InitialObjectScales;
		camera3d->Orientation = glm::normalize(ImportedModel->GetTransformation().Position - camera3d->Position);
		pathtracer->ConstructBVH(models, *Shaders);
	};

	AdditionalFunctionality = [&]() {
		ImGui::SeparatorText("Scene Settings");
		if (ImGui::Button("Import HDRI"))
		{
			auto OutPath = HandleImportDialog("hdr");
			if (OutPath.empty()) return;
			FUSIONCORE::ImportCubeMap(OutPath.c_str(), HDRIsize, *cubemap, *Shaders);
			pathtracer->SetShouldRestart(true);
		}
		if (ImGui::BeginCombo("HDRI size", std::to_string(HDRIsize).c_str())) {
			if (ImGui::Selectable("512"))
			{
				HDRIsize = 512;
			}
			if (ImGui::Selectable("1024"))
			{
				HDRIsize = 1024;
			}
			if (ImGui::Selectable("2048"))
			{
				HDRIsize = 2048;
			}
			if (ImGui::Selectable("4096"))
			{
				HDRIsize = 4096;
			}
			ImGui::EndCombo();
		}
		if (ImGui::SliderFloat("FOV", &FOV, 0.1f, 360.0f)) pathtracer->SetShouldRestart(true);
		ImGui::SliderFloat("Camera Speed", &Speed, 0.0001f, 1000.0f);
	};
}

void PathTracingDemo::Run()
{
	auto window = ApplicationWindow.GetWindow();
	while (!ApplicationWindow.ShouldClose())
	{
		StartTime = FUSIONUTIL::GetTime();
		WindowSize = ApplicationWindow.GetWindowSize();
		FUSIONUTIL::CreateFrameImguiGLFW();
		ImGuizmo::BeginFrame();
		PathTraceComputeShader->HotReload(3000);

		PrevMousePos = CurrentMousePos;
		FUSIONUTIL::GetCursorPosition(window, CurrentMousePos.x, CurrentMousePos.y);
		FUSIONUTIL::GetWindowPosition(window, WindowPos.x, WindowPos.y);

		static bool AllowPressF = true;
		if (FUSIONUTIL::IsKeyPressedOnce(window, FF_KEY_F, AllowPressF))
		{
			IsFullScreen = !IsFullScreen;
			if (!IsFullScreen)
			{
				PrevWindowSize = WindowSize;
				FUSIONUTIL::SetWindowMonitor(window, FUSIONUTIL::GetPrimaryMonitor(), 0, 0, mode.width, mode.height, mode.refreshRate);
			}
			else
			{
				FUSIONUTIL::SetWindowMonitor(window, NULL, WindowPos.x, WindowPos.y, PrevWindowSize.x, PrevWindowSize.y, mode.refreshRate);
			}
		}

		camera3d->UpdateCameraMatrix(FOV, (float)WindowSize.x / (float)WindowSize.y, CAMERA_CLOSE_PLANE, 10000.0f, WindowSize);
		camera3d->HandleInputs(ApplicationWindow.GetWindow(), WindowSize, FF_CAMERA_LAYOUT_FIRST_PERSON, Speed * DeltaTime);

		static bool AllowPressR = true;
		if (FUSIONUTIL::IsKeyPressedOnce(window, FF_KEY_R, AllowPressR))
		{
			pathtracer->SetShouldRestart(true);
			AllowPathTrace = !AllowPathTrace;
		}

		if (!AllowPathTrace)
		{
			Gbuffer->Bind();
			Gbuffer->SetDrawModeDefault();
			FUSIONUTIL::ClearFrameBuffer(0, 0, WindowSize.x, WindowSize.y, FF_COLOR_VOID);

			for (auto& model : models)
			{
				model.first->DrawDeferredImportedMaterial(*camera3d, *Shaders->GbufferShader, []() {});
			}

			ScreenFrameBuffer->Bind();
			FUSIONUTIL::ClearFrameBuffer(0, 0, WindowSize.x, WindowSize.y, FF_COLOR_BLACK);
			Gbuffer->DrawSceneDeferred(*camera3d, *Shaders->DeferredPBRshader, [&]() {}, WindowSize, shadowMaps, *cubemap, FF_COLOR_VOID, 0.3f);

			if (FUSIONUTIL::GetMouseKey(window, FF_GLFW_MOUSE_BUTTON_RIGHT) == FF_GLFW_PRESS)
			{
				auto Pixel = FUSIONCORE::ReadFrameBufferPixel(CurrentMousePos.x, CurrentMousePos.y, FF_FRAMEBUFFER_MODEL_ID_IMAGE_ATTACHMENT, FF_PIXEL_FORMAT_GL_RED, { WindowSize.x, WindowSize.y });
				SelectedModel = FUSIONCORE::GetModel(Pixel.GetRed());
			}

			FUSIONUTIL::GLviewport(0, 0, WindowSize.x, WindowSize.y);
			auto gbufferSize = Gbuffer->GetFBOSize();
			FUSIONCORE::CopyDepthInfoFBOtoFBO(Gbuffer->GetFBO(), { gbufferSize.x ,gbufferSize.y }, ScreenFrameBuffer->GetFBO());
			ScreenFrameBuffer->Bind();
			for (size_t i = 0; i < Lights.size(); i++)
			{
				Lights[i].Draw(*camera3d, *Shaders->LightShader);
			}
			if (SelectedModel)
			{
				ImGui::SetNextWindowSize(ImVec2(mode.width, mode.height));
				ImGui::SetNextWindowPos(ImVec2(0, 0));

				ImGui::Begin("viewport", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);

				ImGui::SetCursorPosX(WindowSize.x - ImGui::CalcTextSize("Right Button").x - 10);
				if (ImGui::BeginCombo("Select an Operation", "Operation")) {
					if (ImGui::Selectable("Translate"))
					{
						SelectedOperation = ImGuizmo::OPERATION::TRANSLATE;
					}
					if (ImGui::Selectable("Rotate"))
					{
						SelectedOperation = ImGuizmo::OPERATION::ROTATE;
					}
					if (ImGui::Selectable("Scale"))
					{
						SelectedOperation = ImGuizmo::OPERATION::SCALE;
					}
					ImGui::EndCombo();
				}

				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();
				ImGuizmo::SetRect(0, 0, WindowSize.x, WindowSize.y);
				glm::mat4 SelectedModelTransformationMatrix = SelectedModel->GetTransformation().GetModelMat4();

				ImGuizmo::Manipulate(glm::value_ptr(camera3d->viewMat), glm::value_ptr(camera3d->projMat), SelectedOperation, ImGuizmo::LOCAL, glm::value_ptr(SelectedModelTransformationMatrix));

				if (ImGuizmo::IsUsing())
				{
					glm::vec3 Scale;
					glm::quat Rotation;
					glm::vec3 Translation;
					DecomposeMatrix(SelectedModelTransformationMatrix, Translation, Rotation, Scale);

					auto& SelectedModelTransformation = SelectedModel->GetTransformation();

					SelectedModelTransformation.RotationMatrix = glm::mat4_cast(Rotation);
					SelectedModelTransformation.Translate((Translation - SelectedModelTransformation.Position) / 100.0f);
					SelectedModelTransformation.ScalingMatrix = glm::scale(glm::mat4(1.0f), Scale);
					SelectedModelTransformation.ObjectScales = SelectedModelTransformation.InitialObjectScales * Scale;
					SelectedModelTransformation.ScaleFactor = Scale;
					SelectedModelTransformation.scale_avg = (SelectedModelTransformation.ObjectScales.x + SelectedModelTransformation.ObjectScales.y + SelectedModelTransformation.ObjectScales.z) / 3.0f;

					IsSceneAltered = true;
				}
				ImGui::End();
			}
			if (pathtracer->ShouldDisplayBVH()) pathtracer->VisualizeBVH(*camera3d, *BVHvisualizeShader, FF_COLOR_RED);
		}

		FUSIONUTIL::GLBindFrameBuffer(FF_GL_FRAMEBUFFER, 0);
		FUSIONUTIL::ClearFrameBuffer(0, 0, WindowSize.x, WindowSize.y, FF_COLOR_VOID);
		if (AllowPathTrace && !FUSIONUTIL::IsAnyItemActive())
		{
			pathtracer->Render(ApplicationWindow, *PathTraceComputeShader, *camera3d, cubemap.get());
		}

		GLuint ViewportImage = !AllowPathTrace ? ScreenFrameBuffer->GetFBOimage() : pathtracer->GetTracedImage();

		FUSIONCORE::DrawTextureOnQuad(ViewportImage, { 0,0 }, { mode.width,mode.height }, *camera3d, *Shaders->TextureOnQuadShader, 1.7f, 1.6f);
		pathtracer->PathTracerDashBoard(OnImportModel, OnSaveScreen, AdditionalFunctionality);
		FUSIONUTIL::RenderImguiGLFW();

		ApplicationWindow.UpdateWindow();
		DeltaTime = FUSIONUTIL::GetTime() - StartTime;
		if (IsSceneAltered && AllowPathTrace)
		{
			pathtracer->ConstructBVH(models, *Shaders);
			IsSceneAltered = false;
		}
	}
}
