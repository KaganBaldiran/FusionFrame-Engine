#include "Application.hpp"
#include "FusionFrame.h"
#include <glew.h>
#include <glfw3.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>

#define SPEED 0.5f
const float BlendAmount = 0.04f;
const float InterBlendAmount = 0.04f;

int Application::Run()
{
	const int width = 1000;
	const int height = 1000;

	GLFWwindow* window = FUSIONUTIL::InitializeWindow(width, height, "FusionFrame Engine");
	
	FUSIONUTIL::DefaultShaders Shaders;
	FUSIONUTIL::InitializeDefaultShaders(Shaders);

	FUSIONCORE::CubeMap cubemap(*Shaders.CubeMapShader);
	FUSIONCORE::ImportCubeMap("Resources/hayloft_2k.hdr", 1024, cubemap, Shaders.HDRIShader->GetID(), Shaders.ConvolutateCubeMapShader->GetID(), Shaders.PreFilterCubeMapShader->GetID());

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	FUSIONCORE::Gbuffer Gbuffer(mode->width, mode->height);

	FUSIONCORE::FrameBuffer ScreenFrameBuffer(mode->width, mode->height);


	FUSIONCORE::LightIcon = std::make_unique<FUSIONCORE::Model>("Resources/LightIcon.fbx");

	FUSIONCORE::Camera3D camera3d;

	FUSIONCORE::Texture2D ShovelDiffuse("Resources/texture_diffuse.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
	FUSIONCORE::Texture2D ShovelNormal("Resources/texture_normal.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
	FUSIONCORE::Texture2D ShovelSpecular("Resources/texture_specular.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);

	FUSIONCORE::Texture2D FloorSpecular("Resources/floor/diagonal_parquet_rough_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, false);
	FUSIONCORE::Texture2D FloorNormal("Resources/floor/diagonal_parquet_nor_dx_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, false);
	FUSIONCORE::Texture2D FloorAlbedo("Resources/floor/diagonal_parquet_diff_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, false);

	FUSIONCORE::Texture2D SofaDiffuse("Resources\\models\\sofa\\textures\\sofa_03_diff_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
	FUSIONCORE::Texture2D SofaNormal("Resources\\models\\sofa\\textures\\sofa_03_nor_dx_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
	FUSIONCORE::Texture2D SofaSpecular("Resources\\models\\sofa\\textures\\sofa_03_rough_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);

	FUSIONCORE::Texture2D MirrorDiffuse("Resources\\models\\stove\\textures\\electric_stove_diff_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
	FUSIONCORE::Texture2D MirrorNormal("Resources\\models\\stove\\textures\\electric_stove_nor_dx_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
	FUSIONCORE::Texture2D MirrorSpecular("Resources\\models\\stove\\textures\\electric_stove_rough_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
	FUSIONCORE::Texture2D MirrorMetalic("Resources\\models\\stove\\textures\\electric_stove_metal_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);

	FUSIONCORE::Texture2D WallDiffuse("Resources\\wall\\textures\\painted_plaster_wall_diff_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
	FUSIONCORE::Texture2D WallNormal("Resources\\wall\\textures\\painted_plaster_wall_nor_dx_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
	FUSIONCORE::Texture2D WallSpecular("Resources\\wall\\textures\\painted_plaster_wall_rough_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);

	FUSIONCORE::Texture2D bearDiffuse("Resources\\taunt\\textures\\bear_diffuse.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
	FUSIONCORE::Texture2D bearNormal("Resources\\taunt\\textures\\bear_normal.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
	FUSIONCORE::Texture2D bearSpecular("Resources\\taunt\\textures\\bear_specular.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);

	int shadowMapSize = 512;

	FUSIONCORE::OmniShadowMap ShadowMap0(shadowMapSize, shadowMapSize, 75.0f);
	FUSIONCORE::OmniShadowMap ShadowMap1(shadowMapSize, shadowMapSize, 75.0f);
	FUSIONCORE::OmniShadowMap ShadowMap2(shadowMapSize, shadowMapSize, 75.0f);
	FUSIONCORE::OmniShadowMap ShadowMap3(shadowMapSize, shadowMapSize, 75.0f);

	Vec2<int> WindowSize;
	Vec2<double> mousePos;

	glm::vec3 Target(0.0f);

	camera3d.SetPosition(glm::vec3(12.353, 13.326, 15.2838));
	camera3d.SetOrientation(glm::vec3(-0.593494, -0.648119, -0.477182));

	auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::uniform_real_distribution<float> RandomFloats(-50.0f, 50.0f);
	std::uniform_real_distribution<float> RandomFloatsY(0.0f, 30.0f);
	std::uniform_real_distribution<float> RandomColor(0.0f, 1.0f);
	std::uniform_real_distribution<float> RandomIntensity(180.0f, 250.0f);
	std::default_random_engine engine(seed);

	std::vector<FUSIONCORE::Light> Lights;

	for (size_t i = 0; i < 60; i++)
	{
		Lights.emplace_back(glm::vec3(RandomFloats(engine), RandomFloatsY(engine), RandomFloats(engine)), glm::vec3(RandomColor(engine), RandomColor(engine), RandomColor(engine)), RandomIntensity(engine));
	}

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
	std::unique_ptr<FUSIONCORE::Model> MainCharac;
	std::unique_ptr<FUSIONCORE::Model> model1;
	std::unique_ptr<FUSIONCORE::Model> Stove;
	std::unique_ptr<FUSIONCORE::Model> grid;
	std::unique_ptr<FUSIONCORE::Model> sofa;

	std::unique_ptr<FUSIONCORE::Model> wall;

	MainCharac = std::make_unique<FUSIONCORE::Model>("Resources\\shovel2.obj");
	model1 = std::make_unique<FUSIONCORE::Model>("Resources\\shovel2.obj");
	Stove = std::make_unique<FUSIONCORE::Model>("Resources\\models\\stove\\stove.obj");
	//Stove = std::make_unique<FUSIONOPENGL::Model>("Resources\\shovel2.obj");

	grid = std::make_unique<FUSIONCORE::Model>("Resources\\floor\\grid.obj");
	sofa = std::make_unique<FUSIONCORE::Model>("Resources\\models\\sofa\\model\\sofa.obj");

	wall = std::make_unique<FUSIONCORE::Model>("Resources\\floor\\grid.obj");

	FUSIONCORE::Model subdModel("Resources\\subDModel.obj");
	subdModel.GetTransformation().ScaleNoTraceBack({ 7.0f,7.0f,7.0f });
	subdModel.GetTransformation().TranslateNoTraceBack({ 19.0f,4.0f,0.0f });

	FUSIONPHYSICS::MESHOPERATIONS::LoopSubdivision(subdModel.Meshes[0], 2);

	/*for (size_t i = 0; i < wall->Meshes.size(); i++)
	{
		wall->Meshes[i].ConstructHalfEdges();
	}*/
	LOG("Duration: " << stopwatch.GetMiliseconds());
#endif // DEBUG
	FUSIONCORE::Material shovelMaterial;
	shovelMaterial.PushTextureMap(TEXTURE_DIFFUSE0, ShovelDiffuse);
	shovelMaterial.PushTextureMap(TEXTURE_NORMAL0, ShovelNormal);
	shovelMaterial.PushTextureMap(TEXTURE_SPECULAR0, ShovelSpecular);

	FUSIONCORE::Material FloorMaterial;
	FloorMaterial.PushTextureMap(TEXTURE_DIFFUSE0, FloorAlbedo);
	FloorMaterial.PushTextureMap(TEXTURE_NORMAL0, FloorNormal);
	FloorMaterial.PushTextureMap(TEXTURE_SPECULAR0, FloorSpecular);
	FloorMaterial.SetTiling(3.0f);

	FUSIONCORE::Material SofaMaterial;
	SofaMaterial.PushTextureMap(TEXTURE_DIFFUSE0, SofaDiffuse);
	SofaMaterial.PushTextureMap(TEXTURE_NORMAL0, SofaNormal);
	SofaMaterial.PushTextureMap(TEXTURE_SPECULAR0, SofaSpecular);

	FUSIONCORE::Material MirrorMaterial;
	MirrorMaterial.PushTextureMap(TEXTURE_DIFFUSE0, MirrorDiffuse);
	MirrorMaterial.PushTextureMap(TEXTURE_NORMAL0, MirrorNormal);
	MirrorMaterial.PushTextureMap(TEXTURE_SPECULAR0, MirrorSpecular);
	MirrorMaterial.PushTextureMap(TEXTURE_METALIC0, MirrorMetalic);

	FUSIONCORE::Material WallMaterial;
	WallMaterial.PushTextureMap(TEXTURE_DIFFUSE0, WallDiffuse);
	WallMaterial.PushTextureMap(TEXTURE_NORMAL0, WallNormal);
	WallMaterial.PushTextureMap(TEXTURE_SPECULAR0, WallSpecular);
	WallMaterial.SetTiling(2.0f);

	FUSIONCORE::Material AnimationModelMaterial;
	AnimationModelMaterial.PushTextureMap(TEXTURE_DIFFUSE0, bearDiffuse);
	AnimationModelMaterial.PushTextureMap(TEXTURE_NORMAL0, bearNormal);
	AnimationModelMaterial.roughness = 0.8f;

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
	sofa->GetTransformation().RotateNoTraceBack(glm::vec3(0.0f, 1.0f, 0.0f), 300.0f);
	sofa->GetTransformation().TranslateNoTraceBack({ -10.0f,-1.0f,-20.0f });

	FUSIONPHYSICS::CollisionBox3DAABB Box1(model1->GetTransformation(), { 1.0f,1.0f,1.0f });
	FUSIONPHYSICS::CollisionBox3DAABB StoveBox(Stove->GetTransformation(), { 1.0f,1.1f,1.0f });
	FUSIONPHYSICS::CollisionBox3DAABB tryBox({ 1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f });
	FUSIONPHYSICS::CollisionBoxPlane Plane({ 1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f });
	FUSIONPHYSICS::CollisionBox3DAABB SofaBox(sofa->GetTransformation(), { 0.7f,0.8f,1.0f });
	FUSIONPHYSICS::CollisionBoxPlane Plane2({ 1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f });
	FUSIONPHYSICS::CollisionBoxPlane floorBox({ 1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f });

	SofaBox.GetTransformation().Translate({ 0.0f,-1.0f,0.0f });

	Plane.GetTransformation().Scale({ 5.0f,5.0f ,5.0f });
	Plane2.GetTransformation().Scale({ 2.0f,2.0f ,2.0f });
	Plane2.GetTransformation().Translate({ 0.7f,0.0f,0.0f });

	floorBox.GetTransformation().Scale({ 135.0f,135.0f ,135.0f });
	floorBox.GetTransformation().Translate({ 0.0f,-1.0f,0.0f });

	grid->GetTransformation().ScaleNoTraceBack({ 5.0f,5.0f ,5.0f });
	grid->GetTransformation().TranslateNoTraceBack({ 0.0f,-1.0f,0.0f });


	model1->PushChild(&Box1);
	sofa->PushChild(&SofaBox);
	sofa->UpdateChildren();

	MainCharac->UpdateChildren();

	Stove->PushChild(&StoveBox);
	Stove->UpdateChildren();

	StoveBox.GetTransformation().ScaleNoTraceBack({ 0.5f,0.9f ,1.0f });


	glm::vec4 BackGroundColor(FUSIONCORE::Color(FF_COLOR_AZURE).GetRGBA());

	Shaders.DeferredPBRshader->use();

	FUSIONCORE::SetEnvironmentIBL(*Shaders.DeferredPBRshader, 1.0f, glm::vec3(BackGroundColor.x, BackGroundColor.y, BackGroundColor.z));
	FUSIONCORE::UseShaderProgram(0);

	const double TARGET_FRAME_TIME = 1.0 / TARGET_FPS;
	auto startTimePerSecond = std::chrono::high_resolution_clock::now();
	int fpsCounter = 0;

	glm::vec3 translateVector(0.0f, 0.0f, 0.01f);

	Vec2<int> PrevWindowSize;
	Vec2<int> PrevWindowPos;
	bool IsFullScreen = false;
	float AOamount = 0.5f;

	bool showDebug = false;

	glm::vec3 originalVector(-1.47019e-07, 0, -1);
	glm::vec3 normalizedVector = glm::normalize(originalVector);

	std::cout << "Original Vector: " << Vec3<float>(originalVector) << std::endl;
	std::cout << "Normalized Vector: " << Vec3<float>(normalizedVector) << std::endl;

	std::vector<FUSIONCORE::Model*> models;
	models.push_back(Stove.get());
	models.push_back(sofa.get());
	models.push_back(model1.get());
	models.push_back(grid.get());
	models.push_back(MainCharac.get());

	//cubemap.SetCubeMapTexture(ShadowMap0.GetShadowMap());

	std::vector<FUSIONCORE::OmniShadowMap*> shadowMaps;
	shadowMaps.push_back(&ShadowMap0);
	shadowMaps.push_back(&ShadowMap1);
	shadowMaps.push_back(&ShadowMap2);
	shadowMaps.push_back(&ShadowMap3);

	FUSIONCORE::Model animationModel("Resources\\taunt\\Jumping.fbx", false, true);
	animationModel.GetTransformation().ScaleNoTraceBack({ 0.1f,0.1f,0.1f });
	animationModel.GetTransformation().TranslateNoTraceBack({ 15.0f,-1.0f,0.0f });
	FUSIONCORE::Animation WalkingAnimation("Resources\\taunt\\MutantWalk.fbx", &animationModel);
	FUSIONCORE::Animation IdleAnimation("Resources\\taunt\\OrcIdle.fbx", &animationModel);
	FUSIONCORE::Animation JumpingAnimation("Resources\\taunt\\Jumping.fbx", &animationModel);
	FUSIONCORE::Animation RunningAnimation("Resources\\taunt\\Running.fbx", &animationModel);
	FUSIONCORE::Animator animator(&IdleAnimation);

	FUSIONPHYSICS::CollisionBox3DAABB Box0(animationModel.GetTransformation(), { 0.4f,0.9f,1.0f });
	animationModel.PushChild(&Box0);
	Box0.GetTransformation().TranslateNoTraceBack({ 0.0f,-1.0f,0.0f });
	models.push_back(&animationModel);

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;
	
	glm::vec3 AnimationModelInterpolationDirection = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 AnimationModelDirection = glm::vec3(0.0f, 0.0f, 1.0f);
	bool Moving = false;
	int WalkingSide = -1;
	float IdleWalkingBlendCoeff = 0.0f;
	float InterpolatingBlendCoeff = 0.0f;

	float JumpingBlendCoeff = 0.0f;

	float PreviousZoom = 0.0f;

	camera3d.SetMinMaxZoom(true,-6.0f, 6.0f);
	camera3d.SetZoomSensitivity(3.0f);

	/*for (size_t mesh = 0; mesh < grid->Meshes.size(); mesh++)
	{
		auto& vertices = grid->Meshes[mesh].GetVertices();
		for (size_t face = 0; face < vertices.size(); face++)
		{
			LOG(face << " use count : " << vertices[face].use_count());
			
		}
	}*/

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		auto start_time = std::chrono::high_resolution_clock::now();

		tryBox.GetTransformation().Translate({ 0.0f,0.0f,std::sin(time(0)) / 5.0f });
		tryBox.UpdateAttributes();

		Plane.GetTransformation().Rotate({ 0.0f,1.0f,0.0f }, std::sin(time(0)));
		Plane.GetTransformation().Translate({ 0.0f,std::sin(time(0)) / 10.0f,0.0f });
		Plane.UpdateAttributes();

		Stove->UpdateChildren();
		SofaBox.UpdateAttributes();

		model1->GetTransformation().Rotate({ 0.0f,1.0f,0.0f }, std::sin(time(0)));
		model1->UpdateChildren();

		animationModel.UpdateChildren();

		bool Collision = false;
		glm::vec3 direction;

		/*if (FUSIONPHYSICS::IsCollidingSAT(tryBox, Box0))
		{
			LOG("tryBox");
			Collision = true;
			direction = FUSIONPHYSICS::CheckCollisionDirection(Box0.GetTransformation().Position - tryBox.GetTransformation().Position, tryBox.GetTransformation().GetModelMat4()).second;
		}*/
		if (FUSIONPHYSICS::IsCollidingSAT(SofaBox, Box0))
		{
			LOG("sofa");
			Collision = true;
			direction = FUSIONPHYSICS::CheckCollisionDirection(Box0.GetTransformation().Position - SofaBox.GetTransformation().Position, Box0.GetTransformation().GetModelMat4()).second;
		}
		if (FUSIONPHYSICS::IsCollidingSAT(Box1, Box0))
		{
			LOG("Box1");
			Collision = true;
			direction = FUSIONPHYSICS::CheckCollisionDirection(Box0.GetTransformation().Position - Box1.GetTransformation().Position, Box0.GetTransformation().GetModelMat4()).second;
		}
		if (FUSIONPHYSICS::IsCollidingSAT(StoveBox, Box0))
		{
			LOG("StoveBox");
			Collision = true;
			direction = FUSIONPHYSICS::CheckCollisionDirection(Box0.GetTransformation().Position - StoveBox.GetTransformation().Position, Box0.GetTransformation().GetModelMat4()).second;
		}

		static bool FirstFloorTouch = true;
		if (!FUSIONPHYSICS::IsCollidingSAT(floorBox, Box0))
		{
			if (FirstFloorTouch)
			{
				animationModel.GetTransformation().Translate({ 0.0f,-0.01f,0.0f });
			}
			else
			{
				animationModel.GetTransformation().Translate({ 0.0f,-0.3f,0.0f });
			}
		}
		else
		{
			if (FirstFloorTouch)
			{
				FirstFloorTouch = false;
			}
		}


		auto Front = glm::vec3(camera3d.Orientation.x, 0.0f, camera3d.Orientation.z);
		auto Back = -Front;
		auto Right = glm::normalize(glm::cross(Front, camera3d.GetUpVector()));
		auto Left = -Right;

		if (Collision)
		{
			if (glm::dot(-direction, camera3d.GetUpVector()) < glm::epsilon<float>())
			{
				if (FirstFloorTouch)
				{
					animationModel.GetTransformation().Translate({ 0.0f,0.01f,0.0f });
				}
				else
				{
					animationModel.GetTransformation().Translate({ 0.0f,0.3f,0.0f });
				}
			}

			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && glm::dot(-direction, Front) <= glm::epsilon<float>())
			{
				Moving = true;
				animationModel.GetTransformation().Translate(Front * SPEED);
			}
			else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && glm::dot(-direction, Back) <= glm::epsilon<float>())
			{
				Moving = true;
				animationModel.GetTransformation().Translate(Back * SPEED);
			}
			else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && glm::dot(-direction, Right) <= glm::epsilon<float>())
			{
				Moving = true;
				animationModel.GetTransformation().Translate(Right * SPEED);
			}
			else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && glm::dot(-direction, Left) <= glm::epsilon<float>())
			{
				Moving = true;
				animationModel.GetTransformation().Translate(Left * SPEED);
			}
			else
			{
				Moving = false;
			}
		}
		else
		{
			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			{
				if (WalkingSide != 0)
				{
					AnimationModelInterpolationDirection = AnimationModelDirection;
					InterpolatingBlendCoeff = 0.0f;
				}
				WalkingSide = 0;
				Moving = true;
				AnimationModelDirection = glm::normalize(glm::mix(AnimationModelInterpolationDirection, { Front.x , 0.0f , -Front.z }, InterpolatingBlendCoeff));
				animationModel.GetTransformation().RotationMatrix = glm::lookAt(glm::vec3(0.0f), AnimationModelDirection, camera3d.GetUpVector());
				animationModel.GetTransformation().Translate(Front * SPEED);
			}
			else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			{
				if (WalkingSide != 1)
				{
					AnimationModelInterpolationDirection = AnimationModelDirection;
					InterpolatingBlendCoeff = 0.0f;
				}
				WalkingSide = 1;
				Moving = true;
				AnimationModelDirection = glm::normalize(glm::mix(AnimationModelInterpolationDirection, { Back.x , 0.0f , -Back.z }, InterpolatingBlendCoeff));
				animationModel.GetTransformation().RotationMatrix = glm::lookAt(glm::vec3(0.0f), AnimationModelDirection, camera3d.GetUpVector());
				animationModel.GetTransformation().Translate(Back * SPEED);
			}
			else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			{
				if (WalkingSide != 2)
				{
					AnimationModelInterpolationDirection = AnimationModelDirection;
					InterpolatingBlendCoeff = 0.0f;
				}
				WalkingSide = 2;
				Moving = true;
				AnimationModelDirection = glm::normalize(glm::mix(AnimationModelInterpolationDirection, { Right.x , 0.0f , -Right.z }, InterpolatingBlendCoeff));
				animationModel.GetTransformation().RotationMatrix = glm::lookAt(glm::vec3(0.0f), AnimationModelDirection, camera3d.GetUpVector());
				animationModel.GetTransformation().Translate(Right * SPEED);
			}
			else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			{
				if (WalkingSide != 3)
				{
					AnimationModelInterpolationDirection = AnimationModelDirection;
					InterpolatingBlendCoeff = 0.0f;
				}
				WalkingSide = 3;
				Moving = true;
				AnimationModelDirection = glm::normalize(glm::mix(AnimationModelInterpolationDirection, { Left.x , 0.0f , -Left.z }, InterpolatingBlendCoeff));
				animationModel.GetTransformation().RotationMatrix = glm::lookAt(glm::vec3(0.0f), AnimationModelDirection, camera3d.GetUpVector());
				animationModel.GetTransformation().Translate(Left * SPEED);
			}
			else
			{
				Moving = false;
			}
		}

		
		if (Moving)
		{
			IdleWalkingBlendCoeff += BlendAmount;
			InterpolatingBlendCoeff += InterBlendAmount;
		}
		else
		{
			if (IdleWalkingBlendCoeff > 0.0f)
			{
				IdleWalkingBlendCoeff -= BlendAmount;
			}
			if (InterpolatingBlendCoeff > 0.0f)
			{
				InterpolatingBlendCoeff -= InterBlendAmount;
			}
		}
		IdleWalkingBlendCoeff = glm::clamp(IdleWalkingBlendCoeff, 0.0f, 1.0f);
		InterpolatingBlendCoeff = glm::clamp(InterpolatingBlendCoeff, 0.0f, 1.0f);


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
			animationModel.GetTransformation().Translate(camera3d.GetUpVector() * SPEED);
		}
		

		animator.UpdateBlendedAnimation(&IdleAnimation, &WalkingAnimation, IdleWalkingBlendCoeff, deltaTime);


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

		ShadowMap0.Draw(*Shaders.OmniShadowMapShader, Lights[0].GetTransformation()->Position, models, camera3d);
		ShadowMap1.Draw(*Shaders.OmniShadowMapShader, Lights[1].GetTransformation()->Position, models, camera3d);
		ShadowMap2.Draw(*Shaders.OmniShadowMapShader, Lights[2].GetTransformation()->Position, models, camera3d);
		ShadowMap3.Draw(*Shaders.OmniShadowMapShader, Lights[3].GetTransformation()->Position, models, camera3d);

		Gbuffer.Bind();
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		glfwGetWindowSize(window, &WindowSize.x, &WindowSize.y);
		glViewport(0, 0, WindowSize.x, WindowSize.y);

		glfwGetCursorPos(window, &mousePos.x, &mousePos.y);
		Target = { mousePos.x / WindowSize.x , -mousePos.y / WindowSize.y, 0.0f };

		camera3d.UpdateCameraMatrix(50.0f, (float)WindowSize.x / (float)WindowSize.y, 0.1f, 180.0f, WindowSize);
		camera3d.SetTarget(&animationModel, 30.0f, { 0.0f,10.0f,0.0f });
		camera3d.HandleInputs(window, WindowSize, FF_CAMERA_LAYOUT_INDUSTRY_STANDARD, 0.06f);

		std::function<void()> shaderPrepe = [&]() {};
		std::function<void()> shaderPrepe1 = [&]() {};
		std::function<void()> shaderPrepe2 = [&]() {};

		auto animationMatrices = animator.GetFinalBoneMatrices();

		model1->Draw(camera3d, *Shaders.GbufferShader, shaderPrepe, cubemap, shovelMaterial, shadowMaps, AOamount);
		MainCharac->Draw(camera3d, *Shaders.GbufferShader, shaderPrepe, cubemap, shovelMaterial, shadowMaps, AOamount);
		Stove->Draw(camera3d, *Shaders.GbufferShader, shaderPrepe, cubemap, MirrorMaterial, shadowMaps, AOamount);
		grid->Draw(camera3d, *Shaders.GbufferShader, shaderPrepe, cubemap, FloorMaterial, shadowMaps, AOamount);
		sofa->Draw(camera3d, *Shaders.GbufferShader, shaderPrepe, cubemap, SofaMaterial, shadowMaps, AOamount);
		animationModel.Draw(camera3d, *Shaders.GbufferShader, shaderPrepe, cubemap, AnimationModelMaterial, shadowMaps, animationMatrices, AOamount);
		
		wall->Draw(camera3d, *Shaders.GbufferShader, shaderPrepe, cubemap, WallMaterial, shadowMaps, AOamount);
		
		FUSIONCORE::Material redMaterial(0.5f, 0.0f, { 1.0f,0.0f,0.0f,1.0f });
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		subdModel.Draw(camera3d, *Shaders.GbufferShader, shaderPrepe, cubemap, redMaterial, shadowMaps, AOamount);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		Gbuffer.Unbind();
		ScreenFrameBuffer.Bind();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		Gbuffer.Draw(camera3d, *Shaders.DeferredPBRshader, [&]() {}, WindowSize, shadowMaps, cubemap, 0.5f, false, 0.5f, 2.0f);

		glViewport(0, 0, WindowSize.x, WindowSize.y);

		auto gbufferSize = Gbuffer.GetFBOSize();
		FUSIONCORE::CopyDepthInfoFBOtoFBO(Gbuffer.GetFBO(), { gbufferSize.x ,gbufferSize.y }, ScreenFrameBuffer.GetFBO());
		ScreenFrameBuffer.Bind();

		cubemap.Draw(camera3d, WindowSize.Cast<float>());

#ifdef ENGINE_DEBUG

		static bool AllowD = true;
		if (IsKeyPressedOnce(window, GLFW_KEY_D, AllowD))
		{
			showDebug = !showDebug;
		}
		if (showDebug)
		{
			for (size_t i = 0; i < Lights.size(); i++)
			{
				Lights[i].Draw(camera3d, *Shaders.LightShader);
			}
			Box0.DrawBoxMesh(camera3d, *Shaders.LightShader);
			Box1.DrawBoxMesh(camera3d, *Shaders.LightShader);
			SofaBox.DrawBoxMesh(camera3d, *Shaders.LightShader);
			StoveBox.DrawBoxMesh(camera3d, *Shaders.LightShader);
			tryBox.DrawBoxMesh(camera3d, *Shaders.LightShader);
			Plane.DrawBoxMesh(camera3d, *Shaders.LightShader);
			Plane2.DrawBoxMesh(camera3d, *Shaders.LightShader);
			floorBox.DrawBoxMesh(camera3d, *Shaders.LightShader);
		}
#endif
		
		ScreenFrameBuffer.Unbind();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		ScreenFrameBuffer.Draw(camera3d, *Shaders.FBOShader, [&]() {}, WindowSize, true, 0.3f, 3.0f);

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

		if (!Moving)
		{
			AnimationModelInterpolationDirection = AnimationModelDirection;
			InterpolatingBlendCoeff = 0.0f;
		}

	}

	FUSIONUTIL::DisposeDefaultShaders(Shaders);

	ScreenFrameBuffer.clean();
	Gbuffer.clean();
	Box0.GetBoxMesh()->Clean();
	Box1.GetBoxMesh()->Clean();
	StoveBox.GetBoxMesh()->Clean();
	tryBox.GetBoxMesh()->Clean();
	Plane.GetBoxMesh()->Clean();
	Plane2.GetBoxMesh()->Clean();
	SofaBox.GetBoxMesh()->Clean();
	floorBox.GetBoxMesh()->Clean();

	shovelMaterial.Clean();
	FloorMaterial.Clean();
	SofaMaterial.Clean();
	MirrorMaterial.Clean();
	WallMaterial.Clean();
	AnimationModelMaterial.Clean();

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

float Application::RoundNonZeroToOne(float input)
{
	float result = 0.0f;
	if (glm::abs(input) > glm::epsilon<float>())
	{
		if (input < 0.0f)
		{
			result = -1.0f;
		}
		else if (input > 0.0f)
		{
			result = 1.0f;
		}
	}
	return result;
}



