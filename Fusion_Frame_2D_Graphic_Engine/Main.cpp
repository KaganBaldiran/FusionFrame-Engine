#include <glew.h>
#include <glfw3.h>
#include <iostream>
#include "FusionUtility/Log.h"
#include "FusionOpengl/Shader.h"
#include "FusionUtility/Thread.h"
#include <memory>
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
#include <chrono>
#include <thread>
#include "FusionOpengl/Color.hpp"
#include "FusionUtility/StopWatch.h"
#include "FusionOpengl/Cubemap.h"

#define TARGET_FPS 144
#define ENGINE_DEBUG

#ifndef ENGINE_DEBUG
#define ENGINE_RELEASE
#endif 

namespace KAGAN_PAVLO
{
	int EngineMain()
	{
		const int width = 1000;
		const int height = 1000;

		GLFWwindow* window = FUSIONUTIL::InitializeWindow(width, height, "FusionFrame Engine");

		FUSIONUTIL::DefaultShaders Shaders;
		FUSIONUTIL::InitializeDefaultShaders(Shaders);

		FUSIONOPENGL::CubeMap cubemap(*Shaders.CubeMapShader);
		FUSIONOPENGL::ImportCubeMap("Resources/kiara_5_noon_2k.hdr", 1024, cubemap, Shaders.HDRIShader->GetID(), Shaders.ConvolutateCubeMapShader->GetID(), Shaders.PreFilterCubeMapShader->GetID());

		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		FUSIONOPENGL::FrameBuffer ScreenFrameBuffer(mode->width, mode->height);

		FUSIONOPENGL::LightIcon = std::make_unique<FUSIONOPENGL::Model>("Resources/LightIcon.fbx");

		FUSIONOPENGL::Camera2D camera;
		FUSIONOPENGL::Camera3D camera3d;

		FUSIONOPENGL::Texture2D texture("Resources/raccoon.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);
		FUSIONOPENGL::Texture2D ShovelDiffuse("Resources/texture_diffuse.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
		FUSIONOPENGL::Texture2D ShovelNormal("Resources/texture_normal.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
		FUSIONOPENGL::Texture2D ShovelSpecular("Resources/texture_specular.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
		FUSIONOPENGL::Texture2D KnightDiffuse("C:\\Users\\kbald\\Desktop\\Models\\texture_diffuse.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);

		Vec2<int> WindowSize;
		Vec2<double> mousePos;

		glm::vec3 Target(0.0f);

		camera3d.SetPosition(glm::vec3(12.353, 13.326, 15.2838));
		camera3d.SetOrientation(glm::vec3(-0.593494, -0.648119, -0.477182));

		FUSIONOPENGL::Light light0({ 0.0f,2.0f,0.0f }, FUSIONOPENGL::Color(FF_COLOR_CINNAMON).GetRGB(), 9.0f);
		FUSIONOPENGL::Light light1({ 3.0f,2.0f,2.0f }, FUSIONOPENGL::Color(FF_COLOR_LIME).GetRGB(), 5.0f);
		FUSIONOPENGL::Light light2({ 2.0f,-4.0f,0.0f }, FUSIONOPENGL::Color(FF_COLOR_AMETHYST).GetRGB(), 5.0f);
		FUSIONOPENGL::Light light3({ 3.0f,-4.0f,7.0f }, FUSIONOPENGL::Color(FF_COLOR_LIME_SHERBET).GetRGB(), 5.0f);

		FUSIONUTIL::ThreadPool threads(5,20);
//#define ASYNC
#define NOTASYNC

#ifdef ASYNC
		FUSIONUTIL::Timer stopwatch;
		stopwatch.Set();
		std::unique_ptr<FUSIONOPENGL::Model> model0;
		std::unique_ptr<FUSIONOPENGL::Model> model1;
		std::unique_ptr<FUSIONOPENGL::Model> WateringPot;

		auto import1 = [&]() { model0 = std::make_unique<FUSIONOPENGL::Model>("Resources\\shovel2.obj" , true); };
		auto import2 = [&]() { model1 = std::make_unique<FUSIONOPENGL::Model>("Resources\\shovel2.obj", true); };
		auto import3 = [&]() { WateringPot = std::make_unique<FUSIONOPENGL::Model>("Resources\\shovel2.obj" , true); };

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
		std::unique_ptr<FUSIONOPENGL::Model> model0;
		std::unique_ptr<FUSIONOPENGL::Model> model1;
		std::unique_ptr<FUSIONOPENGL::Model> WateringPot;

		model0 = std::make_unique<FUSIONOPENGL::Model>("Resources\\shovel2.obj"); 
		model1 = std::make_unique<FUSIONOPENGL::Model>("Resources\\shovel2.obj");
		WateringPot = std::make_unique<FUSIONOPENGL::Model>("Resources\\shovel2.obj");
		LOG("Duration: " << stopwatch.GetMiliseconds());
#endif // DEBUG
		FUSIONOPENGL::Material shovelMaterial;
		shovelMaterial.PushTextureMap(TEXTURE_DIFFUSE0, ShovelDiffuse);
		shovelMaterial.PushTextureMap(TEXTURE_NORMAL0, ShovelNormal);
		shovelMaterial.PushTextureMap(TEXTURE_SPECULAR0, ShovelSpecular);

		FUSIONOPENGL::Material knightMaterial;
		knightMaterial.PushTextureMap(TEXTURE_DIFFUSE0, KnightDiffuse);
		
		model1->GetTransformation().TranslateNoTraceBack({ 0.0f,0.0f,10.0f });
		model1->GetTransformation().ScaleNoTraceBack(glm::vec3(0.15f, 0.15f, 0.15f));
		model0->GetTransformation().ScaleNoTraceBack(glm::vec3(0.15f, 0.15f, 0.15f));
		model0->GetTransformation().RotateNoTraceBack(glm::vec3(0.0f, 1.0f, 0.0f) , 100.0f);

		WateringPot->GetTransformation().ScaleNoTraceBack(glm::vec3(0.1f, 0.1f, 0.1f));
		WateringPot->GetTransformation().TranslateNoTraceBack({ -10.0f,0.0f,10.0f });

		FUSIONPHYSICS::CollisionBox3DAABB Box1(model1->GetTransformation(), { 1.0f,1.1f,1.0f });
		FUSIONPHYSICS::CollisionBox3DAABB Box0(model0->GetTransformation(), { 1.0f,1.1f,1.0f });
		FUSIONPHYSICS::CollisionBox3DAABB WateringPotBox(WateringPot->GetTransformation(), { 1.0f,1.0f,1.0f });

		model0->PushChild(&Box0);
		model1->PushChild(&Box1);

		model0->UpdateChildren();

		glm::vec4 BackGroundColor(175.0f / 255.0f, 225.0f / 255.0f, 225.0f / 255.0f, 1.0f);

		Shaders.PBRShader->use();
		//FUSIONOPENGL::SetEnvironment(*Shaders.PBRShader, 5.0f, glm::vec3(BackGroundColor.x, BackGroundColor.y, BackGroundColor.z),
			//glm::vec3(BackGroundColor.x, BackGroundColor.y, BackGroundColor.z));

		FUSIONOPENGL::SetEnvironmentIBL(*Shaders.PBRShader, 5.0f,glm::vec3(BackGroundColor.x, BackGroundColor.y, BackGroundColor.z));

		FUSIONOPENGL::UseShaderProgram(0);

		const double TARGET_FRAME_TIME = 1.0 / TARGET_FPS;
		auto startTimePerSecond = std::chrono::high_resolution_clock::now();
		int fpsCounter = 0;

		glm::vec3 translateVector(0.0f, 0.0f, 0.01f);

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

			camera.UpdateCameraMatrix(Target, 0.5f, WindowSize);
			camera3d.UpdateCameraMatrix(45.0f, (float)WindowSize.x / (float)WindowSize.y, 0.1f, 100.0f, WindowSize);
			camera3d.HandleInputs(window, WindowSize , FF_CAMERA_LAYOUT_FIRST_PERSON , 0.3f);

			auto ShaderPrep = [&]()
			{
				glUniform2f(glGetUniformLocation(Shaders.BasicShader->GetID(), "ScreenSize"), WindowSize.x, WindowSize.y);
			};

			std::function<void()> shaderPrepe = [&]() {
				model0->GetTransformation().SetModelMatrixUniformLocation(Shaders.PBRShader->GetID(), "model");
				FUSIONOPENGL::SendLightsShader(*Shaders.PBRShader);
				Shaders.PBRShader->setFloat("ModelID", model0->GetModelID());
			};
			std::function<void()> shaderPrepe1 = [&]() {
				model1->GetTransformation().SetModelMatrixUniformLocation(Shaders.PBRShader->GetID(), "model");
				FUSIONOPENGL::SendLightsShader(*Shaders.PBRShader);
				Shaders.PBRShader->setFloat("ModelID", (float)model1->GetModelID());
			};

			std::function<void()> shaderPrepe2 = [&]() {
				WateringPot->GetTransformation().SetModelMatrixUniformLocation(Shaders.PBRShader->GetID(), "model");
				FUSIONOPENGL::SendLightsShader(*Shaders.PBRShader);
				Shaders.PBRShader->setFloat("ModelID", (float)WateringPot->GetModelID());
			};

			WateringPot->Draw(camera3d, *Shaders.PBRShader, knightMaterial, shaderPrepe2, cubemap, 0.4f);
			cubemap.Draw(camera3d, WindowSize.Cast<float>());
			
            #ifdef ENGINE_DEBUG
			light0.Draw(camera3d, *Shaders.LightShader);
			light1.Draw(camera3d, *Shaders.LightShader);
			light2.Draw(camera3d, *Shaders.LightShader);
			light3.Draw(camera3d, *Shaders.LightShader);
			Box0.DrawBoxMesh(camera3d, *Shaders.LightShader);
			Box1.DrawBoxMesh(camera3d, *Shaders.LightShader);
			WateringPotBox.DrawBoxMesh(camera3d, *Shaders.LightShader);
            #endif

			model1->GetTransformation().Rotate({ 0.0f,1.0f,0.0f }, std::sin(time(0)) );
			model1->UpdateChildren();
			model0->GetTransformation().Rotate({ 0.0f,1.0f,0.0f }, std::sin(time(0)) );
			//model0.GetTransformation().Translate({ 0.0f,0.0f,std::sin(time(0)) / 5.0f });
			model0->UpdateChildren();

			if (FUSIONPHYSICS::IsCollidingSAT(Box1, Box0))
			{
			}
			model0->Draw(camera3d, *Shaders.PBRShader, shovelMaterial, shaderPrepe, cubemap,0.4f);
			model1->Draw(camera3d, *Shaders.PBRShader, shovelMaterial, shaderPrepe1, cubemap, 0.4f);

			ScreenFrameBuffer.Unbind();
			ScreenFrameBuffer.Draw(camera3d, *Shaders.FBOShader, [&]() {},WindowSize,false,0.09f,2.0f);

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
				//LOG("FPS: " <<fps);
				fpsCounter = 0;
				startTimePerSecond = std::chrono::high_resolution_clock::now();
			}

		}

		FUSIONUTIL::DisposeDefaultShaders(Shaders);

		ScreenFrameBuffer.clean();
		Box0.GetBoxMesh()->Clean();
		Box1.GetBoxMesh()->Clean();
		WateringPotBox.GetBoxMesh()->Clean();

		glfwTerminate();
		LOG_INF("Window terminated!");
		return 0;
	}
}

int main(int argc, char* argv[])
{
	return KAGAN_PAVLO::EngineMain();
}