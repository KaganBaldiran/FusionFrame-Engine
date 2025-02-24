#pragma once
#include "FusionCore/Material.hpp"
#include "FusionUtility/Log.h"
#include "FusionCore/Shader.h"
#include "FusionCore/Buffer.h"
#include "FusionUtility/VectorMath.h"
#include "FusionCore/Texture.h"
#include "FusionUtility/Initialize.h"
#include "FusionCore/Camera.h"
#include "FusionCore/Mesh.h"
#include "FusionCore/Model.hpp"
#include "FusionCore/Light.hpp"
#include "FusionCore/Framebuffer.hpp"
#include "FusionPhysics/Physics.hpp"
#include "FusionCore/Color.hpp"
#include "FusionUtility/StopWatch.h"
#include "FusionCore/Cubemap.h"
#include "FusionUtility/Thread.h"
#include "FusionPhysics/Octtree.hpp"
#include "FusionCore/ShadowMaps.hpp"
#include "FusionCore/Animator.hpp"
#include "FusionCore/MeshOperations.h"
#include "FusionCore/Shapes.hpp"
#include "FusionEngine/Scene.hpp"
#include "FusionCore/Decal.hpp"
#include "FusionPhysics/ParticleSystem.hpp"
#include "FusionUtility/Definitions.hpp"
#include "FusionUtility/FusionGL.hpp"
#include "FusionCore/Window.hpp"
#include "FusionCore/EventManager.hpp"
#include "FusionUtility/Memory.hpp"
#include <stdio.h>

struct GLFWmonitor;

namespace FUSIONUTIL
{
	FUSIONFRAME_EXPORT_FUNCTION glm::vec2 GetMonitorSize();
	FUSIONFRAME_EXPORT_FUNCTION float GetDeltaFrame();

	FUSIONFRAME_EXPORT_FUNCTION std::shared_ptr<FILE> StartScreenCapturing(const char* FilePath, glm::vec2 ScreenSize);
	FUSIONFRAME_EXPORT_FUNCTION void UpdateScreenCapture(std::shared_ptr<FILE>& RecordingFile, glm::vec2 ScreenSize);
	FUSIONFRAME_EXPORT_FUNCTION void TerminateScreenCapture(std::shared_ptr<FILE>& RecordingFile);

	FUSIONFRAME_EXPORT_FUNCTION glm::dvec2 GetCursorPosition(GLFWwindow* window);
	FUSIONFRAME_EXPORT_FUNCTION void GetCursorPosition(GLFWwindow* window, double& x, double& y);

	FUSIONFRAME_EXPORT_FUNCTION glm::ivec2 GetWindowSize(GLFWwindow* window);
	FUSIONFRAME_EXPORT_FUNCTION void GetWindowSize(GLFWwindow* window, int& x, int& y);
	FUSIONFRAME_EXPORT_FUNCTION glm::ivec2 GetWindowPosition(GLFWwindow* window);
	FUSIONFRAME_EXPORT_FUNCTION void GetWindowPosition(GLFWwindow* window, int& x, int& y);
	FUSIONFRAME_EXPORT_FUNCTION void SetWindowMonitor(GLFWwindow* window, GLFWmonitor* monitor, int Xpos, int Ypos, int width, int height, int refreshRate);

	FUSIONFRAME_EXPORT_FUNCTION bool IsKeyPressedOnce(GLFWwindow* window, int Key, bool& Signal);
	FUSIONFRAME_EXPORT_FUNCTION int GetKey(GLFWwindow* window,int key);
	FUSIONFRAME_EXPORT_FUNCTION int GetMouseKey(GLFWwindow* window, int key);

	FUSIONFRAME_EXPORT_FUNCTION int WindowShouldClose(GLFWwindow* window);

	FUSIONFRAME_EXPORT struct VideoMode
	{
		unsigned int width;
		unsigned int height;
		int refreshRate;
		int redBits;
		int greenBits;
		int blueBits;
	};

	FUSIONFRAME_EXPORT_FUNCTION GLFWmonitor* GetPrimaryMonitor();
	FUSIONFRAME_EXPORT_FUNCTION VideoMode GetVideoMode(GLFWmonitor* monitor);

	FUSIONFRAME_EXPORT_FUNCTION double GetTime();

	FUSIONFRAME_EXPORT_FUNCTION unsigned int GetMaxTextureUnits();
	FUSIONFRAME_EXPORT_FUNCTION unsigned int GetMaxUniformBlockSize();

	FUSIONFRAME_EXPORT_FUNCTION void MakeGLFWcontextCurrent(GLFWwindow* window);
}

