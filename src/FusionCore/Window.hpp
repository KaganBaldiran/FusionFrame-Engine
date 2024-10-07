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
		Window();
		int InitializeWindow(int width, int height, unsigned int MajorGLversion, unsigned int MinorGLversion, bool EnableGLdebug, const char* WindowName);
		void TerminateWindow();
		bool ShouldClose();
		void SwapBuffers();
		void PollEvents();
		void UpdateWindow();
		void MakeWindowContextCurrent();
		inline const glm::ivec2& GetInitialWindowSize() { return this->InitialWindowSize; };
		inline const glm::ivec2& GetWindowSize() { return this->WindowSize; };
		inline const std::string& GetWindowName() { return this->WindowName; };
		inline GLFWwindow* GetWindow() { return this->window; };
		inline bool IsWindowResizedf() { return IsWindowResized; };
	private:
		GLFWwindow* window;
		glm::ivec2 InitialWindowSize;
		glm::ivec2 WindowSize;
		std::string WindowName;

		bool IsWindowResized;

		static void window_size_callback(GLFWwindow* window, int width, int height);
		void OnWindowResize(const int& width,const int& height);
	};
}

