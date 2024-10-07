#include "Window.hpp"
#include <glew.h>
#include <glfw3.h>
#include "../FusionCore/Camera.h"

static std::pair<bool, bool> BaseSystemsInitialised;

void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

FUSIONCORE::Window::Window()
{
	window = nullptr;
	InitialWindowSize = { 0,0 };
	WindowSize = { 0,0 };
	WindowName = "";
}

int FUSIONCORE::Window::InitializeWindow(int width, int height, unsigned int MajorGLversion, unsigned int MinorGLversion,
	                          bool EnableGLdebug, const char* WindowName){
	if (!BaseSystemsInitialised.first && !glfwInit())
	{
		LOG_ERR("Initializing GLFW!");
		glfwTerminate();
		return FF_ERROR_CODE;
	}

	if (!BaseSystemsInitialised.first)
	{
		LOG_INF("GLFW initialized!");
		BaseSystemsInitialised.first = true;
	}
	
	glfwWindowHint(GLFW_SAMPLES, 4);
	glEnable(GL_MULTISAMPLE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MajorGLversion);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MinorGLversion);
	if (EnableGLdebug)
	{
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	}

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, WindowName, NULL, NULL);

	if (window == NULL)
	{
		LOG_ERR("Initializing Window!");
		glfwTerminate();
		return FF_ERROR_CODE;
	}
	LOG_INF("Window initialized!");

	glfwMakeContextCurrent(window);

	if (!BaseSystemsInitialised.second && glewInit() != GLEW_OK)
	{
		LOG_ERR("Initializing GLEW!");
		glfwTerminate();
		return FF_ERROR_CODE;
	}
	if (!BaseSystemsInitialised.second)
	{
		LOG_INF("Glew initialized!");
		BaseSystemsInitialised.second = true;
	}

	int major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	LOG_INF("OpenGL version : " << major << "." << minor << " core");

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glfwSetScrollCallback(window, FUSIONCORE::scrollCallback);

	if (EnableGLdebug)
	{
		glDebugMessageCallback(debugCallback, nullptr);
	}

	this->InitialWindowSize = this->WindowSize = {width,height};
	this->WindowName = WindowName;

	glfwSetWindowUserPointer(window, this);
	glfwSetWindowSizeCallback(window, window_size_callback);

	return FF_SUCCESS_CODE;
}

void FUSIONCORE::Window::TerminateWindow()
{
	glfwDestroyWindow(this->window);
	LOG_INF("Window terminated!");
}

bool FUSIONCORE::Window::ShouldClose()
{
	return glfwWindowShouldClose(window);
}

void FUSIONCORE::Window::SwapBuffers()
{
	glfwSwapBuffers(window);
}

void FUSIONCORE::Window::PollEvents()
{
	glfwPollEvents();
}

void FUSIONCORE::Window::UpdateWindow()
{
	IsWindowResized = false;
	SwapBuffers();
	PollEvents();
}

void FUSIONCORE::Window::MakeWindowContextCurrent()
{
	glfwMakeContextCurrent(window);
	glfwSetWindowUserPointer(window, this);
}

void FUSIONCORE::Window::window_size_callback(GLFWwindow* window, int width, int height)
{
	Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
	if (instance) {
		instance->OnWindowResize(width, height);
	}
}

void FUSIONCORE::Window::OnWindowResize(const int& width, const int& height)
{
	WindowSize.x = width;
	WindowSize.y = height;
	IsWindowResized = true;
}

void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {

	const char* sourceStr;
	switch (source) {
	case GL_DEBUG_SOURCE_API: sourceStr = "API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceStr = "Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY: sourceStr = "Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION: sourceStr = "Application"; break;
	case GL_DEBUG_SOURCE_OTHER: sourceStr = "Other"; break;
	default: sourceStr = "Unknown"; break;
	}

	const char* typeStr;
	switch (type) {
	case GL_DEBUG_TYPE_ERROR: typeStr = "Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behavior"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typeStr = "Undefined Behavior"; break;
	case GL_DEBUG_TYPE_PORTABILITY: typeStr = "Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE: typeStr = "Performance"; break;
	case GL_DEBUG_TYPE_MARKER: typeStr = "Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP: typeStr = "Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP: typeStr = "Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER: typeStr = "Other"; break;
	default: typeStr = "Unknown"; break;
	}

	const char* severityStr;
	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH: severityStr = "High"; break;
	case GL_DEBUG_SEVERITY_MEDIUM: severityStr = "Medium"; break;
	case GL_DEBUG_SEVERITY_LOW: severityStr = "Low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = "Notification"; break;
	default: severityStr = "Unknown"; break;
	}


	fprintf(stderr, "OpenGL Debug Message:\n");
	fprintf(stderr, "    Source: %s\n", sourceStr);
	fprintf(stderr, "    Type: %s\n", typeStr);
	fprintf(stderr, "    ID: %u\n", id);
	fprintf(stderr, "    Severity: %s\n", severityStr);
	fprintf(stderr, "    Message: %s\n", message);
}
