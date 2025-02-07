#include "Application.hpp"
#include <glew.h>
#include <glfw3.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <unordered_set>

//#include "FusionCore/PathTracer/PathTracer.hpp"

#define SPEED 6.0f
#define CAMERA_CLOSE_PLANE 0.5f
#define CAMERA_FAR_PLANE 800.0f
const float BlendAmount = 0.04f;
const float InterBlendAmount = 0.04f;


void PrintError() {
	GLenum err;
	for (;;) {
		err = glGetError();
		if (err == GL_NO_ERROR) break;
		printf("Error: %s\n", glewGetErrorString(err));
	}
}

int Application::Run()
{
	const int width = 1000;
	const int height = 1000;

	FUSIONCORE::Window ApplicationWindow;
	ApplicationWindow.InitializeWindow(width, height, 4, 6, false, "FusionFrame Engine");
	auto window = ApplicationWindow.GetWindow();

	FUSIONCORE::InitializeCascadedShadowMapTextureArray(4096, 1, 1024);
	FUSIONUTIL::DefaultShaders Shaders;

	//FUSIONCORE::Shader shader;
	//shader.PushShaderSource(FUSIONCORE::FF_VERTEX_SHADER_SOURCE, "Shaders/Gbuffer.fs");
	//shader.AlterShaderUniformArrayValue(FUSIONCORE::FF_VERTEX_SHADER_SOURCE, "disableclaymaterial", 2000);
	//shader.AlterShaderMacroDefinitionValue(FUSIONCORE::FF_VERTEX_SHADER_SOURCE, "MAX_LIGHT_COUNT", "556");
	//LOG(shader.GetShaderSource(FUSIONCORE::FF_VERTEX_SHADER_SOURCE));


	FUSIONCORE::CubeMap cubemap(*Shaders.CubeMapShader);
	FUSIONCORE::ImportCubeMap("Resources/kloppenheim_02_puresky_2k.hdr", 512, cubemap, Shaders);

	const FUSIONUTIL::VideoMode mode = FUSIONUTIL::GetVideoMode(FUSIONUTIL::GetPrimaryMonitor());
	FUSIONCORE::GeometryBuffer Gbuffer(mode.width, mode.height);
	FUSIONCORE::ScreenFrameBuffer ScreenFrameBuffer(mode.width, mode.height);

	FUSIONCORE::LightIcon = std::make_unique<FUSIONCORE::Model>("Resources/LightIcon.fbx");

	FUSIONCORE::Camera3D camera3d;
	FUSIONCORE::Camera2D camera2d;

	FUSIONCORE::Texture2D ShovelDiffuse("Resources/texture_diffuse.png");
	FUSIONCORE::Texture2D ShovelNormal("Resources/texture_normal.png");
	FUSIONCORE::Texture2D ShovelSpecular("Resources/texture_specular.png");

	FUSIONCORE::Texture2D FloorSpecular("Resources/floor/snow_02_rough_1k.png", FF_TEXTURE_WRAP_MODE_GL_REPEAT, FF_TEXTURE_WRAP_MODE_GL_REPEAT);
	FUSIONCORE::Texture2D FloorNormal("Resources/floor/snow_02_nor_gl_1k.png", FF_TEXTURE_WRAP_MODE_GL_REPEAT, FF_TEXTURE_WRAP_MODE_GL_REPEAT);
	FUSIONCORE::Texture2D FloorAlbedo("Resources/floor/snow_02_diff_1k.png", FF_TEXTURE_WRAP_MODE_GL_REPEAT, FF_TEXTURE_WRAP_MODE_GL_REPEAT);

	FUSIONCORE::Texture2D SofaDiffuse("Resources\\models\\sofa\\textures\\sofa_03_diff_2k.jpg");
	FUSIONCORE::Texture2D SofaNormal("Resources\\models\\sofa\\textures\\sofa_03_nor_gl_2k.jpg");
	FUSIONCORE::Texture2D SofaSpecular("Resources\\models\\sofa\\textures\\sofa_03_rough_2k.jpg");

	FUSIONCORE::Texture2D StoveDiffuse("Resources\\models\\stove\\textures\\electric_stove_diff_2k.jpg");
	FUSIONCORE::Texture2D StoveNormal("Resources\\models\\stove\\textures\\electric_stove_nor_gl_2k.jpg");
	FUSIONCORE::Texture2D StoveSpecular("Resources\\models\\stove\\textures\\electric_stove_rough_2k.jpg");
	FUSIONCORE::Texture2D StoveMetalic("Resources\\models\\stove\\textures\\electric_stove_metal_2k.jpg");

	FUSIONCORE::Texture2D WallDiffuse("Resources\\wall\\textures\\painted_plaster_wall_diff_2k.jpg");
	FUSIONCORE::Texture2D WallNormal("Resources\\wall\\textures\\painted_plaster_wall_nor_dx_2k.jpg");
	FUSIONCORE::Texture2D WallSpecular("Resources\\wall\\textures\\painted_plaster_wall_rough_2k.jpg");

	FUSIONCORE::Texture2D bearDiffuse("Resources\\taunt\\textures\\bear_diffuse.png");
	FUSIONCORE::Texture2D bearNormal("Resources\\taunt\\textures\\bear_normal.png");
	FUSIONCORE::Texture2D bearSpecular("Resources\\taunt\\textures\\bear_roughness.png");

	FUSIONCORE::Texture2D ShrubDiffuse("Resources\\models\\shrub\\textures\\shrub_04_diff_1k.png");
	FUSIONCORE::Texture2D ShrubNormal("Resources\\models\\shrub\\textures\\shrub_04_nor_gl_1k.png");
	FUSIONCORE::Texture2D ShrubSpecular("Resources\\models\\shrub\\textures\\shrub_04_rough_1k.png");
	FUSIONCORE::Texture2D ShrubAlpha("Resources\\models\\shrub\\textures\\shrub_04_alpha_1k.png");
	FUSIONCORE::Texture2D FalloutPosterNormal("Resources\\Paper_Wrinkled_001_normal.jpg",FF_TEXTURE_WRAP_MODE_GL_REPEAT);

	FUSIONCORE::Texture2D FalloutPoster("Resources\\FalloutPoster.png",FF_TEXTURE_WRAP_MODE_GL_CLAMP_TO_EDGE, FF_TEXTURE_WRAP_MODE_GL_CLAMP_TO_EDGE,
		FF_TEXTURE_TARGET_GL_TEXTURE_2D,FF_DATA_TYPE_GL_UNSIGNED_BYTE,FF_TEXTURE_FILTER_MODE_GL_LINEAR_MIPMAP_LINEAR, FF_TEXTURE_FILTER_MODE_GL_LINEAR_MIPMAP_LINEAR,false);

	int shadowMapSize = 512;

	//FUSIONCORE::OmniShadowMap ShadowMap0(shadowMapSize, shadowMapSize, 75.0f);
	//FUSIONCORE::OmniShadowMap ShadowMap1(shadowMapSize, shadowMapSize, 75.0f);
	//FUSIONCORE::OmniShadowMap ShadowMap2(shadowMapSize, shadowMapSize, 75.0f);
	//FUSIONCORE::OmniShadowMap ShadowMap3(shadowMapSize, shadowMapSize, 75.0f);

	std::vector<float> shadowCascadeLevels{ CAMERA_FAR_PLANE / 10.0f, CAMERA_FAR_PLANE / 7.0f, CAMERA_FAR_PLANE / 5.0f, CAMERA_FAR_PLANE / 2.0f };
	//std::vector<float> shadowCascadeLevels{ CAMERA_FAR_PLANE / 2.0f, CAMERA_FAR_PLANE / 7.0f, CAMERA_FAR_PLANE / 25.0f, CAMERA_FAR_PLANE / 50.0f };
	std::vector<glm::vec2> shadowCascadeTextureSizes{ {2048,2048},{1024,1024},{1024,1024},{1024,1024},{512,512}};
	//std::vector<glm::vec2> shadowCascadeTextureSizes2{ {128,128},{256,256},{512,512},{512,512},{1024,1024}};
	//FUSIONCORE::CascadedDirectionalShadowMap StaticShadowMap(shadowCascadeTextureSizes, shadowCascadeLevels);
	FUSIONCORE::CascadedDirectionalShadowMap sunShadowMap(shadowCascadeTextureSizes,shadowCascadeLevels);
	FUSIONCORE::SetCascadedShadowSoftness(*Shaders.DeferredPBRshader,2.0f);
	FUSIONCORE::SetCascadedShadowBiasMultiplier(*Shaders.DeferredPBRshader);
	//FUSIONCORE::CascadedDirectionalShadowMap sunShadowMap3(1024, 1024, shadowCascadeLevels);

	glm::ivec2 WindowSize;
	glm::dvec2 mousePos(0.0f);

	glm::vec3 Target(0.0f);

	camera3d.SetPosition(glm::vec3(12.353, 13.326, 15.2838));
	camera3d.SetOrientation(glm::vec3(-0.593494, -0.648119, -0.477182));

	auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	std::uniform_real_distribution<float> RandomFloats(-10.0f, 10.0f);
	std::uniform_real_distribution<float> RandomFloatsY(0.0f, 30.0f);
	std::uniform_real_distribution<float> RandomColor(0.0f, 1.0f);
	std::uniform_real_distribution<float> RandomIntensity(400.0f, 600.0f);
	std::default_random_engine engine(seed);

	std::vector<FUSIONCORE::Light> Lights;

	float LightIntensity;
	for (size_t i = 0; i < 10; i++)
	{
		LightIntensity = RandomIntensity(engine);
		Lights.emplace_back(glm::vec3(RandomFloats(engine), RandomFloatsY(engine), RandomFloats(engine)), glm::vec3(RandomColor(engine), RandomColor(engine), RandomColor(engine)), LightIntensity, FF_POINT_LIGHT, LightIntensity / 30.0f);
	}

	FUSIONCORE::Color SunColor(FF_COLOR_BABY_BLUE);
	SunColor.Brighter();
	FUSIONCORE::Light Sun(glm::vec3(-0.593494, 0.648119, 0.777182), SunColor.GetRGB(), 5.0f, FF_DIRECTIONAL_LIGHT);


	//FUSIONCORE::Light Sun2(glm::vec3(-0.593494, 0.648119, -0.777182), FF_COLOR_IRIS_PURPLE, 5.0f, FF_DIRECTIONAL_LIGHT);
	//FUSIONCORE::Light Sun3(glm::vec3(-0.593494, -0.648119, 0.777182), FF_COLOR_IRIS_PURPLE, 5.0f, FF_DIRECTIONAL_LIGHT);
	FUSIONUTIL::ThreadPool threads(5, 20);
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
	std::unique_ptr<FUSIONCORE::Model> Rock = std::make_unique<FUSIONCORE::Model>("Resources\\models\\RockFormation\\RockFormation.obj");
	//std::unique_ptr<FUSIONCORE::Model> Island = std::make_unique<FUSIONCORE::Model>("C:\\Users\\kbald\\Desktop\\Ziad_Game\\Island\\Island_low.fbx");

	//FUSIONCORE::Texture2D IslandBaseColor("C:\\Users\\kbald\\Desktop\\Ziad_Game\\Island\\Textures\\1K\\Island_low_Island_BaseColor.png");
	//FUSIONCORE::Texture2D IslandNormal("C:\\Users\\kbald\\Desktop\\Ziad_Game\\Island\\Textures\\1K\\Island_low_Island_Normal_inverted.png");
	//FUSIONCORE::Texture2D IslandRoughness("C:\\Users\\kbald\\Desktop\\Ziad_Game\\Island\\Textures\\1K\\Island_low_Island_Roughness.png");

	//Island->GetTransformation().ScaleNoTraceBack(glm::vec3(0.2f));
	//Island->GetTransformation().TranslateNoTraceBack(glm::vec3(0.0f, 60.0f, 0.0f));
	//Stove->SetIndirectCommandBuffer(1, 0, 0, 0);

	//FUSIONCORE::Model Isaac("C:\\Users\\kbald\\Desktop\\Isaac\\Isaac_low.obj");

	//Isaac.GetTransformation().ScaleNoTraceBack(glm::vec3(7.0f));
	//grid->GetTransformation().ScaleNoTraceBack({ 8.0f,8.0f ,8.0f });
	grid->GetTransformation().TranslateNoTraceBack({ 0.0f,-1.0f,0.0f });

	Rock->GetTransformation().ScaleNoTraceBack(glm::vec3(0.2f));

	FUSIONCORE::VBO RockInstanceVBO;
	auto RockDistibutedPoints = FUSIONCORE::MESHOPERATIONS::DistributePointsOnMeshSurfaceRandomized(grid->Meshes[0], grid->GetTransformation(), 5, 129);
	FUSIONCORE::MESHOPERATIONS::FillInstanceDataVBO(RockInstanceVBO, RockDistibutedPoints);
	Rock->SetInstanced(RockInstanceVBO, RockDistibutedPoints.size());

	auto RockBoxes = FUSIONPHYSICS::GenerateAABBCollisionBoxesFromInstancedModel(Rock->GetTransformation(), RockDistibutedPoints);

	FUSIONCORE::Model subdModel("Resources\\subDModel.obj");
	//FUSIONCORE::Model subdModel("subDModel.obj");

	//FUSIONCORE::MESHOPERATIONS::ImportObj("Resources\\subDModel.obj", subdModel);
	//subdModel.GetTransformation().ScaleNoTraceBack({ 9.0f,9.0f,9.0f });
	subdModel.GetTransformation().TranslateNoTraceBack({ 22.0f,6.0f,-9.0f });

	auto ImportedModels = FUSIONCORE::ImportMultipleModelsFromDirectory("Resources\\models\\Grasses", false);
	auto shrub = std::move(ImportedModels[1]);
	ImportedModels.erase(ImportedModels.begin() + 1);

	FUSIONCORE::Model Terrain("Resources\\Terrain.obj");
	FUSIONCORE::Model AlpDroneCollisionBox("Resources\\terrainTest.obj");
	//FUSIONCORE::MESHOPERATIONS::ImportObj("Resources\\Terrain.obj", Terrain);
	Terrain.GetTransformation().Scale(glm::vec3(10.0f));
	Terrain.GetTransformation().TranslateNoTraceBack(glm::vec3(10.0f,0.0f,0.0f));
	std::vector<std::shared_ptr<FUSIONPHYSICS::CollisionBox>> TerrainCollisionBoxes;

	//AlpDroneCollisionBox.GetTransformation().Scale(glm::vec3(5.0f));
	AlpDroneCollisionBox.GetTransformation().Translate(glm::vec3(70.0f,0.0f,0.0f));
	FUSIONPHYSICS::CollisionBox AlpCollisionBox(AlpDroneCollisionBox.Meshes[0], AlpDroneCollisionBox.GetTransformation());

	//FUSIONCORE::MESHOPERATIONS::LoopSubdivision(Rock->Meshes[0], 2);
	FUSIONCORE::MESHOPERATIONS::LoopSubdivision(subdModel, 2);

	//FUSIONCORE::MESHOPERATIONS::SmoothObject(subdModel.Meshes[0]);
	FUSIONCORE::MESHOPERATIONS::ExportObj("subdModel.obj", *Stove);

	TerrainCollisionBoxes.reserve(Terrain.Meshes.size());
	for (size_t i = 0; i < Terrain.Meshes.size(); i++)
	{
		TerrainCollisionBoxes.push_back(std::make_shared<FUSIONPHYSICS::CollisionBox>(Terrain.Meshes[i], Terrain.GetTransformation()));;
	}

	auto Tower = std::move(ImportedModels[1]);
	ImportedModels.erase(ImportedModels.begin() + 1);
	Tower->GetTransformation().TranslateNoTraceBack({ 39.0f,0.0f,-9.0f });
	Tower->GetTransformation().ScaleNoTraceBack({ 0.1f,0.1f,0.1f });
	FUSIONPHYSICS::CollisionBoxAABB TowerBox(Tower->GetTransformation(), glm::vec3(1.0f));
	Tower->PushChild(&TowerBox);
	Tower->UpdateChildren();
	//FUSIONPHYSICS::MESHOPERATIONS::TestAssimp("Resources\\subDModel.obj", "C:\\Users\\kbald\\Desktop\\subdOrijinalTestAssimp.obj");

	//shrub->GetTransformation().ScaleNoTraceBack(glm::vec3(24.0f));
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
	shovelMaterial.PushTextureMap(FF_TEXTURE_DIFFUSE, &ShovelDiffuse);
	shovelMaterial.PushTextureMap(FF_TEXTURE_NORMAL, &ShovelNormal);
	shovelMaterial.PushTextureMap(FF_TEXTURE_SPECULAR, &ShovelSpecular);

	FUSIONCORE::Material FloorMaterial;
	FloorMaterial.PushTextureMap(FF_TEXTURE_DIFFUSE, &FloorAlbedo);
	FloorMaterial.PushTextureMap(FF_TEXTURE_NORMAL, &FloorNormal);
	FloorMaterial.PushTextureMap(FF_TEXTURE_SPECULAR, &FloorSpecular);
	FloorMaterial.SetTiling(3.0f);

	FUSIONCORE::Material SofaMaterial;
	SofaMaterial.PushTextureMap(FF_TEXTURE_DIFFUSE, &SofaDiffuse);
	SofaMaterial.PushTextureMap(FF_TEXTURE_NORMAL, &SofaNormal);
	SofaMaterial.PushTextureMap(FF_TEXTURE_SPECULAR, &SofaSpecular);

	FUSIONCORE::Material MirrorMaterial;
	MirrorMaterial.PushTextureMap(FF_TEXTURE_DIFFUSE, &StoveDiffuse);
	MirrorMaterial.PushTextureMap(FF_TEXTURE_NORMAL, &StoveNormal);
	MirrorMaterial.PushTextureMap(FF_TEXTURE_SPECULAR, &StoveSpecular);
	MirrorMaterial.PushTextureMap(FF_TEXTURE_METALLIC, &StoveMetalic);

	FUSIONCORE::Material WallMaterial;
	WallMaterial.PushTextureMap(FF_TEXTURE_DIFFUSE, &WallDiffuse);
	WallMaterial.PushTextureMap(FF_TEXTURE_NORMAL, &WallNormal);
	WallMaterial.PushTextureMap(FF_TEXTURE_SPECULAR, &WallSpecular);
	WallMaterial.SetTiling(2.0f);

	FUSIONCORE::Material AnimationModelMaterial;
	AnimationModelMaterial.PushTextureMap(FF_TEXTURE_DIFFUSE, &bearDiffuse);
	AnimationModelMaterial.PushTextureMap(FF_TEXTURE_NORMAL, &bearNormal);
	AnimationModelMaterial.PushTextureMap(FF_TEXTURE_SPECULAR, &bearSpecular);

	FUSIONCORE::Material ShrubMaterial;
	ShrubMaterial.PushTextureMap(FF_TEXTURE_DIFFUSE, &ShrubDiffuse);
	ShrubMaterial.PushTextureMap(FF_TEXTURE_NORMAL, &ShrubNormal);
	ShrubMaterial.PushTextureMap(FF_TEXTURE_SPECULAR, &ShrubSpecular);
	ShrubMaterial.PushTextureMap(FF_TEXTURE_ALPHA, &ShrubAlpha);

	FUSIONCORE::Material FalloutPosterMaterial;
	FalloutPosterMaterial.PushTextureMap(FF_TEXTURE_DIFFUSE, &FalloutPoster);
	FalloutPosterMaterial.PushTextureMap(FF_TEXTURE_NORMAL, &FalloutPosterNormal);
	FalloutPosterMaterial.SetTiling(-1.0f);
	FalloutPosterMaterial.Roughness = 0.7f;
	//FalloutPosterMaterial.metalic = 0.8f;
	//FalloutPosterMaterial.Albedo = { 1.0f,0.0f,0.0f,1.0f };

	/*FUSIONCORE::Material IslandMaterial;
	IslandMaterial.PushTextureMap(TEXTURE_DIFFUSE0, IslandBaseColor);
	IslandMaterial.PushTextureMap(TEXTURE_NORMAL0, IslandNormal);
	IslandMaterial.PushTextureMap(TEXTURE_SPECULAR0, IslandRoughness);*/

	model1->GetTransformation().TranslateNoTraceBack({ 0.0f,0.0f,10.0f });
	model1->GetTransformation().ScaleNoTraceBack(glm::vec3(0.1f, 0.1f, 0.1f));

	MainCharac->GetTransformation().ScaleNoTraceBack(glm::vec3(0.1f, 0.1f, 0.1f));
	MainCharac->GetTransformation().RotateNoTraceBack(glm::vec3(0.0f, 1.0f, 0.0f), 90.0f);
	MainCharac->GetTransformation().TranslateNoTraceBack({ 4.0f,1.0f,-10.0f });

	//Stove->GetTransformation().ScaleNoTraceBack(glm::vec3(7.0f, 7.0f, 7.0f));
	Stove->GetTransformation().TranslateNoTraceBack({ 0.0f,4.0f,30.0f });
	//Stove->GetTransformation().RotateNoTraceBack(glm::vec3(0.0f, 1.0f, 0.0f), 70.0f);

	//wall->GetTransformation().ScaleNoTraceBack(glm::vec3(5.0f, 5.0f, 5.0f));
	wall->GetTransformation().TranslateNoTraceBack({ -60.0f,10.0f,0.0f });
	wall->GetTransformation().RotateNoTraceBack(glm::vec3(0.0f, 0.0f, 1.0f), 90.0f);

	//sofa->GetTransformation().ScaleNoTraceBack(glm::vec3(13.0f, 13.0f, 13.0f));
	sofa->GetTransformation().RotateNoTraceBack(glm::vec3(0.0f, 1.0f, 0.0f), 300.0f);
	sofa->GetTransformation().TranslateNoTraceBack({ -10.0f,-1.0f,-20.0f });

	FUSIONPHYSICS::CollisionBoxAABB Box1(model1->GetTransformation(), { 1.0f,1.0f,1.0f });
	FUSIONPHYSICS::CollisionBoxAABB StoveBox(Stove->GetTransformation(), glm::vec3(1.0f));
	FUSIONPHYSICS::CollisionBoxAABB tryBox({ 1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f });
	FUSIONPHYSICS::CollisionBoxPlane Plane({ 1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f });
	//FUSIONPHYSICS::CollisionBoxAABB IslandBox(Island->GetTransformation(), { 1.0f,1.0f,1.0f });
	//FUSIONPHYSICS::CollisionBox3DAABB SofaBox(sofa->GetTransformation(), { 0.7f,0.8f,1.0f });
	FUSIONPHYSICS::CollisionBoxAABB SofaBox(sofa->GetTransformation(), glm::vec3(1.0f));
	FUSIONPHYSICS::CollisionBoxPlane Plane2({ 1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f });
	FUSIONPHYSICS::CollisionBoxPlane floorBox({ 1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f });

	/*Island->PushChild(&IslandBox);
	Island->UpdateChildren();*/

	FUSIONCORE::Model IMPORTTEST("Resources\\floor\\grid.obj");
	//FUSIONCORE::MESHOPERATIONS::ImportObj("Resources\\floor\\grid.obj", IMPORTTEST);
	//IMPORTTEST.GetTransformation().Translate({ 7.0f,7.0f,0.0f });
	//IMPORTTEST.GetTransformation().ScaleNoTraceBack({ 0.05f,0.05f,0.05f });

	//Plane.GetTransformation().Scale({ 7.0f,7.0f ,7.0f });
	Plane.GetTransformation().Translate({ 0.7f,10.0f,0.0f });
	Plane.UpdateAttributes();
	//Plane2.GetTransformation().Scale({ 2.0f,2.0f ,2.0f });
	Plane2.GetTransformation().Translate({ 0.7f,0.0f,0.0f });

	//floorBox.GetTransformation().Scale(glm::vec3(200.0f));
	floorBox.GetTransformation().Translate({ 0.0f,-1.0f,0.0f });



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

	FUSIONCORE::Color FogColor(FF_COLOR_CORNFLOWER_BLUE);
	FogColor.Darker(0.3f);
	FUSIONCORE::SetEnvironment(*Shaders.DeferredPBRshader, 1.0f, FogColor.GetRGB(), FogColor.GetRGB());
	//FUSIONCORE::SetEnvironmentIBL(*Shaders.DeferredPBRshader, 2.0f, glm::vec3(BackGroundColor.x, BackGroundColor.y, BackGroundColor.z));
	FUSIONCORE::UseShaderProgram(0);

	const double TARGET_FRAME_TIME = 1.0 / TARGET_FPS;
	auto startTimePerSecond = std::chrono::high_resolution_clock::now();
	int fpsCounter = 0;

	glm::vec3 translateVector(0.0f, 0.0f, 0.01f);

	glm::ivec2 PrevWindowSize;
	glm::ivec2 PrevWindowPos;
	bool IsFullScreen = false;
	float AOamount = 0.5f;

	bool showDebug = false;

	std::vector<FUSIONCORE::Model*> StaticModels;
	StaticModels.push_back(Stove.get());
	StaticModels.push_back(sofa.get());
	StaticModels.push_back(grid.get());
	//StaticModels.push_back(&IMPORTTEST);
	StaticModels.push_back(&subdModel);
	StaticModels.push_back(Tower.get());
	StaticModels.push_back(shrub.get());
	StaticModels.push_back(Rock.get());
	//models.push_back(Island.get());
	StaticModels.push_back(model1.get());
	StaticModels.push_back(MainCharac.get());
	StaticModels.push_back(&Terrain);

	//models.push_back(Rock.get());
	std::vector<FUSIONCORE::Model*> DynamicModels;
	DynamicModels.push_back(model1.get());
	DynamicModels.push_back(MainCharac.get());

	//cubemap.SetCubeMapTexture(ShadowMap0.GetShadowMap());
	//ShadowMap0.BindShadowMapLight(Lights[0]);
	//ShadowMap1.BindShadowMapLight(Lights[1]);
	//ShadowMap2.BindShadowMapLight(Lights[2]);


	std::vector<FUSIONCORE::OmniShadowMap*> shadowMaps;
	//shadowMaps.push_back(&ShadowMap0);
	//shadowMaps.push_back(&ShadowMap1);
	//shadowMaps.push_back(&ShadowMap2);
	//shadowMaps.push_back(&ShadowMap3);

	sunShadowMap.BindShadowMapLight(Sun);
	//sunShadowMap2.BindShadowMapLight(Sun2);
	//sunShadowMap3.BindShadowMapLight(Sun3);

	std::vector<FUSIONCORE::CascadedDirectionalShadowMap*> cascadedShadowMaps;
	cascadedShadowMaps.push_back(&sunShadowMap);
	//cascadedShadowMaps.push_back(&sunShadowMap2);
	//cascadedShadowMaps.push_back(&sunShadowMap3);

	FUSIONCORE::UploadLightsShader(*Shaders.DeferredPBRshader);
	
	FUSIONCORE::Model animationModel("Resources\\taunt\\Jumping.fbx", false, true);
	animationModel.GetTransformation().ScaleNoTraceBack({ 0.01f,0.01f,0.01f });
	animationModel.GetTransformation().TranslateNoTraceBack({ 15.0f,-1.0f,0.0f });
	FUSIONCORE::Animation WalkingAnimation("Resources\\taunt\\MutantWalk.fbx", &animationModel);
	FUSIONCORE::Animation IdleAnimation("Resources\\taunt\\OrcIdle.fbx", &animationModel);
	//FUSIONCORE::Animation JumpingAnimation("Resources\\taunt\\Jumping.fbx", &animationModel);
	//FUSIONCORE::Animation RunningAnimation("Resources\\taunt\\Running.fbx", &animationModel);
	FUSIONCORE::Animator animator(&IdleAnimation);

	FUSIONCORE::Model Capsule("Resources\\Sphere.obj");
	FUSIONPHYSICS::CollisionBox Capsule0(Capsule.Meshes[0], Capsule.GetTransformation());
	//Capsule0.GetTransformation().ScaleNoTraceBack(glm::vec3(6.0f, 5.5f, 6.0f));
	Capsule0.GetTransformation().TranslateNoTraceBack({ 15.0f,2.0f,0.0f });
	animationModel.PushChild(&Capsule0);
	Capsule0.GetTransformation().TranslateNoTraceBack({ 0.0f,-3.0f,0.0f });
	StaticModels.push_back(&animationModel);

	std::vector<FUSIONCORE::Object*> ObjectInstances;
	ObjectInstances.push_back(&Capsule0);
	ObjectInstances.push_back(&TowerBox);
	ObjectInstances.push_back(&SofaBox);
	ObjectInstances.push_back(&StoveBox);
	ObjectInstances.push_back(&Box1);
	ObjectInstances.push_back(&Plane);
	ObjectInstances.push_back(&tryBox);
	//ObjectInstances.push_back(&IslandBox);
	ObjectInstances.push_back(&AlpCollisionBox);

	ObjectInstances.reserve(RockBoxes.size());
	for (size_t i = 0; i < RockBoxes.size(); i++)
	{
		ObjectInstances.push_back(RockBoxes[i].get());
	}

	ObjectInstances.reserve(TerrainCollisionBoxes.size());
	for (size_t i = 0; i < TerrainCollisionBoxes.size(); i++)
	{
		ObjectInstances.push_back(TerrainCollisionBoxes[i].get());
	}

	FUSIONCORE::VBO instanceVBO;
	auto DistibutedPoints = FUSIONCORE::MESHOPERATIONS::DistributePointsOnMeshSurfaceRandomized(grid->Meshes[0], grid->GetTransformation(), 1000, 118);
	FUSIONCORE::MESHOPERATIONS::FillInstanceDataVBO(instanceVBO, DistibutedPoints);
	shrub->SetInstanced(instanceVBO, DistibutedPoints.size() / 4);

	/*FUSIONCORE::VBO IslandVBO;
	auto IslandDistibutedPoints = FUSIONCORE::MESHOPERATIONS::DistributePointsOnMeshSurfaceRandomized(grid->Meshes[0], grid->GetTransformation(), 5, 200);
	FUSIONCORE::MESHOPERATIONS::FillInstanceDataVBO(IslandVBO, IslandDistibutedPoints);
	Island->SetInstanced(IslandVBO, IslandDistibutedPoints.size());*/
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

	//camera3d.SetMinMaxZoom(true, -15.0f, 15.0f);
	//camera3d.SetZoomSensitivity(3.0f);

	FUSIONPHYSICS::QuadNode headNode;

	FUSIONPHYSICS::ParticleEmitter emitter0(10000, *Shaders.ParticleInitializeShader,
		glm::vec4(1.0f, 1.0f, 1.0f, 0.9f),
		glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
		glm::vec3(-30.3f, 10.0f, -30.3f),
		glm::vec3(30.3f, 70.0f, 30.3f),
		glm::vec3(-0.2f),
		glm::vec3(0.2f, -0.8f, 0.2f),
		glm::vec3(-0.5f),
		glm::vec3(0.5f, -3.0f, 0.5f),
		glm::vec3(1.0f),
		1.0f,
		20.0f,
		0.0001f);

	FUSIONCORE::WorldTransform particleTransform;
	particleTransform.Scale(glm::vec3(0.003f));
	particleTransform.Rotate(glm::vec3(1.0f, 0.0f, 0.0f), 90);

	//std::shared_ptr<FILE> screenCaptureFile;

	FUSIONCORE::Color Pixel = FUSIONCORE::Color(glm::vec4(0.0f));

	FUSIONCORE::DecalDeferred decal0;
	//decal0.GetTransformation().Scale({ 4.0f, 6.0f, 1.0f });
	decal0.GetTransformation().Translate({ 15.0f, 10.0f, 0.0f });
	decal0.GetTransformation().Rotate({ 0.0f, 1.0f, 0.0f },90.0f);

	while (!FUSIONUTIL::WindowShouldClose(window))
	{
		float currentFrame = FUSIONUTIL::GetTime();
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
		sofa->UpdateChildren();
		Tower->UpdateChildren();

		model1->GetTransformation().Rotate({ 0.0f,1.0f,0.0f }, std::sin(time(0)));
		model1->UpdateChildren();

		animationModel.UpdateChildren();

		bool Collision = false;
		glm::vec3 direction;

		shrub->GetTransformation().RotateNoTraceBack({ 1.0f,0.0f,1.0f }, std::sin(time(0)) * 0.1f);

		FUSIONPHYSICS::UpdateQuadTreeWorldPartitioning(headNode, ObjectInstances, 2, 5);
		auto UniqueQuadObjects = Capsule0.GetUniqueQuadsObjects();
		glm::vec3 CapsuleQuadCenter = Capsule0.GetAssociatedQuads()[0]->Center;
		emitter0.GetTransformation().Position = { (CapsuleQuadCenter.x + Capsule0.GetTransformation().Position.x) * 0.5f,Capsule0.GetTransformation().Position.y , (CapsuleQuadCenter.z + Capsule0.GetTransformation().Position.z) * 0.5f };

		//LOG("UNIQUE BOX COUNT: " << UniqueQuadObjects.size());
		auto CapsuleModelMat = Capsule0.GetTransformation().GetModelMat4();
		for (const auto& Box : UniqueQuadObjects)
		{
			auto QuadModel = Box->DynamicObjectCast<FUSIONPHYSICS::CollisionBox*>();
			if (QuadModel != nullptr)
			{
				auto CollisionResponse = FUSIONPHYSICS::IsCollidingSAT(Capsule0, *QuadModel);
				if (CollisionResponse.first)
				{
					Collision = true;
					glm::vec3 ObjectPosition = FUSIONCORE::TranslateVertex(CapsuleModelMat, *Capsule0.GetTransformation().OriginPoint) -
						FUSIONCORE::TranslateVertex(QuadModel->GetTransformation().GetModelMat4(), *QuadModel->GetTransformation().OriginPoint);
					direction = glm::normalize(CollisionResponse.second);
				}
			}
		}

		static bool FirstFloorTouch = true;
		if (!FUSIONPHYSICS::IsCollidingSAT(floorBox, Capsule0).first)
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


		auto Front = glm::normalize(glm::vec3(camera3d.Orientation.x, 0.0f, camera3d.Orientation.z));
		auto Back = -Front;
		auto Right = glm::normalize(glm::cross(Front, camera3d.GetUpVector()));
		auto Left = -Right;

		//LOG("direction: " << Vec3<float>(direction));
		if (Collision)
		{
			if (glm::dot(-direction, camera3d.GetUpVector()) < 0.0f)
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

			if (FirstFloorTouch)
			{
				FirstFloorTouch = false;
			}

			//animationModel.GetTransformation().Translate(glm::normalize(glm::vec3(direction.x, 0.0f, direction.z)) * SPEED * 0.01f * deltaTime);
			direction = -direction;
			const float E = 0.0f;
			/*if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && glm::dot(direction, Front) <= E)
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
			}*/

			if (FUSIONUTIL::GetKey(window, FF_KEY_UP) == FF_GLFW_PRESS && glm::dot(direction, Front) <= E)
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
			else if (FUSIONUTIL::GetKey(window, FF_KEY_DOWN) == FF_GLFW_PRESS && glm::dot(direction, Back) <= E)
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
			else if (FUSIONUTIL::GetKey(window, FF_KEY_RIGHT) == FF_GLFW_PRESS && glm::dot(direction, Right) <= E)
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
			else if (FUSIONUTIL::GetKey(window, FF_KEY_LEFT) == FF_GLFW_PRESS && glm::dot(direction, Left) <= E)
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
		else
		{
			if (FUSIONUTIL::GetKey(window, FF_KEY_UP) == FF_GLFW_PRESS)
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
			else if (FUSIONUTIL::GetKey(window, FF_KEY_DOWN) == FF_GLFW_PRESS)
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
			else if (FUSIONUTIL::GetKey(window, FF_KEY_RIGHT) == FF_GLFW_PRESS)
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
			else if (FUSIONUTIL::GetKey(window, FF_KEY_LEFT) == FF_GLFW_PRESS)
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
		if (FUSIONUTIL::GetKey(window, FF_KEY_SPACE) == FF_GLFW_PRESS && !AllowJump && AllowReset)
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

		
		//animator.UpdateBlendedAnimation(&IdleAnimation, &WalkingAnimation, IdleWalkingBlendCoeff, deltaTime);

		static bool AllowPressF = true;
		if (!AllowPressF && FUSIONUTIL::GetKey(window, FF_KEY_F) == FF_GLFW_RELEASE)
		{
			AllowPressF = true;
		}
		if (FUSIONUTIL::GetKey(window, FF_KEY_F) == FF_GLFW_PRESS && AllowPressF)
		{
			AllowPressF = false;
			IsFullScreen = !IsFullScreen;
			if (!IsFullScreen)
			{
				glfwGetWindowPos(window, &PrevWindowPos.x, &PrevWindowPos.y);
				PrevWindowSize = WindowSize;
				const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
				glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
			}
			else
			{
				const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
				glfwSetWindowMonitor(window, NULL, PrevWindowPos.x, PrevWindowPos.y, PrevWindowSize.x, PrevWindowSize.y, mode->refreshRate);
			}
		}

		static bool DebugFBO = false;
		static bool AllowPressN = true;
		if (FUSIONUTIL::IsKeyPressedOnce(window, FF_KEY_N, AllowPressN))
		{
			DebugFBO = !DebugFBO;
		}


		//glfwGetWindowSize(window, &WindowSize.x, &WindowSize.y);
		WindowSize = ApplicationWindow.GetWindowSize();
		/*if (direction != glm::vec3(0.0f))
		{
		  camera3d.Orientation = direction;
		}*/
		/*static float FOV = 45.0f;
		if (FUSIONUTIL::IsKeyPressedOnce(window,FF_KEY_H))
		{
			FOV = 45.0f;
		}
		else if (FUSIONUTIL::IsKeyPressedOnce(window, FF_KEY_J))
		{
			FOV = 70.0f;
		}
		
		if (FUSIONUTIL::IsKeyPressedOnce(window, FF_KEY_K))
		{
			camera3d.Orientation = { 0.219249, -0.26206, -0.939816 };
		}*/
		//LOG("camera3d.Orientation: " << Vec3<float>(camera3d.Orientation));

		auto BlendedAnimation0 = [&]() { animator.UpdateBlendedAnimation(&IdleAnimation, &WalkingAnimation, IdleWalkingBlendCoeff, deltaTime); };
		auto CameraSetTarget = [&]() { 
			camera3d.UpdateCameraMatrix(45.0f, (float)WindowSize.x / (float)WindowSize.y, CAMERA_CLOSE_PLANE, CAMERA_FAR_PLANE, WindowSize);
		};

		threads.enqueue(BlendedAnimation0);
		threads.enqueue(CameraSetTarget);
		threads.wait();

		//LOG("CAMERA POSITION : " << Vec3<float>(camera3d.Position));
		//camera3d.UpdateCameraMatrix(45.0f, (float)WindowSize.x / (float)WindowSize.y, CAMERA_CLOSE_PLANE, CAMERA_FAR_PLANE, WindowSize);
		camera3d.SetTarget(&animationModel, 7.0f, { 0.0f,1.0f,0.0f });
		camera3d.HandleInputs(window, WindowSize, FF_CAMERA_LAYOUT_INDUSTRY_STANDARD,0.1f);

		camera2d.UpdateCameraMatrix({ 0.0f,0.0f,0.0f }, 1.0f, WindowSize);
		//LOG("MAX: " << FUSIONUTIL::GetMaxUniformBlockSize());
		//camera3d.UpdateCameraClusters(*Shaders.CameraClusterComputeShader, *Shaders.CameraLightCullingComputeShader);
		/*glm::vec4 SomePoint = glm::vec4(-0.0f, 0.0f, -20.1f, 1.0f);
		SomePoint = glm::inverse(camera3d.viewMat) * SomePoint;
		Isaac.GetTransformation().TranslationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(SomePoint));*/

		emitter0.UpdateParticleEmitter(*Shaders.ParticleUpdateComputeShader, *Shaders.ParticleSpawnComputeShader, deltaTime);

		//ShadowMap0.Draw(*Shaders.OmniShadowMapShader, Lights[0], models, camera3d);
		//ShadowMap1.Draw(*Shaders.OmniShadowMapShader, Lights[1], models, camera3d);
		//ShadowMap2.Draw(*Shaders.OmniShadowMapShader, Lights[2], models, camera3d);
		//ShadowMap3.Draw(*Shaders.OmniShadowMapShader, Lights[3], models, camera3d);

		//glBindFramebuffer(GL_FRAMEBUFFER, sunShadowMap.GetShadowMapFBO());
		//glClear(GL_DEPTH_BUFFER_BIT);
		FUSIONCORE::RefreshCascadedShadowMapBuffers(camera3d, cascadedShadowMaps, *Shaders.CascadedLightSpaceMatrixComputeShader);
		sunShadowMap.Draw(Shaders, camera3d, StaticModels, Sun);
		//sunShadowMap.Draw(Shaders,2, camera3d, models,Sun);
		//sunShadowMap.Draw(Shaders,1, camera3d, models,Sun);
		//sunShadowMap.Draw(Shaders,0, camera3d, models,Sun);
		//sunShadowMap2.Draw(Shaders,camera3d, models, Sun2);
		//sunShadowMap3.Draw(Shaders,0, camera3d, models, Sun3);

		Gbuffer.Bind();
		FUSIONUTIL::ClearFrameBuffer(0, 0, WindowSize.x, WindowSize.y, FF_COLOR_VOID);
		//FUSIONUTIL::GLClearColor(glm::vec4(0.0f));
		//FUSIONUTIL::GLClear(FF_CLEAR_BUFFER_BIT_GL_COLOR_BUFFER_BIT | FF_CLEAR_BUFFER_BIT_GL_DEPTH_BUFFER_BIT);
		FUSIONUTIL::EnableDepthTest();

		//Gbuffer.SetDrawModeDefaultRestricted();
		//FUSIONUTIL::GLviewport(0, 0, WindowSize.x, WindowSize.y);

		glm::vec2 PrevMousePos = glm::vec2(mousePos.x, mousePos.y);
		//glfwGetCursorPos(window, &mousePos.x, &mousePos.y);
		FUSIONUTIL::GetCursorPosition(window, mousePos.x, mousePos.y);
		glm::vec2 CurrentMousePos = glm::vec2(mousePos.x, mousePos.y);

		std::function<void()> shaderPrepe = [&]() {};
		std::function<void()> shaderPrepe1 = [&]() {};
		std::function<void()> shaderPrepe2 = [&]() {};

		auto animationMatrices = animator.GetFinalBoneMatrices();
		//IMPORTTEST.Draw(camera3d, *Shaders.GbufferShader, shaderPrepe, cubemap, shovelMaterial, shadowMaps, AOamount);
		//shrub->DrawDeferredInstanced(camera3d, *Shaders.InstancedGbufferShader, shaderPrepe, ShrubMaterial,instanceVBO,DistibutedPoints.size());
		shrub->DrawDeferredInstancedImportedMaterial(camera3d, *Shaders.InstancedGbufferShader, shaderPrepe, instanceVBO, DistibutedPoints.size());
		//if (FUSIONCORE::IsModelInsideCameraFrustum(*model1, camera3d))
		//Island->DrawDeferred(camera3d, *Shaders.GbufferShader, shaderPrepe, IslandMaterial);

		{
			Rock->DrawDeferredInstancedImportedMaterial(camera3d, *Shaders.InstancedGbufferShader, shaderPrepe, RockInstanceVBO, RockDistibutedPoints.size());
		}
		{
			model1->DrawDeferred(camera3d, *Shaders.GbufferShader, shaderPrepe, shovelMaterial);
		}
		MainCharac->DrawDeferred(camera3d, *Shaders.GbufferShader, shaderPrepe, shovelMaterial);
		grid->DrawDeferred(camera3d, *Shaders.GbufferShader, shaderPrepe, FloorMaterial);

		if (FUSIONCORE::IsModelInsideCameraFrustumSphere(*Stove, camera3d, 0.3f))
		{
			Stove->DrawDeferred(camera3d, *Shaders.GbufferShader, shaderPrepe, MirrorMaterial);
			//Stove->DrawDeferredIndirect(camera3d, *Shaders.GbufferShader, shaderPrepe, MirrorMaterial);

		}

		for (size_t i = 0; i < ImportedModels.size(); i++)
		{
			ImportedModels[i]->DrawDeferredImportedMaterial(camera3d, *Shaders.GbufferShader, shaderPrepe);
		}

		if (FUSIONCORE::IsModelInsideCameraFrustumSphere(*Tower, camera3d, 0.3f))
		{
			Tower->DrawDeferredImportedMaterial(camera3d, *Shaders.GbufferShader, shaderPrepe);

		}
		//Isaac.DrawDeferredImportedMaterial(camera3d, *Shaders.GbufferShader, shaderPrepe);

		//if (FUSIONCORE::IsObjectQuadInsideCameraFrustum(RockBox, camera3d))


		//Terrain.DrawDeferredImportedMaterial(camera3d, *Shaders.GbufferShader, shaderPrepe, AOamount);
		wall->DrawDeferred(camera3d, *Shaders.GbufferShader, shaderPrepe, WallMaterial);

		//if (!Collision)
		//{
		//}
	

		FUSIONCORE::Material redMaterial(0.3f, 0.6f, { 1.0f,0.0f,0.0f,1.0f });
		subdModel.DrawDeferred(camera3d, *Shaders.GbufferShader, shaderPrepe, redMaterial);
		if (FUSIONCORE::IsModelInsideCameraFrustumSphere(*sofa, camera3d, 0.3f))
		{
			sofa->DrawDeferred(camera3d, *Shaders.GbufferShader, shaderPrepe, SofaMaterial);
		}
		Capsule.DrawDeferred(camera3d, *Shaders.GbufferShader, shaderPrepe, FUSIONCORE::Material());
		Terrain.DrawDeferred(camera3d, *Shaders.GbufferShader, shaderPrepe, redMaterial);

		Gbuffer.SetDrawModeDecalPass();
		decal0.Draw(Gbuffer, FalloutPosterMaterial, camera3d, { WindowSize.x , WindowSize.y }, Shaders);
		Gbuffer.SetDrawModeDefault();
		animationModel.DrawDeferred(camera3d, *Shaders.GbufferShader, shaderPrepe, AnimationModelMaterial, animationMatrices);
		
		Gbuffer.Unbind();
		ScreenFrameBuffer.Bind();
		FUSIONUTIL::GLClearColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		FUSIONUTIL::GLClear(FF_CLEAR_BUFFER_BIT_GL_COLOR_BUFFER_BIT | FF_CLEAR_BUFFER_BIT_GL_DEPTH_BUFFER_BIT);
		//Gbuffer.DrawSSR(camera3d, *Shaders.SSRshader, [&]() {}, WindowSize);
		Gbuffer.DrawSceneDeferred(camera3d, *Shaders.DeferredPBRshader, [&]() {}, WindowSize, shadowMaps, cubemap, FF_COLOR_VOID, 0.3f);

		//glViewport(0, 0, WindowSize.x, WindowSize.y);
		FUSIONUTIL::GLviewport(0, 0, WindowSize.x, WindowSize.y);
		auto gbufferSize = Gbuffer.GetFBOSize();
		FUSIONCORE::CopyDepthInfoFBOtoFBO(Gbuffer.GetFBO(), { gbufferSize.x ,gbufferSize.y }, ScreenFrameBuffer.GetFBO());
		ScreenFrameBuffer.Bind();

		for (size_t i = 0; i < Lights.size(); i++)
		{
			Lights[i].Draw(camera3d, *Shaders.LightShader);
		}

		if (FUSIONUTIL::GetMouseKey(window, FF_GLFW_MOUSE_BUTTON_RIGHT) == FF_GLFW_PRESS)
		{
			Pixel = FUSIONCORE::ReadFrameBufferPixel(mousePos.x, mousePos.y, FF_FRAMEBUFFER_MODEL_ID_IMAGE_ATTACHMENT, FF_PIXEL_FORMAT_GL_RED, { WindowSize.x, WindowSize.y });
		}

		emitter0.DrawParticles(*Shaders.ParticleRenderShader, grid->Meshes[0], particleTransform, camera3d);
		cubemap.Draw(camera3d, WindowSize);

		size_t SelectedObjectID = static_cast<size_t>(Pixel.GetRed());
		if (SelectedObjectID > 0)
		{
			auto PixelObject = FUSIONCORE::GetObject(SelectedObjectID);
			LOG_PARAMETERS(PixelObject);
			if (PixelObject && PixelObject->GetObjectType() == FUSIONCORE::FF_OBJECT_TYPE_MODEL)
			{
				auto CastObject = PixelObject->DynamicObjectCast<FUSIONCORE::Model*>();
				if (FUSIONUTIL::GetMouseKey(window, FF_GLFW_MOUSE_BUTTON_LEFT) == FF_GLFW_PRESS)
				{
					glm::vec4 PrevMouseWorldPos = glm::inverse(camera3d.viewMat) * glm::vec4(PrevMousePos.x, PrevMousePos.y, 0.0f, 1.0f);
					glm::vec4 CurrentMouseWorldPos = glm::inverse(camera3d.viewMat) * glm::vec4(CurrentMousePos.x, CurrentMousePos.y, 0.0f, 1.0f);
					glm::vec3 DeltaMouse = CurrentMouseWorldPos - PrevMouseWorldPos;
					CastObject->GetTransformation().Translate(glm::vec3(DeltaMouse.x, -DeltaMouse.y, DeltaMouse.z) / 10.0f);
				}

				FUSIONUTIL::GLPolygonMode(FF_CULL_FACE_MODE_GL_FRONT_AND_BACK, FF_GL_LINE);
				//glPolygonMode(FF_CULL_FACE_MODE_GL_FRONT_AND_BACK, FF_GL_LINE);
				CastObject->DrawDeferred(camera3d, *Shaders.LightShader, shaderPrepe, FUSIONCORE::Material());
				FUSIONUTIL::GLPolygonMode(FF_CULL_FACE_MODE_GL_FRONT_AND_BACK, FF_GL_FILL);
				//glPolygonMode(FF_CULL_FACE_MODE_GL_FRONT_AND_BACK, FF_GL_FILL);
			}
			else if (PixelObject && PixelObject->GetObjectType() == FUSIONCORE::FF_OBJECT_TYPE_LIGHT)
			{
				auto CastObject = PixelObject->DynamicObjectCast<FUSIONCORE::Light*>();
				if (FUSIONUTIL::GetMouseKey(window, FF_GLFW_MOUSE_BUTTON_LEFT) == FF_GLFW_PRESS)
				{
					glm::vec4 PrevMouseWorldPos = glm::inverse(camera3d.viewMat) * glm::vec4(PrevMousePos.x, PrevMousePos.y, 0.0f, 1.0f);
					glm::vec4 CurrentMouseWorldPos = glm::inverse(camera3d.viewMat) * glm::vec4(CurrentMousePos.x, CurrentMousePos.y, 0.0f, 1.0f);
					glm::vec3 DeltaMouse = CurrentMouseWorldPos - PrevMouseWorldPos;
					//CastObject->GetTransformation().Translate(glm::vec3(DeltaMouse.x, -DeltaMouse.y, DeltaMouse.z) / 10.0f);
					//::UploadLightsShader(*Shaders.DeferredPBRshader);
				}
			}
		}
#ifdef ENGINE_DEBUG

		static bool AllowG = true;
		if (FUSIONUTIL::IsKeyPressedOnce(window, FF_KEY_G, AllowG))
		{
			showDebug = !showDebug;
		}
		if (showDebug)
		{
			/*Capsule0.DrawBoxMesh(camera3d, *Shaders.LightShader);
			Box1.DrawBoxMesh(camera3d, *Shaders.LightShader);
			SofaBox.DrawBoxMesh(camera3d, *Shaders.LightShader);
			StoveBox.DrawBoxMesh(camera3d, *Shaders.LightShader);
			tryBox.DrawBoxMesh(camera3d, *Shaders.LightShader);
			Plane.DrawBoxMesh(camera3d, *Shaders.LightShader);
			Plane2.DrawBoxMesh(camera3d, *Shaders.LightShader);
			floorBox.DrawBoxMesh(camera3d, *Shaders.LightShader);*/

			for (auto& Box : ObjectInstances)
			{
				Box->DynamicObjectCast<FUSIONPHYSICS::CollisionBox*>()->DrawBoxMesh(camera3d, *Shaders.LightShader);
			}
			/*for (auto& Box : TerrainCollisionBoxes)
			{
				Box.DrawBoxMesh(camera3d, *Shaders.LightShader);
			}*/

			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			FUSIONUTIL::GLPolygonMode(FF_CULL_FACE_MODE_GL_FRONT_AND_BACK, FF_GL_LINE);
			decal0.VisualiseDecalCage(camera3d, *Shaders.LightShader, { 1.0f,0.0f,0.0f });
			subdModel.DrawDeferred(camera3d, *Shaders.GbufferShader, shaderPrepe, FUSIONCORE::Material());
			FUSIONUTIL::GLPolygonMode(FF_CULL_FACE_MODE_GL_FRONT_AND_BACK, FF_GL_FILL);
			//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			FUSIONPHYSICS::VisualizeQuadTree(headNode, camera3d, *Shaders.LightShader, FF_COLOR_RED);
		}
#endif

		//glDisable(GL_DEPTH_TEST);

		//FUSIONCORE::SHAPES::DrawRectangleTextured(bearNormal, {0.0f,1.0f}, {1.0f,0.1f}, 0.0f, camera2d, *Shaders.ShapeTexturedShader);


		//FUSIONCORE::SHAPES::DrawRectangle(glm::vec4(glm::vec3(FF_COLOR_CHARCOAL), 0.5f), { 0.0f,1.0f }, { 3.0f,0.1f }, 0.0f, camera2d, Shaders);


		//FUSIONCORE::SHAPES::DrawRectangle(FF_COLOR_BLUSH_PINK, { 0.5f,0.0f }, { 0.5f,0.5f }, 0.0f, camera2d,Shaders);
		//FUSIONCORE::SHAPES::DrawTriangle(FF_COLOR_AQUAMARINE, { 0.0f,0.0f }, { 0.5f,0.5f }, 0.0f, camera2d, *Shaders.ShapeBasicShader);
		//FUSIONCORE::SHAPES::DrawTriangleTextured(bearNormal, { 0.0f,0.0f }, { 0.5f,0.5f }, 0.0f, camera2d, *Shaders.ShapeTexturedShader);
		//FUSIONCORE::SHAPES::DrawHexagon(FF_COLOR_AMETHYST, { 0.0f,0.0f }, { 0.5f,0.5f }, 0.0f, camera2d, Shaders);
		//FUSIONCORE::SHAPES::DrawHexagonTextured(FloorAlbedo, { 0.0f,0.0f }, { 0.5f,0.5f }, 0.0f, camera2d,Shaders);
		//glEnable(GL_DEPTH_TEST);

		//ScreenFrameBuffer.Unbind();
		////glBindFramebuffer(GL_FRAMEBUFFER, 0);
		FUSIONUTIL::GLBindFrameBuffer(FF_GL_FRAMEBUFFER, 0);

		auto DebugFbo = [&]() {
			Shaders.FBOShader->setBool("Debug", DebugFBO);
		};

		ScreenFrameBuffer.Draw(camera3d, *Shaders.FBOShader, DebugFbo, WindowSize, true, 0.7f, 0.1f, 2.0f, 1.7f, 1.6f);

		static bool AllowPressP = true;
		if (FUSIONUTIL::IsKeyPressedOnce(window, FF_KEY_P, AllowPressP))
		{
			FUSIONCORE::SaveFrameBufferImage(WindowSize.x, WindowSize.y, "ScreenShot.png", GL_COLOR_ATTACHMENT0);
		}

		//glfwPollEvents();
		ApplicationWindow.SwapBuffers();
		ApplicationWindow.PollEvents();
		//FUSIONCORE::ClearObjectUpToDateBoundingBoxes();


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
	FUSIONCORE::TerminateCascadedShadowMapTextureArray();

	ScreenFrameBuffer.clean();
	Gbuffer.clean();

	
	//IslandMaterial.Clean();


	ApplicationWindow.TerminateWindow();
	FUSIONUTIL::TerminateGLFW();
	return 0;
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





