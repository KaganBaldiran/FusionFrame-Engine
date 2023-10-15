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

int main()
{
	const int width = 1000;
	const int height = 1000;

	GLFWwindow* window = INIT::InitializeWindow(width, height, "FusionFrame Engine");

	std::unique_ptr<Shader> BasicShader = std::make_unique<Shader>("Shaders/Basic.vs", "Shaders/Basic.fs");

	std::unique_ptr<Buffer> Triangle = std::make_unique<Buffer>();

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

	Texture2D texture("Resources/raccoon.png", GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,true);
	Vec2<int> WindowSize;

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glfwGetWindowSize(window, &WindowSize.x, &WindowSize.y);
		glViewport(0, 0, WindowSize.x, WindowSize.y);

		glUseProgram(BasicShader->GetID());
		Triangle->BindVAO();

		glm::mat4 projection(1.0f);
		projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -0.1f, 100.0f);
		glUniformMatrix4fv(glGetUniformLocation(BasicShader->GetID(), "proj"), 1, GL_FALSE, glm::value_ptr(projection));

		texture.Bind(0, BasicShader->GetID(), "texture0");

		glDrawArrays(GL_TRIANGLES, 0, 3);

		Triangle->UnbindVAO();
		texture.Unbind();
		glUseProgram(0);

		glfwPollEvents();
		glfwSwapBuffers(window);
	}


	DeleteShaderProgram(BasicShader->GetID());
	glfwTerminate();
	LOG_INF("Window terminated!");
	return 0;
}