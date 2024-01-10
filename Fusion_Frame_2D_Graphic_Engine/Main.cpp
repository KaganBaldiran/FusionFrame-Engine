#include <glew.h>
#include <glfw3.h>
#include <iostream>
#include "Log.h"
#include "Shader.h"
#include "Thread.h"
#include <memory>
#include "Buffer.h"
#include "VectorMath.h"
#include "Texture.h"
#include "Initialize.h"
#include "Camera.h"
#include "Mesh.h"
#include "SDL_CUSTOM.hpp"
#include "SDL_camera2d.hpp"
#include "DrawingFunctions.hpp"
#include "Model.hpp"
#include "Light.hpp"
#include "Framebuffer.hpp"
#include "Physics.hpp"
#include <chrono>
#include <thread>
#include "Color.hpp"

#define TARGET_FPS 144

#define ENGINE_DEBUG

#ifndef ENGINE_DEBUG
#define ENGINE_RELEASE
#endif 

#define OPENGL 
#ifdef OPENGL

namespace KAGAN_PAVLO
{
	int EngineMain()
	{
		const int width = 1000;
		const int height = 1000;

		GLFWwindow* window = INIT::InitializeWindow(width, height, "FusionFrame Engine");

		Shader BasicShader("Shaders/Basic.vs", "Shaders/Basic.fs");
		Shader PixelShader("Shaders/PixelShader.vs", "Shaders/PixelShader.fs");
		Shader MeshBasicShader("Shaders/MeshBasic.vs", "Shaders/MeshBasic.fs");
		Shader LightShader("Shaders/Light.vs", "Shaders/Light.fs");
		Shader FBOShader("Shaders/FBO.vs", "Shaders/FBO.fs");
		Shader PBRShader("Shaders/PBR.vs", "Shaders/PBR.fs");

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
		FUSIONOPENGL::Light light1({ 0.0f,2.0f,2.0f }, FUSIONOPENGL::Color(FF_COLOR_LIME).GetRGB(), 5.0f);
		FUSIONOPENGL::Light light2({ 2.0f,-4.0f,0.0f }, FUSIONOPENGL::Color(FF_COLOR_AMETHYST).GetRGB(), 5.0f);
		FUSIONOPENGL::Light light3({ 1.0f,-4.0f,2.0f }, FUSIONOPENGL::Color(FF_COLOR_LIME_SHERBET).GetRGB(), 5.0f);

		FUSIONOPENGL::Model model0("Resources\\shovel2.obj");
		FUSIONOPENGL::Model model1("Resources\\shovel2.obj");
		FUSIONOPENGL::Model WateringPot("Resources\\shovel2.obj");

		FUSIONOPENGL::Material shovelMaterial;
		shovelMaterial.PushTextureMap(TEXTURE_DIFFUSE0, ShovelDiffuse);
		shovelMaterial.PushTextureMap(TEXTURE_NORMAL0, ShovelNormal);
		shovelMaterial.PushTextureMap(TEXTURE_SPECULAR0, ShovelSpecular);

		FUSIONOPENGL::Material knightMaterial;
		knightMaterial.PushTextureMap(TEXTURE_DIFFUSE0, KnightDiffuse);

		model1.GetTransformation().TranslateNoTraceBack({ 0.0f,0.0f,10.0f });
		model1.GetTransformation().ScaleNoTraceBack(glm::vec3(0.15f, 0.15f, 0.15f));
		model0.GetTransformation().ScaleNoTraceBack(glm::vec3(0.15f, 0.15f, 0.15f));
		model0.GetTransformation().RotateNoTraceBack(glm::vec3(0.0f, 1.0f, 0.0f) , 100.0f);

		WateringPot.GetTransformation().ScaleNoTraceBack(glm::vec3(0.1f, 0.1f, 0.1f));
		WateringPot.GetTransformation().TranslateNoTraceBack({ -10.0f,0.0f,10.0f });

		FUSIONPHYSICS::CollisionBox3DAABB Box1(model1.GetTransformation(), { 1.0f,1.1f,1.0f });
		FUSIONPHYSICS::CollisionBox3DAABB Box0(model0.GetTransformation(), { 1.0f,1.1f,1.0f });
		FUSIONPHYSICS::CollisionBox3DAABB WateringPotBox(WateringPot.GetTransformation(), { 1.0f,1.0f,1.0f });

		model0.PushChild(&Box0);
		model1.PushChild(&Box1);

		model0.UpdateChildren();

		glm::vec4 BackGroundColor(175.0f / 255.0f, 225.0f / 255.0f, 225.0f / 255.0f, 1.0f);

		PBRShader.use();
		FUSIONOPENGL::SetEnvironment(PBRShader, 5.0f, glm::vec3(BackGroundColor.x, BackGroundColor.y, BackGroundColor.z),
			glm::vec3(BackGroundColor.x, BackGroundColor.y, BackGroundColor.z));
		UseShaderProgram(0);

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
			camera3d.HandleInputs(window, WindowSize);
			camera3d.UpdateCameraMatrix(45.0f, (float)WindowSize.x / (float)WindowSize.y, 0.1f, 100.0f, WindowSize);

			auto ShaderPrep = [&]()
			{
				glUniform2f(glGetUniformLocation(BasicShader.GetID(), "ScreenSize"), WindowSize.x, WindowSize.y);
			};

			std::function<void()> shaderPrepe = [&]() {
				model0.GetTransformation().SetModelMatrixUniformLocation(PBRShader.GetID(), "model");
				FUSIONOPENGL::SendLightsShader(PBRShader);
				PBRShader.setFloat("ModelID", model0.GetModelID());
			};
			std::function<void()> shaderPrepe1 = [&]() {
				model1.GetTransformation().SetModelMatrixUniformLocation(PBRShader.GetID(), "model");
				FUSIONOPENGL::SendLightsShader(PBRShader);
				PBRShader.setFloat("ModelID", (float)model1.GetModelID());
			};

			std::function<void()> shaderPrepe2 = [&]() {
				WateringPot.GetTransformation().SetModelMatrixUniformLocation(PBRShader.GetID(), "model");
				FUSIONOPENGL::SendLightsShader(PBRShader);
				PBRShader.setFloat("ModelID", (float)WateringPot.GetModelID());
			};

			WateringPot.Draw(camera3d, PBRShader, knightMaterial, shaderPrepe2);

			
            #ifdef ENGINE_DEBUG
			light0.Draw(camera3d, LightShader);
			light1.Draw(camera3d, LightShader);
			light2.Draw(camera3d, LightShader);
			light3.Draw(camera3d, LightShader);
			Box0.DrawBoxMesh(camera3d, LightShader);
			Box1.DrawBoxMesh(camera3d, LightShader);
			WateringPotBox.DrawBoxMesh(camera3d, LightShader);
            #endif

			model1.GetTransformation().Rotate({ 0.0f,1.0f,0.0f }, std::sin(time(0)) );
			model1.UpdateChildren();
			model0.GetTransformation().Rotate({ 0.0f,1.0f,0.0f }, std::sin(time(0)) );
			//model0.GetTransformation().Translate({ 0.0f,0.0f,std::sin(time(0)) / 5.0f });
			model0.UpdateChildren();

			//LOG("LOCATION : " << Vec3<float>(camera3d.Position) << " " << Vec3<float>(WateringPot.GetTransformation().Position));

			if (FUSIONPHYSICS::IsCollidingSAT(Box1, Box0))
			{
				model1.Draw(camera3d, PBRShader, shovelMaterial, shaderPrepe1);
			}

			model0.Draw(camera3d,PBRShader,shovelMaterial, shaderPrepe);

			ScreenFrameBuffer.Unbind();

			auto fboPrep = [&]() {};
			ScreenFrameBuffer.Draw(camera3d,FBOShader, fboPrep,WindowSize,false,0.09f,2.0f);

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

		DeleteShaderProgram(PixelShader.GetID());
		DeleteShaderProgram(BasicShader.GetID());
		DeleteShaderProgram(MeshBasicShader.GetID());
		DeleteShaderProgram(LightShader.GetID());
		DeleteShaderProgram(FBOShader.GetID());
		DeleteShaderProgram(PBRShader.GetID());

		ScreenFrameBuffer.clean();
		Box0.GetBoxMesh()->Clean();
		Box1.GetBoxMesh()->Clean();
		WateringPotBox.GetBoxMesh()->Clean();

		glfwTerminate();
		LOG_INF("Window terminated!");
		return 0;
	}
}

#elif defined SDL

namespace KAGAN_PAVLO
{
	int EngineMain()
	{
		const int width = 1000;
		const int height = 1000;

		const int fps = 144;
		const int framedelay = 1000 / fps;

		Uint32 framestart;
		int frametime;

		SDL_Renderer* renderer = NULL;
		SDL_DisplayMode dpm;
		bool isGameRunning = false;
		std::pair<int, SDL_Window*>  window = SDL_CUSTOM::init("FusionFrame Engine", &renderer, width, height,false,dpm);

		if (window.first == SDL_ERROR || renderer == NULL)
		{
			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window.second);
			IMG_Quit();
			SDL_Quit();
			return -1;
		}
		else if (window.first == SDL_SUCCESS)
		{
			isGameRunning = true;
		}

		Vec2<int> TextureSize;
		Vec2<int> ScarfySize;
		SDL_Texture* texture0 = SDL_CUSTOM::LoadInTexture("Resources/raccoon.png", TextureSize,renderer);
		SDL_Texture* scarfy = SDL_CUSTOM::LoadInTexture("Resources/scarfy.png", ScarfySize, renderer);

		Vec2<int> indiciesLeft[] = {
			{0,0},
			{1,0},
			{2,0},
			{3,0},
			{4,0},
			{5,0},
		};

		std::vector<Vec2<int>> scarfyIndiciesLeft(indiciesLeft, indiciesLeft + 6);

		SDL_CUSTOM::TextureObject scarfyObj(scarfy, ScarfySize);
		scarfyObj.DestRec = { width / 2, height / 2 , (int)(ScarfySize.x / 12.0f) , ScarfySize.y };
		SDL_RendererFlip flipScarfy = SDL_FLIP_NONE;
		bool ScarfyPlayAnimation = true;

		
		Vec2<int> WindowSize;
		Vec2<int> MousePos;
		Vec2<float> MouseDelta;
		float zoom = 1.0f;
		float sensitivity = 0.2f;

		Vec4<float> clearColor({ 255, 255, 255, 255 });
		SDL_Event GameEvent;

		SDL_CAMERA2D::SDLCamera2D camera;

		FusionDrawSDL::DrawCircle(200, 200, 50, { 255, 0, 0, 255 });

		std::vector<Vec2<int>> polygon0;
		std::vector<Vec2<int>> triangle;

		std::vector<unsigned int> polygon0indices;

		triangle.push_back({ 100, 400 });
		triangle.push_back({ 300, 200 });
		
		triangle.push_back({ 500, 400 });
		triangle.push_back({ 100, 400 });

		int hexagonRadius = 50;
		for (int i = 0; i < 6; ++i) {
			float angle = i * (2 * M_PI / 6); 
			int x = 200 + static_cast<int>(hexagonRadius * cos(angle));
			int y = 200 + static_cast<int>(hexagonRadius * sin(angle));
			polygon0.push_back({ x, y });
		}

		polygon0.push_back(polygon0[0]);

		FusionDrawSDL::DrawPolygon(polygon0, polygon0indices, { 255, 0, 0, 255 });
		FusionDrawSDL::DrawPolygon(triangle, polygon0indices, { 0, 255, 0, 255 });

		FusionDrawSDL::DrawLine({ 100,300 }, { 200,200 }, { 0, 0, 255, 255 });

		int speed = 5;

		while (isGameRunning)
		{
			ScarfyPlayAnimation = false;
			MouseDelta({ 0,0 });
			framestart = SDL_GetTicks64();

			SDL_GetWindowSize(window.second, &WindowSize.x, &WindowSize.y);
			SDL_CUSTOM::HandleEvent(isGameRunning, GameEvent);

			if (GameEvent.type == SDL_MOUSEMOTION)
			{
				Vec2<int> mouseTemp(MousePos);
				SDL_GetMouseState(&MousePos.x, &MousePos.y);
				MouseDelta((MousePos - mouseTemp).Cast<float>());
					
				//LOG("MouseDelta: " << MouseDelta);
			}
			else if(GameEvent.type == SDL_MOUSEWHEEL)
			{
				if (GameEvent.wheel.y > 0)
				{
					zoom += sensitivity;
				}
				else if (GameEvent.wheel.y < 0)
				{
					zoom -= sensitivity;
				}
			}

			if (GameEvent.type == SDL_KEYDOWN)
			{
				switch (GameEvent.key.keysym.sym)
				{
				case SDLK_UP:
					
					break;

				case SDLK_DOWN:
					
					break;

				case SDLK_LEFT:
					flipScarfy = SDL_FLIP_HORIZONTAL;
					scarfyObj.Translate(-speed, 0);
					ScarfyPlayAnimation = true;
					break;

				case SDLK_RIGHT:
					flipScarfy = SDL_FLIP_NONE;
					scarfyObj.Translate(speed, 0);
					ScarfyPlayAnimation = true;
					break;

				default:
					
					break;
				}
			}
			
			camera.UpdateCamera(MouseDelta, WindowSize, zoom);

			SDL_SetRenderDrawColor(renderer, clearColor.x, clearColor.y, clearColor.z, clearColor.w);
			SDL_RenderClear(renderer);

			SDL_Rect texture0DestRec = { (0 - camera.GetCameraFrustum().x) * zoom , (0 - camera.GetCameraFrustum().z) , (TextureSize.x / 2.0f) * zoom,(TextureSize.y / 2.0f) * zoom };
			SDL_Rect Rectangle0 = { (700 - camera.GetCameraFrustum().x ) * zoom , 0 - camera.GetCameraFrustum().z, 200 * zoom,200 * zoom };

			FusionDrawSDL::DrawPixelBuffer(renderer);
				
			SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
			SDL_RenderFillRect(renderer, &Rectangle0);

			SDL_RenderCopy(renderer, texture0, NULL, &texture0DestRec);

			//scarfyObj.Rotate(0.5f);
			scarfyObj.Draw(scarfyIndiciesLeft, ScarfySize.x / 12.0f, ScarfySize.y, 6, renderer, 5 , flipScarfy,ScarfyPlayAnimation);
			
			SDL_RenderPresent(renderer);

			frametime = SDL_GetTicks64() - framestart;

			if (framedelay > frametime)
			{
				SDL_Delay(framedelay - frametime);
			}
		}

		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window.second);
		IMG_Quit();
		SDL_Quit();
		LOG_INF("Window Terminated!");
		return 0;
	}
}

#endif 

int main(int argc, char* argv[])
{
	return KAGAN_PAVLO::EngineMain();
}