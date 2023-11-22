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

		Camera2D camera;
		Camera3D camera3d;

		FUSIONOPENGL::TextureObj raccon;

		Texture2D texture("Resources/raccoon.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);
		Texture2D ShovelDiffuse("Resources/texture_diffuse.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
		Texture2D ShovelNormal("Resources/texture_normal.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);
		Texture2D ShovelSpecular("Resources/texture_specular.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, false);

		Vec2<int> WindowSize;
		Vec2<double> mousePos;

		glm::vec3 Target(0.0f);

		raccon.GetTransformation()->Scale({0.5f,0.5f,1.0f});
		raccon.GetTransformation()->Translate({ 0.0f,0.0f,4.0f });

		camera3d.SetPosition(glm::vec3(12.353, 13.326, 15.2838));
		camera3d.SetOrientation(glm::vec3(-0.593494, -0.648119, -0.477182));

		FUSIONOPENGL::Model model0("Resources\\shovel2.obj");
		FUSIONOPENGL::Model model1("Resources\\shovel2.obj");

		Material shovelMaterial;
		shovelMaterial.PushTextureMap(TEXTURE_DIFFUSE0, ShovelDiffuse);
		shovelMaterial.PushTextureMap(TEXTURE_NORMAL0, ShovelNormal);
		shovelMaterial.PushTextureMap(TEXTURE_SPECULAR0, ShovelSpecular);

		model1.GetTransformation().Translate({ 4.0f,0.0f,0.0f });
		model0.GetTransformation().Scale(glm::vec3(0.15f, 0.15f, 0.15f));

		model0.PushChild(&model1);

		MeshBasicShader.use();
		MeshBasicShader.setFloat("FogIntesityUniform", 5.0f);
		MeshBasicShader.setVec3("FogColor", glm::vec3(1.0f, 1.0f, 1.0f));
		UseShaderProgram(0);

		while (!glfwWindowShouldClose(window))
		{
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
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

			raccon.Draw(camera3d, BasicShader.GetID(), texture , ShaderPrep);

			model0.GetTransformation().Rotate({ 0.0f,1.0f,0.0f }, 0.05f);
			model0.UpdateChildren();

			std::function<void()> shaderPrepe = [&]() {
				model0.GetTransformation().SetModelMatrixUniformLocation(MeshBasicShader.GetID(), "model");
			};
			std::function<void()> shaderPrepe1 = [&]() {
				model1.GetTransformation().SetModelMatrixUniformLocation(MeshBasicShader.GetID(), "model");
			};
			//LOG("POSITION: " << Vec3<float>(camera3d.Position) << " ORIENTATION: " << Vec3<float>(camera3d.Orientation));
			model0.Draw(camera3d,MeshBasicShader,shovelMaterial, shaderPrepe);
			model1.Draw(camera3d, MeshBasicShader, shovelMaterial, shaderPrepe1);

			glfwPollEvents();
			glfwSwapBuffers(window);
		}

		DeleteShaderProgram(PixelShader.GetID());
		DeleteShaderProgram(BasicShader.GetID());
		DeleteShaderProgram(MeshBasicShader.GetID());

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
		SDL_Texture* texture0 = SDL_CUSTOM::LoadInTexture("Resources/raccoon.png", TextureSize,renderer);
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

		while (isGameRunning)
		{
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
			
			camera.UpdateCamera(MouseDelta, WindowSize, zoom);

			SDL_SetRenderDrawColor(renderer, clearColor.x, clearColor.y, clearColor.z, clearColor.w);
			SDL_RenderClear(renderer);

			SDL_Rect texture0DestRec = { (0 - camera.GetCameraFrustum().x) * zoom , (0 - camera.GetCameraFrustum().z) , (TextureSize.x / 2.0f) * zoom,(TextureSize.y / 2.0f) * zoom };
			SDL_Rect Rectangle0 = { (700 - camera.GetCameraFrustum().x ) * zoom , 0 - camera.GetCameraFrustum().z, 200 * zoom,200 * zoom };

	
			FusionDrawSDL::DrawPixelBuffer(renderer);
				
				
			SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
			SDL_RenderFillRect(renderer, &Rectangle0);

			SDL_RenderCopy(renderer, texture0, NULL, &texture0DestRec);
			
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