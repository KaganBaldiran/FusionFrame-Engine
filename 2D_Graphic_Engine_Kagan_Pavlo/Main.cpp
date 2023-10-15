#include <glew.h>
#include <glfw3.h>
#include <iostream>
#include "Log.h"
#include "Shader.h"
#include "Thread.h"

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


	while (!glfwWindowShouldClose(window))
	{
		



		glfwPollEvents();
		glfwSwapBuffers(window);
	}


	glfwTerminate();
	LOG_INF("Window terminated!");
	return 0;
}