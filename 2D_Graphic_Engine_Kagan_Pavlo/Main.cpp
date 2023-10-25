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

#define SDL 
#ifdef OPENGL

namespace KAGAN_PAVLO
{
	int EngineMain()
	{
		const int width = 1000;
		const int height = 1000;

		GLFWwindow* window = INIT::InitializeWindow(width, height, "FusionFrame Engine");

		std::unique_ptr<Shader> BasicShader = std::make_unique<Shader>("Shaders/Basic.vs", "Shaders/Basic.fs");

		std::unique_ptr<Buffer> Triangle = std::make_unique<Buffer>();
		std::unique_ptr<Buffer> Rectangle = std::make_unique<Buffer>();

		Camera2D camera;

		Triangle->Bind();

		float vertices[] = {
		  -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		   0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		   0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f, 0.5f, 1.0f
		};


		Triangle->BufferDataFill(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		Triangle->AttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		Triangle->AttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		Triangle->AttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

		Triangle->Unbind();

		TextureObj raccon;

		Texture2D texture("Resources/raccoon.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);
		Vec2<int> WindowSize;
		Vec2<double> mousePos;

		glm::vec3 Target(0.0f);

		raccon.Scale({ 0.5f,0.5f,1.0f });

		while (!glfwWindowShouldClose(window))
		{
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			glfwGetWindowSize(window, &WindowSize.x, &WindowSize.y);
			glViewport(0, 0, WindowSize.x, WindowSize.y);

			glfwGetCursorPos(window, &mousePos.x, &mousePos.y);
			Target = { mousePos.x / WindowSize.x , -mousePos.y / WindowSize.y, 0.0f };

			camera.UpdateCameraMatrix(Target, 0.5f, WindowSize);


			raccon.Draw(camera, BasicShader->GetID(), texture);



			glfwPollEvents();
			glfwSwapBuffers(window);
		}


		DeleteShaderProgram(BasicShader->GetID());
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