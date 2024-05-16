#include "FusionFrame.h"
#include <memory>
#include <glew.h>
#include <glfw3.h>

glm::vec2 FUSIONUTIL::GetMonitorSize()
{
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	return glm::vec2(mode->width, mode->height);
}

float FUSIONUTIL::GetDeltaFrame()
{
	static float deltaTime = 0.0f;
	static float lastFrame = 0.0f;

	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	return deltaTime;
}

std::shared_ptr<FILE> FUSIONUTIL::StartScreenCapturing(const char* FilePath, glm::vec2 ScreenSize)
{
	std::string Command = "ffmpeg -y -f rawvideo -pixel_format rgb24 -video_size " + std::to_string(ScreenSize.x) + "x" + std::to_string(ScreenSize.y) + " -framerate 25 -i - -vf vflip -c:v libx264 -preset ultrafast -crf 0 " + FilePath;        FILE* avconv = _popen(Command.c_str(), "w");
	return std::shared_ptr<FILE>(avconv, [](FILE* file) { if (file) _pclose(file); });
}

void FUSIONUTIL::UpdateScreenCapture(std::shared_ptr<FILE>& RecordingFile, glm::vec2 ScreenSize)
{
	std::vector<unsigned char> pixels(ScreenSize.x * ScreenSize.y * 3);
	glReadPixels(0, 0, static_cast<GLsizei>(ScreenSize.x), static_cast<GLsizei>(ScreenSize.y), GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
	if (RecordingFile) {
		fwrite(pixels.data(), pixels.size(), 1, RecordingFile.get());
	}
}

void FUSIONUTIL::TerminateScreenCapture(std::shared_ptr<FILE>& RecordingFile)
{
	if (RecordingFile) {
		RecordingFile.reset();
	}
}

glm::dvec2 FUSIONUTIL::GetCursorPosition(GLFWwindow* window)
{
	glm::dvec2 mousePos;
	glfwGetCursorPos(window, &mousePos.x, &mousePos.y);
	return mousePos;
}

void FUSIONUTIL::GetCursorPosition(GLFWwindow* window, double& x, double& y)
{
	glfwGetCursorPos(window, &x, &y);
}

glm::ivec2 FUSIONUTIL::GetWindowSize(GLFWwindow* window)
{
	glm::ivec2 WindowSize;
	glfwGetWindowSize(window, &WindowSize.x, &WindowSize.y);
	return WindowSize;
}

void FUSIONUTIL::GetWindowSize(GLFWwindow* window, int& x, int& y)
{
	glfwGetWindowSize(window, &x, &y);
}

glm::ivec2 FUSIONUTIL::GetWindowPosition(GLFWwindow* window)
{
	glm::ivec2 WindowPos;
	glfwGetWindowPos(window, &WindowPos.x, &WindowPos.y);
	return WindowPos;
}

void FUSIONUTIL::GetWindowPosition(GLFWwindow* window, int& x, int& y)
{
	glfwGetWindowPos(window, &x, &y);
}

bool FUSIONUTIL::IsKeyPressedOnce(GLFWwindow* window, int Key, bool& Signal)
{
	if (!Signal && glfwGetKey(window, Key) == GLFW_RELEASE)
	{
		Signal = true;
	}
	if (glfwGetKey(window, Key) == GLFW_PRESS && Signal)
	{
		Signal = false;
		return true;
	}
	return false;
}

int FUSIONUTIL::GetKey(GLFWwindow* window, int key)
{
	return glfwGetKey(window, key);
}

int FUSIONUTIL::GetMouseKey(GLFWwindow* window, int key)
{
	return glfwGetMouseButton(window, key);
}

int FUSIONUTIL::WindowShouldClose(GLFWwindow* window)
{
	return glfwWindowShouldClose(window);
}

GLFWmonitor* FUSIONUTIL::GetPrimaryMonitor()
{
	return glfwGetPrimaryMonitor();
}

FUSIONUTIL::VideoMode FUSIONUTIL::GetVideoMode(GLFWmonitor* monitor)
{
	auto mode = glfwGetVideoMode(monitor);
	VideoMode NewMode;
	NewMode.width = mode->width;
	NewMode.height = mode->height;
	NewMode.refreshRate = mode->refreshRate;
	NewMode.redBits= mode->redBits;
	NewMode.greenBits= mode->greenBits;
	NewMode.blueBits= mode->blueBits;

	return NewMode;
}

double FUSIONUTIL::GetTime()
{
	return glfwGetTime();
}

unsigned int FUSIONUTIL::GetMaxTextureUnits()
{
	int maxTextureUnits;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
	return maxTextureUnits;
}

unsigned int FUSIONUTIL::GetMaxUniformBlockSize()
{
	GLint maxUniformBlockSize;
	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);
	return maxUniformBlockSize;
}


