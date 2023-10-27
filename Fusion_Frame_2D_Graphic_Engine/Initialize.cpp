#include "Initialize.h"

GLFWwindow* INIT::InitializeWindow(int width, int height, const char* WindowName)
{
	if (!glfwInit())
	{
		LOG_ERR("Initializing GLFW!");
		glfwTerminate();
		return nullptr;
	}
	LOG_INF("GLFW initialized!");

	glfwWindowHint(GLFW_SAMPLES, 4);

	glEnable(GL_MULTISAMPLE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, WindowName, NULL, NULL);

	if (window == NULL)
	{
		LOG_ERR("Initializing Window!");
		glfwTerminate();
		return nullptr;
	}
	LOG_INF("Window initialized!");


	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
	{
		LOG_ERR("Initializing GLEW!");
		glfwTerminate();
		return nullptr;
	}
	LOG_INF("Glew initialized!");
	LOG_INF("OpenGL version : 3.3 core");

    return window;
}
