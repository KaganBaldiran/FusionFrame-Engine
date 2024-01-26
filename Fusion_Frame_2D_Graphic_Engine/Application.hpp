#pragma once
#include <glew.h>
#include <glfw3.h>

#define TARGET_FPS 144
#define ENGINE_DEBUG

#ifndef ENGINE_DEBUG
#define ENGINE_RELEASE
#endif 

class Application
{
public:

	int Run();
	bool IsKeyPressedOnce(GLFWwindow* window , int Key, bool& Signal);

private:

};
