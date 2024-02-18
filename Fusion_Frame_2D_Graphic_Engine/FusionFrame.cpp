#include "FusionFrame.h"

glm::vec2 FUSIONUTIL::GetMonitorSize()
{
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    return glm::vec2(mode->width, mode->height);
}
