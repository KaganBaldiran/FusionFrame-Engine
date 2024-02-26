#include "FusionFrame.h"

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
