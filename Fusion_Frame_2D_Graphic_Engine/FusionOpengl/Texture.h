#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/VectorMath.h"
#include "../FusionUtility/Log.h"
#include <string>

namespace FUSIONOPENGL
{
	class Texture2D
	{
	public:
		Texture2D() = default;

		Texture2D(const char* filePath, GLenum target, GLenum type, GLuint Mag_filter, GLuint Min_filter, GLuint Wrap_S_filter, GLuint Wrap_T_filter, bool Flip);
		~Texture2D();

		GLuint GetTexture();
		int GetWidth();
		int GetHeight();
		void Bind(GLuint slot, GLuint shader, const char* uniform);
		void Unbind();
		std::string GetFilePath();
		int GetChannelCount();

		std::string PbrMapType;

	private:
		GLuint id;
		int width, height, channels;
		std::string PathData;

	};
}
