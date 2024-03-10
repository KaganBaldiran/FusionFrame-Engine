#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "Object.hpp"
#define FF_CAMERA_LAYOUT_FIRST_PERSON 0X100
#define FF_CAMERA_LAYOUT_INDUSTRY_STANDARD 0X101

//#define FREE_INDUSTRY_STANDARD_CAMERA  // define if you want to freely change the targetPosition using alt-lm click 
#define FF_CAMERA_LAYOUT_INDUSTRY_STANDARD_FREE_ROTATION // define if you want to freely rotate the camera without needing to press alt 

namespace FUSIONCORE
{
	void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	void WindowSizeCallback(GLFWwindow* window, int width, int height);

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
#ifndef FREE_INDUSTRY_STANDARD_CAMERA
		void SetTargetPosition(glm::vec3 TargetPos);
		void SetTarget(Object* object, float Distance);
		void SetTarget(Object* object, float Distance , glm::vec3 Offset);
		void SetMinMaxZoom(bool clampZoom = false , float minZoom = -1.0f, float maxZoom = -1.0f);
		inline float GetCameraZoom() { return this->Zoom; };
		void SetZoomSensitivity(float Speed);
#endif
		void Matrix(GLuint shaderprogram, const char* uniform);
		void HandleInputs(GLFWwindow* window, Vec2<int> WindowSize, int CameraLayout = FF_CAMERA_LAYOUT_FIRST_PERSON, float speed = 0.03f);
		const glm::vec3 GetUpVector() {return this->Up; };
		inline const float& GetCameraFOV() { return this->FOV; };

		glm::vec3 Orientation;
		glm::vec3 Position;
		float FarPlane;
		float NearPlane;

	private:

		bool firstclick = true;
		int w_width = 1000;
		int w_height = 1000;
		float sensitivity = 100.0f;

		float Zoom = 1.0f;
		float MinZoom = 0.8f;
		float MaxZoom = 1.2f;
		float ZoomSpeed = 1.0f;
		bool ClampZoom = false;

		float FOV = 45.0f;

		glm::vec3 targetPosition;
		glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);

		int CameraLayout;

	};
}


