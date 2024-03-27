#include "Application.hpp"
#include "FusionFrame.h"
#include <glew.h>
#include <glfw3.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <unordered_set>

#define SPEED 24.0f
#define CAMERA_CLOSE_PLANE 0.1f
#define CAMERA_FAR_PLANE 180.0f
const float BlendAmount = 0.04f;
const float InterBlendAmount = 0.04f;

int Application::Run()
{
	const int width = 1000;
	const int height = 1000;

	GLFWwindow* window = FUSIONUTIL::InitializeWindow(width, height,4,6, "FusionFrame Engine");

	FUSIONCORE::InitializeAnimationUniformBuffer();
	
	FUSIONUTIL::DefaultShaders Shaders;
	FUSIONUTIL::InitializeDefaultShaders(Shaders);

	FUSIONCORE::CubeMap cubemap(*Shaders.CubeMapShader);
	FUSIONCORE::ImportCubeMap("Resources/sunflowers_puresky_2k.hdr", 1024, cubemap, Shaders.HDRIShader->GetID(), Shaders.ConvolutateCubeMapShader->GetID(), Shaders.PreFilterCubeMapShader->GetID());

	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	FUSIONCORE::Gbuffer Gbuffer(mode->width, mode->height);

	FUSIONCORE::FrameBuffer ScreenFrameBuffer(mode->width, mode->height);


	FUSIONCORE::LightIcon = std::make_unique<FUSIONCORE::Model>("Resources/LightIcon.fbx");

	FUSIONCORE::Camera3D camera3d;

	FUSIONCORE::Texture2D ShovelDiffuse("Resources/texture_diffuse.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);
	FUSIONCORE::Texture2D ShovelNormal("Resources/texture_normal.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);
	FUSIONCORE::Texture2D ShovelSpecular("Resources/texture_specular.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);

	FUSIONCORE::Texture2D FloorSpecular("Resources/floor/diagonal_parquet_rough_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, true);
	FUSIONCORE::Texture2D FloorNormal("Resources/floor/diagonal_parquet_nor_dx_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, true);
	FUSIONCORE::Texture2D FloorAlbedo("Resources/floor/diagonal_parquet_diff_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, true);

	FUSIONCORE::Texture2D SofaDiffuse("Resources\\models\\sofa\\textures\\sofa_03_diff_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);
	FUSIONCORE::Texture2D SofaNormal("Resources\\models\\sofa\\textures\\sofa_03_nor_dx_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);
	FUSIONCORE::Texture2D SofaSpecular("Resources\\models\\sofa\\textures\\sofa_03_rough_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);

	FUSIONCORE::Texture2D MirrorDiffuse("Resources\\models\\stove\\textures\\electric_stove_diff_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);
	FUSIONCORE::Texture2D MirrorNormal("Resources\\models\\stove\\textures\\electric_stove_nor_dx_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);
	FUSIONCORE::Texture2D MirrorSpecular("Resources\\models\\stove\\textures\\electric_stove_rough_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);
	FUSIONCORE::Texture2D MirrorMetalic("Resources\\models\\stove\\textures\\electric_stove_metal_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);

	FUSIONCORE::Texture2D WallDiffuse("Resources\\wall\\textures\\painted_plaster_wall_diff_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);
	FUSIONCORE::Texture2D WallNormal("Resources\\wall\\textures\\painted_plaster_wall_nor_dx_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);
	FUSIONCORE::Texture2D WallSpecular("Resources\\wall\\textures\\painted_plaster_wall_rough_2k.jpg", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);
	
	FUSIONCORE::Texture2D bearDiffuse("Resources\\taunt\\textures\\bear_diffuse.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);
	FUSIONCORE::Texture2D bearNormal("Resources\\taunt\\textures\\bear_normal.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);
	FUSIONCORE::Texture2D bearSpecular("Resources\\taunt\\textures\\bear_roughness.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);

	FUSIONCORE::Texture2D ShrubDiffuse("Resources\\models\\shrub\\textures\\shrub_04_diff_1k.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);
	FUSIONCORE::Texture2D ShrubNormal("Resources\\models\\shrub\\textures\\shrub_04_nor_dx_1k.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);
	FUSIONCORE::Texture2D ShrubSpecular("Resources\\models\\shrub\\textures\\shrub_04_rough_1k.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);
	FUSIONCORE::Texture2D ShrubAlpha("Resources\\models\\shrub\\textures\\shrub_04_alpha_1k.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);

	int shadowMapSize = 512;

	FUSIONCORE::OmniShadowMap ShadowMap0(shadowMapSize, shadowMapSize, 75.0f);
	FUSIONCORE::OmniShadowMap ShadowMap1(shadowMapSize, shadowMapSize, 75.0f);
	FUSIONCORE::OmniShadowMap ShadowMap2(shadowMapSize, shadowMapSize, 75.0f);
	FUSIONCORE::OmniShadowMap ShadowMap3(shadowMapSize, shadowMapSize, 75.0f);

	std::vector<float> shadowCascadeLevels{ CAMERA_FAR_PLANE / 50.0f, CAMERA_FAR_PLANE / 25.0f, CAMERA_FAR_PLANE / 10.0f, CAMERA_FAR_PLANE / 2.0f };
	FUSIONCORE::CascadedDirectionalShadowMap sunShadowMap(2048, 2048, shadowCascadeLevels);

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

	for (size_t i = 0; i < 10; i++)
	{
		Lights.emplace_back(glm::vec3(RandomFloats(engine), RandomFloatsY(engine), RandomFloats(engine)), glm::vec3(RandomColor(engine), RandomColor(engine), RandomColor(engine)), RandomIntensity(engine));
	}

	FUSIONCORE::Color SunColor(FF_COLOR_LEMONADE);
	SunColor.Brighter();
	FUSIONCORE::Light Sun(glm::vec3(-0.593494, 0.648119, 0.477182),SunColor.GetRGB(), 6.0f, FF_DIRECTIONAL_LIGHT);

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
	std::unique_ptr<FUSIONCORE::Model> MainCharac = std::make_unique<FUSIONCORE::Model>("Resources\\shovel2.obj");
	std::unique_ptr<FUSIONCORE::Model> model1 = std::make_unique<FUSIONCORE::Model>("Resources\\shovel2.obj");;
	std::unique_ptr<FUSIONCORE::Model> Stove = std::make_unique<FUSIONCORE::Model>("Resources\\models\\stove\\stove.obj");;
	std::unique_ptr<FUSIONCORE::Model> grid = std::make_unique<FUSIONCORE::Model>("Resources\\floor\\grid.obj");
	std::unique_ptr<FUSIONCORE::Model> sofa = std::make_unique<FUSIONCORE::Model>("Resources\\models\\sofa\\model\\sofa.obj");
	std::unique_ptr<FUSIONCORE::Model> wall = std::make_unique<FUSIONCORE::Model>("Resources\\floor\\grid.obj");

	FUSIONCORE::Model subdModel;
	FUSIONCORE::MESHOPERATIONS::ImportObj("Resources\\subDModel.obj", subdModel);
	subdModel.GetTransformation().ScaleNoTraceBack({ 9.0f,9.0f,9.0f });
	subdModel.GetTransformation().TranslateNoTraceBack({ 22.0f,6.0f,-9.0f });

	auto ImportedModels = FUSIONCORE::ImportMultipleModelsFromDirectory("Resources\\models\\Grasses",false);
	auto shrub = std::move(ImportedModels[1]);
	ImportedModels.erase(ImportedModels.begin() + 1);
	auto Tower = std::move(ImportedModels[1]);
	ImportedModels.erase(ImportedModels.begin() + 1);
	Tower->GetTransformation().TranslateNoTraceBack({ 39.0f,0.0f,-9.0f });
	FUSIONPHYSICS::CollisionBox3DAABB TowerBox(Tower->GetTransformation(), glm::vec3(1.0f));
	TowerBox.UpdateAttributes();
	//FUSIONPHYSICS::MESHOPERATIONS::TestAssimp("Resources\\subDModel.obj", "C:\\Users\\kbald\\Desktop\\subdOrijinalTestAssimp.obj");
	FUSIONCORE::MESHOPERATIONS::LoopSubdivision(subdModel.Meshes[0], 1);
	
	shrub->GetTransformation().ScaleNoTraceBack(glm::vec3(24.0f));
	//FUSIONPHYSICS::MESHOPERATIONS::SmoothObject(subdModel.Meshes[0]);

	/*for (size_t i = 0; i < sofa->Meshes.size(); i++)
	{
		FUSIONPHYSICS::MESHOPERATIONS::LoopSubdivision(sofa->Meshes[i] , 3);
	}*/
	

	

	//FUSIONPHYSICS::MESHOPERATIONS::LoopSubdivision(IMPORTTEST.Meshes[0], 1);

	//FUSIONPHYSICS::MESHOPERATIONS::ExportObj("C:\\Users\\kbald\\Desktop\\TEST2Subd.obj", subdModel);

	//FUSIONPHYSICS::MESHOPERATIONS::LoopSubdivision(grid->Meshes[0], 1);
	//FUSIONPHYSICS::MESHOPERATIONS::LoopSubdivision(grid->Meshes[0], 2);
	//FUSIONPHYSICS::MESHOPERATIONS::LoopSubdivision(grid->Meshes[0], 1);

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
	AnimationModelMaterial.PushTextureMap(TEXTURE_SPECULAR0, bearSpecular);

	FUSIONCORE::Material ShrubMaterial;
	ShrubMaterial.PushTextureMap(TEXTURE_DIFFUSE0, ShrubDiffuse);
	ShrubMaterial.PushTextureMap(TEXTURE_NORMAL0, ShrubNormal);
	ShrubMaterial.PushTextureMap(TEXTURE_SPECULAR0, ShrubSpecular);
	ShrubMaterial.PushTextureMap(TEXTURE_ALPHA0, ShrubAlpha);

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
	FUSIONPHYSICS::CollisionBox3DAABB StoveBox(Stove->GetTransformation(), glm::vec3(1.0f));
	FUSIONPHYSICS::CollisionBox3DAABB tryBox({ 1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f });
	FUSIONPHYSICS::CollisionBoxPlane Plane({ 1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f });
	//FUSIONPHYSICS::CollisionBox3DAABB SofaBox(sofa->GetTransformation(), { 0.7f,0.8f,1.0f });
	FUSIONPHYSICS::CollisionBox3DAABB SofaBox(sofa->GetTransformation(), glm::vec3(1.0f));
	FUSIONPHYSICS::CollisionBoxPlane Plane2({ 1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f });
	FUSIONPHYSICS::CollisionBoxPlane floorBox({ 1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f });

	FUSIONCORE::Model IMPORTTEST("Resources\\floor\\grid.obj");
	//FUSIONCORE::MESHOPERATIONS::ImportObj("Resources\\floor\\grid.obj", IMPORTTEST);
	//IMPORTTEST.GetTransformation().Translate({ 7.0f,7.0f,0.0f });
	IMPORTTEST.GetTransformation().ScaleNoTraceBack({ 0.05f,0.05f,0.05f });

	Plane.GetTransformation().Scale({ 7.0f,7.0f ,7.0f });
	Plane.GetTransformation().Translate({ 0.7f,10.0f,0.0f });
	Plane.UpdateAttributes();
	Plane2.GetTransformation().Scale({ 2.0f,2.0f ,2.0f });
	Plane2.GetTransformation().Translate({ 0.7f,0.0f,0.0f });

	floorBox.GetTransformation().Scale(glm::vec3(190.0f));
	floorBox.GetTransformation().Translate({ 0.0f,-1.0f,0.0f });

	grid->GetTransformation().ScaleNoTraceBack({ 8.0f,8.0f ,8.0f });
	grid->GetTransformation().TranslateNoTraceBack({ 0.0f,-1.0f,0.0f });

	//SofaBox.GetTransformation().Translate({ 0.0f,-1.0f,0.0f });

	model1->PushChild(&Box1);
	sofa->PushChild(&SofaBox);
	sofa->UpdateChildren();


	MainCharac->UpdateChildren();

	Stove->PushChild(&StoveBox);
	Stove->UpdateChildren();

	//StoveBox.GetTransformation().ScaleNoTraceBack({ 0.5f,0.8f ,1.0f });
	//StoveBox.GetTransformation().Translate({ 0.4f,-0.2f,-1.3f });

	glm::vec4 BackGroundColor(FUSIONCORE::Color(FF_COLOR_AZURE).GetRGBA());

	Shaders.DeferredPBRshader->use();

	FUSIONCORE::SetEnvironmentIBL(*Shaders.DeferredPBRshader, 2.0f, glm::vec3(BackGroundColor.x, BackGroundColor.y, BackGroundColor.z));
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
	models.push_back(&IMPORTTEST);
	models.push_back(&subdModel);
	models.push_back(Tower.get());

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

	FUSIONCORE::Model Capsule("Resources\\Sphere.obj");
	FUSIONPHYSICS::CollisionBox Capsule0(Capsule.Meshes[0], Capsule.GetTransformation());
	Capsule0.GetTransformation().ScaleNoTraceBack(glm::vec3(6.0f, 5.5f, 6.0f));
	Capsule0.GetTransformation().TranslateNoTraceBack({ 15.0f,5.0f,0.0f});
	animationModel.PushChild(&Capsule0);
	Capsule0.GetTransformation().TranslateNoTraceBack({ 0.0f,-1.0f,0.0f });
	models.push_back(&animationModel);

	std::vector<FUSIONCORE::Object*> ObjectInstances;
	ObjectInstances.push_back(&Capsule0);
	ObjectInstances.push_back(&TowerBox);
	ObjectInstances.push_back(&SofaBox);
	ObjectInstances.push_back(&StoveBox);
	ObjectInstances.push_back(&Box1);
	ObjectInstances.push_back(&Plane);
	ObjectInstances.push_back(&tryBox);

	FUSIONCORE::VBO instanceVBO;
	auto DistibutedPoints = FUSIONCORE::MESHOPERATIONS::DistributePointsOnMeshSurface(grid->Meshes[0], grid->GetTransformation(), 2000, 109);
	FUSIONCORE::MESHOPERATIONS::FillInstanceDataVBO(instanceVBO, DistibutedPoints);

	/*FUSIONCORE::VBO TowerinstanceVBO;
	auto TowerDistibutedPoints = FUSIONCORE::MESHOPERATIONS::DistributePointsOnMeshSurface(grid->Meshes[0], grid->GetTransformation(), 4, 109);
	FUSIONCORE::MESHOPERATIONS::FillInstanceDataVBO(TowerinstanceVBO, TowerDistibutedPoints);*/

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

	camera3d.SetMinMaxZoom(true,-15.0f, 15.0f);
	camera3d.SetZoomSensitivity(3.0f);

	FUSIONPHYSICS::QuadNode headNode;

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		
		//LOG("FPS: " << 1.0f / deltaTime);

		auto start_time = std::chrono::high_resolution_clock::now();

		tryBox.GetTransformation().Translate({ 0.0f,0.0f,std::sin(time(0)) / 5.0f });
		tryBox.UpdateAttributes();

		Plane.GetTransformation().Rotate({ 0.0f,1.0f,0.0f }, std::sin(time(0)));
		Plane.GetTransformation().Translate({ 0.0f,std::sin(time(0)) / 10.0f,0.0f });
		Plane.UpdateAttributes();

		Stove->UpdateChildren();
		//SofaBox.UpdateAttributes();
		//sofa->UpdateChildren();

		model1->GetTransformation().Rotate({ 0.0f,1.0f,0.0f }, std::sin(time(0)));
		model1->UpdateChildren();

		animationModel.UpdateChildren();


		bool Collision = false;
		glm::vec3 direction;

		shrub->GetTransformation().RotateNoTraceBack({ 1.0f,0.0f,1.0f}, std::sin(time(0)) * 0.1f);

		FUSIONPHYSICS::UpdateQuadTreeWorldPartitioning(headNode, ObjectInstances,2,5);
		auto UniqueQuadObjects = Capsule0.GetUniqueQuadsObjects();
		LOG("UNIQUE BOX COUNT: " << UniqueQuadObjects.size());
		for (const auto& Box : UniqueQuadObjects)
		{
			auto QuadModel = Box->DynamicObjectCast<FUSIONPHYSICS::CollisionBox*>();
			if (QuadModel != nullptr)
			{
				if (FUSIONPHYSICS::IsCollidingSAT(*QuadModel, Capsule0))
				{
					Collision = true;
					glm::vec3 ObjectPosition = FUSIONCORE::TranslateVertex(Capsule0.GetTransformation().GetModelMat4(), *Capsule0.GetTransformation().OriginPoint) -
						                       FUSIONCORE::TranslateVertex(QuadModel->GetTransformation().GetModelMat4(), *QuadModel->GetTransformation().OriginPoint);
					direction = glm::normalize(ObjectPosition);
				}
			}
		}

		static bool FirstFloorTouch = true;
		if (!FUSIONPHYSICS::IsCollidingSAT(floorBox, Capsule0))
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


		auto Front = glm::normalize(glm::vec3(camera3d.Orientation.x,0.0f,camera3d.Orientation.z));
		auto Back = -Front;
		auto Right = glm::normalize(glm::cross(Front, camera3d.GetUpVector()));
		auto Left = -Right;

		//LOG("FRONT: " << Vec3<float>(Front));

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

			animationModel.GetTransformation().Translate(glm::normalize(glm::vec3(direction.x,0.0f,direction.z)) * SPEED * 0.5f * deltaTime);

			direction = -direction;
			const float E = -0.3f;
			if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && glm::dot(direction, Front) <= E)
			{
				Moving = true;
				animationModel.GetTransformation().Translate(Front * SPEED * deltaTime);
			}
			else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && glm::dot(direction, Back) <= E)
			{
				Moving = true;
				animationModel.GetTransformation().Translate(Back * SPEED * deltaTime);
			}
			else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && glm::dot(direction, Right) <= E)
			{
				Moving = true;
				animationModel.GetTransformation().Translate(Right * SPEED * deltaTime);
			}
			else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && glm::dot(direction, Left) <= E)
			{
				Moving = true;
				animationModel.GetTransformation().Translate(Left * SPEED * deltaTime);
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
				animationModel.GetTransformation().Translate(Front * SPEED * deltaTime);
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
				animationModel.GetTransformation().Translate(Back * SPEED * deltaTime);
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
				animationModel.GetTransformation().Translate(Right * SPEED * deltaTime);
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
				animationModel.GetTransformation().Translate(Left * SPEED * deltaTime);
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
			animationModel.GetTransformation().Translate(camera3d.GetUpVector() * 0.5f);
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

		glfwGetWindowSize(window, &WindowSize.x, &WindowSize.y);
		camera3d.UpdateCameraMatrix(45.0f, (float)WindowSize.x / (float)WindowSize.y, CAMERA_CLOSE_PLANE, CAMERA_FAR_PLANE, WindowSize);
		camera3d.SetTarget(&animationModel, 30.0f, { 0.0f,10.0f,0.0f });
		camera3d.HandleInputs(window, WindowSize, FF_CAMERA_LAYOUT_INDUSTRY_STANDARD, 0.06f);

		ShadowMap0.Draw(*Shaders.OmniShadowMapShader, Lights[0], models, camera3d);
		ShadowMap1.Draw(*Shaders.OmniShadowMapShader, Lights[1], models, camera3d);
		ShadowMap2.Draw(*Shaders.OmniShadowMapShader, Lights[2], models, camera3d);
		ShadowMap3.Draw(*Shaders.OmniShadowMapShader, Lights[3], models, camera3d);

		sunShadowMap.Draw(*Shaders.CascadedDirectionalShadowShader, camera3d, models,Sun);

		Gbuffer.Bind();
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		glViewport(0, 0, WindowSize.x, WindowSize.y);

		glfwGetCursorPos(window, &mousePos.x, &mousePos.y);

		std::function<void()> shaderPrepe = [&]() {};
		std::function<void()> shaderPrepe1 = [&]() {};
		std::function<void()> shaderPrepe2 = [&]() {};


		auto animationMatrices = animator.GetFinalBoneMatrices();
		//IMPORTTEST.Draw(camera3d, *Shaders.GbufferShader, shaderPrepe, cubemap, shovelMaterial, shadowMaps, AOamount);
		//shrub->DrawDeferredInstanced(camera3d, *Shaders.InstancedGbufferShader, shaderPrepe, ShrubMaterial,instanceVBO,DistibutedPoints.size(), AOamount);
		shrub->DrawDeferredInstancedImportedMaterial(camera3d, *Shaders.InstancedGbufferShader, shaderPrepe, instanceVBO, DistibutedPoints.size(), AOamount);
		//if (FUSIONCORE::IsModelInsideCameraFrustum(*model1, camera3d))
		{
			model1->DrawDeferred(camera3d, *Shaders.GbufferShader, shaderPrepe, shovelMaterial, AOamount);
		}
		MainCharac->DrawDeferred(camera3d, *Shaders.GbufferShader, shaderPrepe, shovelMaterial, AOamount);
		grid->DrawDeferred(camera3d, *Shaders.GbufferShader, shaderPrepe, FloorMaterial, AOamount);

		//if (FUSIONCORE::IsModelInsideCameraFrustum(*Stove, camera3d))
		{
			Stove->DrawDeferred(camera3d, *Shaders.GbufferShader, shaderPrepe, MirrorMaterial, AOamount);
		}
	
		for (size_t i = 0; i < ImportedModels.size(); i++)
		{
			ImportedModels[i]->DrawDeferredImportedMaterial(camera3d, *Shaders.GbufferShader, shaderPrepe, AOamount);
		}

		
		Tower->DrawDeferredImportedMaterial(camera3d, *Shaders.GbufferShader, shaderPrepe, AOamount);


		wall->DrawDeferred(camera3d, *Shaders.GbufferShader, shaderPrepe, WallMaterial, AOamount);

		//if (!Collision)
		//{
		   animationModel.DrawDeferred(camera3d, *Shaders.GbufferShader, shaderPrepe, AnimationModelMaterial, animationMatrices, AOamount);
		//}
		
		
		FUSIONCORE::Material redMaterial(0.3f, 0.0f, { 1.0f,0.0f,0.0f,1.0f });
		subdModel.DrawDeferred(camera3d, *Shaders.GbufferShader, shaderPrepe, redMaterial, AOamount);
		//if (FUSIONCORE::IsModelInsideCameraFrustum(*sofa, camera3d))
		{
			sofa->DrawDeferred(camera3d, *Shaders.GbufferShader, shaderPrepe, SofaMaterial, AOamount);
		}
		Capsule.DrawDeferred(camera3d, *Shaders.GbufferShader, shaderPrepe, FUSIONCORE::Material(), AOamount);
		
		Gbuffer.Unbind();
		ScreenFrameBuffer.Bind();
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Gbuffer.Draw(camera3d, *Shaders.DeferredPBRshader, [&]() {}, WindowSize, shadowMaps,sunShadowMap, cubemap, 0.5f);

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
			Capsule0.DrawBoxMesh(camera3d, *Shaders.LightShader);
			Box1.DrawBoxMesh(camera3d, *Shaders.LightShader);
			SofaBox.DrawBoxMesh(camera3d, *Shaders.LightShader);
			StoveBox.DrawBoxMesh(camera3d, *Shaders.LightShader);
			tryBox.DrawBoxMesh(camera3d, *Shaders.LightShader);
			Plane.DrawBoxMesh(camera3d, *Shaders.LightShader);
			Plane2.DrawBoxMesh(camera3d, *Shaders.LightShader);
			floorBox.DrawBoxMesh(camera3d, *Shaders.LightShader);
			TowerBox.DrawBoxMesh(camera3d, *Shaders.LightShader);

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			subdModel.DrawDeferred(camera3d, *Shaders.GbufferShader, shaderPrepe, FUSIONCORE::Material(), AOamount);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			FUSIONPHYSICS::VisualizeQuadTree(headNode, camera3d, *Shaders.LightShader, FF_COLOR_RED);
		}
#endif
		
		ScreenFrameBuffer.Unbind();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		ScreenFrameBuffer.Draw(camera3d, *Shaders.FBOShader, [&]() {}, WindowSize, sunShadowMap,true, 0.7f, 3.0f);

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
	
	shovelMaterial.Clean();
	FloorMaterial.Clean();
	SofaMaterial.Clean();
	MirrorMaterial.Clean();
	WallMaterial.Clean();
	AnimationModelMaterial.Clean();
	ShrubMaterial.Clean();

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





