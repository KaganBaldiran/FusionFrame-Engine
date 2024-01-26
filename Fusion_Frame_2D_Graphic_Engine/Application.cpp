#include "Application.hpp"
#include <glew.h>
#include <glfw3.h>
#include <iostream>
#include "FusionUtility/Log.h"
#include "FusionOpengl/Shader.h"
#include "FusionOpengl/Buffer.h"
#include "FusionUtility/VectorMath.h"
#include "FusionOpengl/Texture.h"
#include "FusionUtility/Initialize.h"
#include "FusionOpengl/Camera.h"
#include "FusionOpengl/Mesh.h"
#include "FusionOpengl/Model.hpp"
#include "FusionOpengl/Light.hpp"
#include "FusionOpengl/Framebuffer.hpp"
#include "FusionPhysics/Physics.hpp"
#include "FusionOpengl/Color.hpp"
#include "FusionUtility/StopWatch.h"
#include "FusionOpengl/Cubemap.h"
#include "FusionUtility/Thread.h"
#include "FusionPhysics/Octtree.hpp"
#include <chrono>
#include <thread>
#include <memory>

#define SPEED 1.0f

int Application::Run()
{
	const int width = 1000;
	const int height = 1000;

	GLFWwindow* window = FUSIONUTIL::InitializeWindow(width, height, "FusionFrame Engine");

	FUSIONUTIL::DefaultShaders Shaders;
	FUSIONUTIL::InitializeDefaultShaders(Shaders);

	FUSIONOPENGL::CubeMap cubemap(*Shaders.CubeMapShader);
	FUSIONOPENGL::ImportCubeMap("Resources/hayloft_2k.hdr", 1024, cubemap, Shaders.HDRIShader->GetID(), Shaders.ConvolutateCubeMapShader->GetID(), Shaders.PreFilterCubeMapShader->GetID());

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	FUSIONOPENGL::FrameBuffer ScreenFrameBuffer(mode->width, mode->height);

	FUSIONOPENGL::LightIcon = std::make_unique<FUSIONOPENGL::Model>("Resources/LightIcon.fbx");

	FUSIONOPENGL::Camera3D camera3d;

	FUSIONOPENGL::Texture2D ShovelDiffuse("Resources/texture_diffuse.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
	FUSIONOPENGL::Texture2D ShovelNormal("Resources/texture_normal.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
	FUSIONOPENGL::Texture2D ShovelSpecular("Resources/texture_specular.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);

	FUSIONOPENGL::Texture2D FloorSpecular("Resources/floor/diagonal_parquet_rough_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, false);
	FUSIONOPENGL::Texture2D FloorNormal("Resources/floor/diagonal_parquet_nor_dx_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, false);
	FUSIONOPENGL::Texture2D FloorAlbedo("Resources/floor/diagonal_parquet_diff_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, false);

	FUSIONOPENGL::Texture2D SofaDiffuse("Resources\\models\\sofa\\textures\\sofa_03_diff_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
	FUSIONOPENGL::Texture2D SofaNormal("Resources\\models\\sofa\\textures\\sofa_03_nor_dx_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
	FUSIONOPENGL::Texture2D SofaSpecular("Resources\\models\\sofa\\textures\\sofa_03_rough_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);

	FUSIONOPENGL::Texture2D MirrorDiffuse("Resources\\models\\stove\\textures\\electric_stove_diff_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
	FUSIONOPENGL::Texture2D MirrorNormal("Resources\\models\\stove\\textures\\electric_stove_nor_dx_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
	FUSIONOPENGL::Texture2D MirrorSpecular("Resources\\models\\stove\\textures\\electric_stove_rough_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
	FUSIONOPENGL::Texture2D MirrorMetalic("Resources\\models\\stove\\textures\\electric_stove_metal_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);

	FUSIONOPENGL::Texture2D WallDiffuse("Resources\\wall\\textures\\painted_plaster_wall_diff_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
	FUSIONOPENGL::Texture2D WallNormal("Resources\\wall\\textures\\painted_plaster_wall_nor_dx_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
	FUSIONOPENGL::Texture2D WallSpecular("Resources\\wall\\textures\\painted_plaster_wall_rough_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);

	Vec2<int> WindowSize;
	Vec2<double> mousePos;

	glm::vec3 Target(0.0f);

	camera3d.SetPosition(glm::vec3(12.353, 13.326, 15.2838));
	camera3d.SetOrientation(glm::vec3(-0.593494, -0.648119, -0.477182));

	auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::uniform_real_distribution<float> RandomFloats(0.0f, 20.0f);
	std::default_random_engine engine(seed);

	FUSIONOPENGL::Light light0({ RandomFloats(engine),RandomFloats(engine),RandomFloats(engine) }, FUSIONOPENGL::Color(FF_COLOR_CINNAMON).GetRGB(), 150.0f);
	FUSIONOPENGL::Light light1({ RandomFloats(engine),RandomFloats(engine),RandomFloats(engine) }, FUSIONOPENGL::Color(FF_COLOR_LIME).GetRGB(), 150.0f);
	FUSIONOPENGL::Light light2({ RandomFloats(engine),RandomFloats(engine),RandomFloats(engine) }, FUSIONOPENGL::Color(FF_COLOR_AMETHYST).GetRGB(), 150.0f);
	FUSIONOPENGL::Light light3({ RandomFloats(engine),RandomFloats(engine),RandomFloats(engine) }, FUSIONOPENGL::Color(FF_COLOR_LIME_SHERBET).GetRGB(), 150.0f);

	//FUSIONUTIL::ThreadPool threads(5, 20);
//#define ASYNC
#define NOTASYNC

#ifdef ASYNC
	FUSIONUTIL::Timer stopwatch;
	stopwatch.Set();
	std::unique_ptr<FUSIONOPENGL::Model> model0;
	std::unique_ptr<FUSIONOPENGL::Model> model1;
	std::unique_ptr<FUSIONOPENGL::Model> WateringPot;

	auto import1 = [&]() { model0 = std::make_unique<FUSIONOPENGL::Model>("Resources\\shovel2.obj", true); };
	auto import2 = [&]() { model1 = std::make_unique<FUSIONOPENGL::Model>("Resources\\shovel2.obj", true); };
	auto import3 = [&]() { WateringPot = std::make_unique<FUSIONOPENGL::Model>("Resources\\shovel2.obj", true); };

	std::vector<std::function<void()>> functions;
	functions.push_back(import1);
	functions.push_back(import2);
	functions.push_back(import3);

	threads.enqueue(import1);
	threads.enqueue(import2);
	threads.enqueue(import3);
	/*
	FUSIONUTIL::ExecuteFunctionsAsync(functions);

	// Wait for all shared_futures to finish
	for (auto& shared_future : FUSIONUTIL::shared_futures)
	{

		shared_future.wait();

	}*/

	threads.wait();

	model0->ConstructMeshes(model0->PreMeshDatas);
	model1->ConstructMeshes(model1->PreMeshDatas);
	WateringPot->ConstructMeshes(WateringPot->PreMeshDatas);

	LOG("Duration: " << stopwatch.returnMiliseconds());

#elif defined NOTASYNC
	FUSIONUTIL::Timer stopwatch;
	stopwatch.Set();
	std::unique_ptr<FUSIONOPENGL::Model> MainCharac;
	std::unique_ptr<FUSIONOPENGL::Model> model1;
	std::unique_ptr<FUSIONOPENGL::Model> Stove;
	std::unique_ptr<FUSIONOPENGL::Model> grid;
	std::unique_ptr<FUSIONOPENGL::Model> sofa;

	std::unique_ptr<FUSIONOPENGL::Model> wall;

	MainCharac = std::make_unique<FUSIONOPENGL::Model>("Resources\\shovel2.obj");
	model1 = std::make_unique<FUSIONOPENGL::Model>("Resources\\shovel2.obj");
	Stove = std::make_unique<FUSIONOPENGL::Model>("Resources\\models\\stove\\stove.obj");
	grid = std::make_unique<FUSIONOPENGL::Model>("Resources\\floor\\grid.obj");
	sofa = std::make_unique<FUSIONOPENGL::Model>("Resources\\models\\sofa\\model\\sofa.obj");

	wall = std::make_unique<FUSIONOPENGL::Model>("Resources\\floor\\grid.obj");

	LOG("Duration: " << stopwatch.GetMiliseconds());
#endif // DEBUG
	FUSIONOPENGL::Material shovelMaterial;
	shovelMaterial.PushTextureMap(TEXTURE_DIFFUSE0, ShovelDiffuse);
	shovelMaterial.PushTextureMap(TEXTURE_NORMAL0, ShovelNormal);
	shovelMaterial.PushTextureMap(TEXTURE_SPECULAR0, ShovelSpecular);

	FUSIONOPENGL::Material FloorMaterial;
	FloorMaterial.PushTextureMap(TEXTURE_DIFFUSE0, FloorAlbedo);
	FloorMaterial.PushTextureMap(TEXTURE_NORMAL0, FloorNormal);
	FloorMaterial.PushTextureMap(TEXTURE_SPECULAR0, FloorSpecular);
	FloorMaterial.SetTiling(3.0f);

	FUSIONOPENGL::Material SofaMaterial;
	SofaMaterial.PushTextureMap(TEXTURE_DIFFUSE0, SofaDiffuse);
	SofaMaterial.PushTextureMap(TEXTURE_NORMAL0, SofaNormal);
	SofaMaterial.PushTextureMap(TEXTURE_SPECULAR0, SofaSpecular);

	FUSIONOPENGL::Material MirrorMaterial;
	MirrorMaterial.PushTextureMap(TEXTURE_DIFFUSE0, MirrorDiffuse);
	MirrorMaterial.PushTextureMap(TEXTURE_NORMAL0, MirrorNormal);
	MirrorMaterial.PushTextureMap(TEXTURE_SPECULAR0, MirrorSpecular);
	MirrorMaterial.PushTextureMap(TEXTURE_METALIC0, MirrorMetalic);

	FUSIONOPENGL::Material WallMaterial;
	WallMaterial.PushTextureMap(TEXTURE_DIFFUSE0, WallDiffuse);
	WallMaterial.PushTextureMap(TEXTURE_NORMAL0, WallNormal);
	WallMaterial.PushTextureMap(TEXTURE_SPECULAR0, WallSpecular);
	WallMaterial.SetTiling(2.0f);

	model1->GetTransformation().TranslateNoTraceBack({ 0.0f,0.0f,10.0f });
	model1->GetTransformation().ScaleNoTraceBack(glm::vec3(0.15f, 0.15f, 0.15f));
	MainCharac->GetTransformation().ScaleNoTraceBack(glm::vec3(0.15f, 0.15f, 0.15f));
	MainCharac->GetTransformation().RotateNoTraceBack(glm::vec3(0.0f, 1.0f, 0.0f), 90.0f);
	MainCharac->GetTransformation().TranslateNoTraceBack({ 4.0f,1.0f,-10.0f });

	Stove->GetTransformation().ScaleNoTraceBack(glm::vec3(7.0f, 7.0f, 7.0f));
	Stove->GetTransformation().TranslateNoTraceBack({ 0.0f,4.0f,30.0f });
	//Stove->GetTransformation().RotateNoTraceBack(glm::vec3(0.0f, 1.0f, 0.0f), 70.0f);

	wall->GetTransformation().ScaleNoTraceBack(glm::vec3(5.0f, 5.0f, 5.0f));
	wall->GetTransformation().TranslateNoTraceBack({ -60.0f,10.0f,0.0f });
	wall->GetTransformation().RotateNoTraceBack(glm::vec3(0.0f, 0.0f, 1.0f), 90.0f);

	sofa->GetTransformation().ScaleNoTraceBack(glm::vec3(13.0f, 13.0f, 13.0f));
	sofa->GetTransformation().RotateNoTraceBack(glm::vec3(0.0f, 1.0f, 0.0f), 90.0f);
	sofa->GetTransformation().TranslateNoTraceBack({ -10.0f,-1.0f,-5.0f });


	FUSIONPHYSICS::CollisionBox3DAABB Box1(model1->GetTransformation(), { 1.0f,1.0f,1.0f });
	FUSIONPHYSICS::CollisionBox3DAABB Box0(MainCharac->GetTransformation(), { 1.0f,1.0f,1.0f });
	FUSIONPHYSICS::CollisionBox3DAABB WateringPotBox(Stove->GetTransformation(), { 1.0f,1.0f,1.0f });
	FUSIONPHYSICS::CollisionBox3DAABB tryBox({ 1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f });
	FUSIONPHYSICS::CollisionBoxPlane Plane({ 1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f });
	FUSIONPHYSICS::CollisionBox3DAABB SofaBox(sofa->GetTransformation(), { 0.63f,1.2f,1.0f });
	FUSIONPHYSICS::CollisionBoxPlane Plane2({ 1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f });
	FUSIONPHYSICS::CollisionBoxPlane floorBox({ 1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f });

	Plane.GetTransformation().Scale({ 5.0f,5.0f ,5.0f });
	Plane2.GetTransformation().Scale({ 2.0f,2.0f ,2.0f });
	Plane2.GetTransformation().Translate({ 0.7f,0.0f,0.0f });

	floorBox.GetTransformation().Scale({ 120.0f,120.0f ,120.0f });
	floorBox.GetTransformation().Translate({ 0.0f,-1.0f,0.0f });

	grid->GetTransformation().ScaleNoTraceBack({ 5.0f,5.0f ,5.0f });
	grid->GetTransformation().TranslateNoTraceBack({ 0.0f,-1.0f,0.0f });


	MainCharac->PushChild(&Box0);
	model1->PushChild(&Box1);
	sofa->PushChild(&SofaBox);

	MainCharac->UpdateChildren();

	Stove->PushChild(&WateringPotBox);
	Stove->UpdateChildren();

	glm::vec4 BackGroundColor(175.0f / 255.0f, 225.0f / 255.0f, 225.0f / 255.0f, 1.0f);

	Shaders.PBRShader->use();

	FUSIONOPENGL::SetEnvironmentIBL(*Shaders.PBRShader, 3.0f, glm::vec3(BackGroundColor.x, BackGroundColor.y, BackGroundColor.z));

	FUSIONOPENGL::UseShaderProgram(0);

	const double TARGET_FRAME_TIME = 1.0 / TARGET_FPS;
	auto startTimePerSecond = std::chrono::high_resolution_clock::now();
	int fpsCounter = 0;

	glm::vec3 translateVector(0.0f, 0.0f, 0.01f);

	Vec2<int> PrevWindowSize;
	Vec2<int> PrevWindowPos;
	bool IsFullScreen = false;
	float AOamount = 0.5f;


	bool showDebug = false;

	while (!glfwWindowShouldClose(window))
	{
		auto start_time = std::chrono::high_resolution_clock::now();



		ScreenFrameBuffer.Bind();
		glClearColor(BackGroundColor.x, BackGroundColor.y, BackGroundColor.z, BackGroundColor.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwGetWindowSize(window, &WindowSize.x, &WindowSize.y);
		glViewport(0, 0, WindowSize.x, WindowSize.y);

		glfwGetCursorPos(window, &mousePos.x, &mousePos.y);
		Target = { mousePos.x / WindowSize.x , -mousePos.y / WindowSize.y, 0.0f };

		camera3d.UpdateCameraMatrix(45.0f, (float)WindowSize.x / (float)WindowSize.y, 0.1f, 180.0f, WindowSize);
		camera3d.SetTarget(&*MainCharac, 20.0f);
		camera3d.HandleInputs(window, WindowSize, FF_CAMERA_LAYOUT_INDUSTRY_STANDARD, 0.06f);

		std::function<void()> shaderPrepe = [&]() {};
		std::function<void()> shaderPrepe1 = [&]() {};
		std::function<void()> shaderPrepe2 = [&]() {};

		tryBox.GetTransformation().Translate({ 0.0f,0.0f,std::sin(time(0)) / 5.0f });
		tryBox.UpdateAttributes();

		Plane.GetTransformation().Rotate({ 0.0f,1.0f,0.0f }, std::sin(time(0)));
		Plane.GetTransformation().Translate({ 0.0f,std::sin(time(0)) / 10.0f,0.0f });
		Plane.UpdateAttributes();

		Stove->Draw(camera3d, *Shaders.PBRShader, shaderPrepe, cubemap, MirrorMaterial, AOamount);
		grid->Draw(camera3d, *Shaders.PBRShader, shaderPrepe, cubemap, FloorMaterial, AOamount);
		sofa->Draw(camera3d, *Shaders.PBRShader, shaderPrepe, cubemap, SofaMaterial, AOamount);
		wall->Draw(camera3d, *Shaders.PBRShader, shaderPrepe, cubemap, WallMaterial, AOamount);

		cubemap.Draw(camera3d, WindowSize.Cast<float>());
		Stove->UpdateChildren();
		sofa->UpdateChildren();

#ifdef ENGINE_DEBUG

		static bool AllowD = true;
		if (IsKeyPressedOnce(window, GLFW_KEY_D, AllowD))
		{
			showDebug = !showDebug;
		}
		if (showDebug)
		{
			light0.Draw(camera3d, *Shaders.LightShader);
			light1.Draw(camera3d, *Shaders.LightShader);
			light2.Draw(camera3d, *Shaders.LightShader);
			light3.Draw(camera3d, *Shaders.LightShader);
			Box0.DrawBoxMesh(camera3d, *Shaders.LightShader);
			Box1.DrawBoxMesh(camera3d, *Shaders.LightShader);
			SofaBox.DrawBoxMesh(camera3d, *Shaders.LightShader);
			WateringPotBox.DrawBoxMesh(camera3d, *Shaders.LightShader);
			tryBox.DrawBoxMesh(camera3d, *Shaders.LightShader);
			Plane.DrawBoxMesh(camera3d, *Shaders.LightShader);
			Plane2.DrawBoxMesh(camera3d, *Shaders.LightShader);
			floorBox.DrawBoxMesh(camera3d, *Shaders.LightShader);
		}
#endif

		model1->GetTransformation().Rotate({ 0.0f,1.0f,0.0f }, std::sin(time(0)));
		model1->UpdateChildren();
		MainCharac->UpdateChildren();

		bool Collision = false;
		glm::vec3 direction;
		if (FUSIONPHYSICS::IsCollidingSAT(tryBox, Box0))
		{
			Collision = true;
			direction = FUSIONPHYSICS::CheckCollisionDirection(tryBox.GetTransformation().Position - Box0.GetTransformation().Position, tryBox.GetTransformation().GetModelMat4()).second;
		}
		if (FUSIONPHYSICS::IsCollidingSAT(Box1, Box0))
		{
			Collision = true;
			direction = FUSIONPHYSICS::CheckCollisionDirection(Box1.GetTransformation().Position - Box0.GetTransformation().Position, Box1.GetTransformation().GetModelMat4()).second;
		}
		if (FUSIONPHYSICS::IsCollidingSAT(WateringPotBox, Box0))
		{
			Collision = true;
			direction = FUSIONPHYSICS::CheckCollisionDirection(WateringPotBox.GetTransformation().Position - Box0.GetTransformation().Position, WateringPotBox.GetTransformation().GetModelMat4()).second;
		}
		if (FUSIONPHYSICS::IsCollidingSAT(SofaBox, Box0))
		{
			Collision = true;
			direction = FUSIONPHYSICS::CheckCollisionDirection(SofaBox.GetTransformation().Position - Box0.GetTransformation().Position, SofaBox.GetTransformation().GetModelMat4()).second;
		}

		static bool FirstFloorTouch = true;
		if (!FUSIONPHYSICS::IsCollidingSAT(floorBox, Box0))
		{
			//direction = FUSIONPHYSICS::CheckCollisionDirection(Box0.GetTransformation().Position - SofaBox.GetTransformation().Position, Box0.GetTransformation().GetModelMat4()).second;
			if (FirstFloorTouch)
			{
				MainCharac->GetTransformation().Translate({ 0.0f,-0.01f,0.0f });
			}
			else
			{
				MainCharac->GetTransformation().Translate({ 0.0f,-0.3f,0.0f });
			}
		}
		else
		{
			if (FirstFloorTouch)
			{
				FirstFloorTouch = false;
			}
		}

		//LOG("dIRECTION: " << Vec3<float>(direction));

		if (Collision)
		{
			MainCharac->GetTransformation().Translate(-direction * 0.4f);
		}
		else
		{
			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			{
				MainCharac->GetTransformation().Translate(glm::vec3(camera3d.Orientation.x, 0.0f, camera3d.Orientation.z) * SPEED);
			}
			if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			{
				MainCharac->GetTransformation().Translate(-glm::vec3(camera3d.Orientation.x, 0.0f, camera3d.Orientation.z) * SPEED);
			}
			if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			{
				MainCharac->GetTransformation().Translate(glm::cross(camera3d.Orientation, camera3d.GetUpVector()) * SPEED);
			}
			if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			{
				MainCharac->GetTransformation().Translate(-glm::cross(camera3d.Orientation, camera3d.GetUpVector()) * SPEED);
			}

		}
		static bool AllowJump = false;
		static bool AllowReset = true;
		if (!AllowReset && stopwatch.GetSeconds() >= 2.0f)
		{
			AllowJump = false;
			AllowReset = true;
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !AllowJump && AllowReset)
		{
			AllowJump = true;
			AllowReset = false;
		}
		if (AllowReset)
		{
			stopwatch.Reset();
		}
		if (AllowJump)
		{
			if (stopwatch.GetSeconds() >= 1.0f)
			{
				AllowJump = false;
			}
			MainCharac->GetTransformation().Translate(camera3d.GetUpVector() * SPEED);
		}


		static bool AllowPressF = true;
		if (!AllowPressF && glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)
		{
			AllowPressF = true;
		}
		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && AllowPressF)
		{
			AllowPressF = false;
			IsFullScreen = !IsFullScreen;
			if (!IsFullScreen)
			{
				glfwGetWindowPos(window, &PrevWindowPos.x, &PrevWindowPos.y);
				PrevWindowSize(WindowSize);
				const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
				glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
			}
			else
			{
				const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
				glfwSetWindowMonitor(window, NULL, PrevWindowPos.x, PrevWindowPos.y, PrevWindowSize.x, PrevWindowSize.y, mode->refreshRate);
			}

		}

		model1->Draw(camera3d, *Shaders.PBRShader, shaderPrepe, cubemap, shovelMaterial, AOamount);
		MainCharac->Draw(camera3d, *Shaders.PBRShader, shaderPrepe, cubemap, shovelMaterial, AOamount);

		ScreenFrameBuffer.Unbind();
		ScreenFrameBuffer.Draw(camera3d, *Shaders.FBOShader, [&]() {}, WindowSize, true, 0.5f, 2.0f);

		glfwPollEvents();
		glfwSwapBuffers(window);

		auto end_time = std::chrono::high_resolution_clock::now();
		auto elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count() / 1e6;

		if (elapsed_time < TARGET_FRAME_TIME) {

			std::this_thread::sleep_for(std::chrono::microseconds(static_cast<long long>((TARGET_FRAME_TIME - elapsed_time) * 1e6)));
		}

		fpsCounter++;
		auto overAllElapsedPerSecond = std::chrono::duration_cast<std::chrono::seconds>(end_time - startTimePerSecond).count();

		if (overAllElapsedPerSecond >= 1.0)
		{
			double fps = fpsCounter / overAllElapsedPerSecond;
			fpsCounter = 0;
			startTimePerSecond = std::chrono::high_resolution_clock::now();
		}
	}

	FUSIONUTIL::DisposeDefaultShaders(Shaders);

	ScreenFrameBuffer.clean();
	Box0.GetBoxMesh()->Clean();
	Box1.GetBoxMesh()->Clean();
	WateringPotBox.GetBoxMesh()->Clean();
	tryBox.GetBoxMesh()->Clean();
	Plane.GetBoxMesh()->Clean();
	Plane2.GetBoxMesh()->Clean();
	SofaBox.GetBoxMesh()->Clean();
	floorBox.GetBoxMesh()->Clean();

	shovelMaterial.Clear();
	FloorMaterial.Clear();
	SofaMaterial.Clear();
	MirrorMaterial.Clear();
	WallMaterial.Clear();

	glfwTerminate();
	LOG_INF("Window terminated!");
	return 0;
}

bool Application::IsKeyPressedOnce(GLFWwindow* window, int Key, bool& Signal)
{
	if (!Signal && glfwGetKey(window, Key) == GLFW_RELEASE)
	{
		Signal = true;
	}
	if (glfwGetKey(window, Key) == GLFW_PRESS && Signal)
	{
		Signal = false;
		return true;
	}
	return false;
}


