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

	class Model;

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
		glm::mat4 ProjectionViewMat;
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
		inline const float& GetCameraAspectRatio() { return this->Aspect; };

		glm::vec3 Orientation;
		glm::vec3 PlanarOrientation;
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
		float Aspect;

		bool ClampZoom = false;

		float FOV = 45.0f;

		glm::vec3 targetPosition;
		glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);

		int CameraLayout;
	};

	bool IsModelInsideCameraFrustum(FUSIONCORE::Model& model, FUSIONCORE::Camera3D& camera);

	static std::pair<glm::vec3, glm::vec3> GetCameraFrustum(FUSIONCORE::Camera3D& camera)
	{
		glm::mat4 inverseMatrix = glm::inverse(camera.projMat * camera.viewMat);
		std::vector<glm::vec4> FrustumCorners;
		for (size_t x = 0; x < 2; x++)
		{
			for (size_t y = 0; y < 2; y++)
			{
				for (size_t z = 0; z < 2; z++)
				{
					glm::vec4 Point = inverseMatrix * glm::vec4(2.0f * x - 1.0f,
						2.0f * y - 1.0f,
						2.0f * z - 1.0f,
						1.0f);
					FrustumCorners.push_back(Point / Point.w);
				}
			}
		}

		glm::vec3 center = glm::vec3(0.0f);
		for (size_t i = 0; i < FrustumCorners.size(); i++)
		{
			center += glm::vec3(FrustumCorners[i]);
		}
		center /= FrustumCorners.size();

		float minX = std::numeric_limits<float>::max();
		float maxX = std::numeric_limits<float>::lowest();
		float minY = std::numeric_limits<float>::max();
		float maxY = std::numeric_limits<float>::lowest();
		float minZ = std::numeric_limits<float>::max();
		float maxZ = std::numeric_limits<float>::lowest();

		for (size_t i = 0; i < FrustumCorners.size(); i++)
		{
			const auto Corner = FrustumCorners[i];
			minX = std::min(minX, Corner.x);
			maxX = std::max(maxX, Corner.x);
			minY = std::min(minY, Corner.y);
			maxY = std::max(maxY, Corner.y);
			minZ = std::min(minZ, Corner.z);
			maxZ = std::max(maxZ, Corner.z);
		}

		/*const float Zmultiplier = std::abs(camera.FarPlane - camera.NearPlane) * 0.05f;
		if (minZ < 0.0f)
		{
			minZ *= Zmultiplier;
		}
		else
		{
			minZ /= Zmultiplier;
		}

		if (maxZ < 0.0f)
		{
			maxZ /= Zmultiplier;
		}
		else
		{
			maxZ *= Zmultiplier;
		}*/

		return std::make_pair<glm::vec3, glm::vec3>({minX,minY,minZ},{maxX,maxY,maxZ});
	}

	
}


