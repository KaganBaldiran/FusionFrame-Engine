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

int main()
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

	Texture2D texture("Resources/raccoon.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,true);
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