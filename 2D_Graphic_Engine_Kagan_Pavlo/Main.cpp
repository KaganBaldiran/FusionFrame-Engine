#include <glew.h>
#include <glfw3.h>
#include <iostream>
#include "Log.h"
#include "Shader.h"
#include "Thread.h"
#include <memory>
#include "Buffer.h"
#include "VectorMath.h"

int main()
{
	const int width = 1000;
	const int height = 1000;

	if (!glfwInit())
	{
		LOG_ERR("Initializing GLFW!");
		glfwTerminate();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);

	glEnable(GL_MULTISAMPLE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, "FusionFrame Engine", NULL, NULL);


	if (window == NULL)
	{
		LOG_ERR("Initializing Window!");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
	{
		LOG_ERR("Initializing GLEW!");
		glfwTerminate();
		return -1;
	}

	std::unique_ptr<Shader> BasicShader = std::make_unique<Shader>("Shaders/Basic.vs", "Shaders/Basic.fs");

	std::unique_ptr<Buffer> Triangle = std::make_unique<Buffer>();

	Triangle->Bind();

	float vertices[] = {
	   -0.5f, -0.5f, 0.0f,  1.0f,0.0f,0.0f,   0.0f, 0.0f,
		0.5f, -0.5f, 0.0f,  0.0f,1.0f,0.0f,   1.0f, 0.0f,
		0.0f,  0.5f, 0.0f,  0.0f,0.0f,1.0f,   0.5f, -1.0f
	};

	Triangle->BufferDataFill(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	Triangle->AttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	Triangle->AttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	Triangle->AttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

	Triangle->Unbind();
	
	while (!glfwWindowShouldClose(window))
	{
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(BasicShader->GetID());
		Triangle->BindVAO();

		glm::mat4 projection(1.0f);
		projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -0.1f, 100.0f);
		glUniformMatrix4fv(glGetUniformLocation(BasicShader->GetID(), "proj"), 1, GL_FALSE, glm::value_ptr(projection));

		glDrawArrays(GL_TRIANGLES, 0, 3);

		Triangle->UnbindVAO();
		glUseProgram(0);

		glfwPollEvents();
		glfwSwapBuffers(window);
	}


	DeleteShaderProgram(BasicShader->GetID());
	glfwTerminate();
	LOG_INF("Window terminated!");
	return 0;
}