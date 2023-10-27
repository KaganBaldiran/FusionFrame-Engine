#pragma once
#include <glew.h>
#include <glfw3.h>
#include "Log.h"

namespace INIT
{
	GLFWwindow* InitializeWindow(int width , int height , const char* WindowName);
}