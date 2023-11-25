#pragma once
#include <glew.h>
#include <glfw3.h>
#include "Log.h"
#include "VectorMath.h"
#define CAMERA_LAYOUT_FIRST_PERSON 0X100
#define CAMERA_LAYOUT_INDUSTRY_STANDARD 0X101

namespace FUSIONOPENGL
{
	class Camera
	{
	public:
		Camera();
		~Camera();

		void SetViewMatrixUniformLocation(GLuint shader, const char* uniform);
		void SetProjMatrixUniformLocation(GLuint shader, const char* uniform);
		void SetCameraMatrixUniformLocation(GLuint shader, const char* uniform);
		Vec2<float> GetScreenRatio(Vec2<int> windowSize);
		void SetRatioMatrixUniformLocation(GLuint shader, const char* uniform);

		glm::mat4 viewMat;
		glm::mat4 projMat;
		glm::mat4 CameraMat;
		glm::mat4 RatioMat;
	};


	class Camera2D : public Camera
	{
	public:

		Camera2D();
		void UpdateCameraMatrix(glm::vec3 target, float zoom, Vec2<int> windowSize);

	private:
		glm::vec3 target;
	};

	class Camera3D : public Camera
	{
	public:

		Camera3D();

		void UpdateCameraMatrix(float fovDegree, float aspect, float near, float far, Vec2<int> windowSize);
		void SetOrientation(glm::vec3 Orien);
		void SetPosition(glm::vec3 Pos);
		void Matrix(GLuint shaderprogram, const char* uniform);
		void HandleInputs(GLFWwindow* window, Vec2<int> WindowSize);

		glm::vec3 Orientation;
		glm::vec3 Position;
		float FarPlane;
		float NearPlane;

	private:

		bool firstclick = true;
		int w_width = 1000;
		int w_height = 1000;
		float speed = 0.03f;
		float sensitivity = 100.0f;

		glm::vec3 targetPosition;
		glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);

	};
}


