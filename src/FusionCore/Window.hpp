#pragma once
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include <memory>
#include "../FusionUtility/FusionDLLExport.h"

struct GLFWwindow;

namespace FUSIONCORE
{
	class FUSIONFRAME_EXPORT Window
	{
	public:
		Window() :window(nullptr), WindowSize({ 0,0 }), WindowName("") {};
		int InitializeWindow(int width, int height, unsigned int MajorGLversion, unsigned int MinorGLversion, bool EnableGLdebug, const char* WindowName);
		void TerminateWindow();
		bool ShouldClose();
		void SwapBuffers();
		void PollEvents();
		void MakeWindowContextCurrent();
		inline const glm::vec2& GetWindowSize() { return this->WindowSize; };
		inline const std::string& GetWindowName() { return this->WindowName; };
		inline GLFWwindow* GetWindow() { return this->window; };
	private:
		GLFWwindow* window;
		glm::vec2 WindowSize;
		std::string WindowName;
	};
}

