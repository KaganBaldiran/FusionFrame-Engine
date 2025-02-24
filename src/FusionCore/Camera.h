#pragma once
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "Object.hpp"
#include "Shader.h"
#include "../FusionUtility/FusionDLLExport.h"

#define FF_CAMERA_LAYOUT_FIRST_PERSON 0X100
#define FF_CAMERA_LAYOUT_INDUSTRY_STANDARD 0X101
#define MAX_LIGHT_PER_CLUSTER 100

//#define FREE_INDUSTRY_STANDARD_CAMERA  // define if you want to freely change the targetPosition using alt-lm click 
//#define FF_CAMERA_LAYOUT_INDUSTRY_STANDARD_FREE_ROTATION // define if you want to freely rotate the camera without needing to press alt 

//Forward Declarations 
struct GLFWwindow;

namespace FUSIONCORE
{
	FUSIONFRAME_EXPORT_FUNCTION void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	FUSIONFRAME_EXPORT_FUNCTION void WindowSizeCallback(GLFWwindow* window, int width, int height);

	class Model;

	class FUSIONFRAME_EXPORT Camera
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


	class FUSIONFRAME_EXPORT Camera2D : public Camera
	{
	public:

		Camera2D();
		void UpdateCameraMatrix(glm::vec3 target, float zoom, const glm::ivec2& windowSize);

	private:
		glm::vec3 target;
	};

	/*
 Represents a 3D camera used for viewing scenes in a 3D environment.

 The Camera3D class extends the base Camera class and provides functionality
 specific to 3D cameras. 

 Example usage:
 // Create a 3D camera
 Camera3D myCamera;

 // Update camera matrices based on window size and other parameters
 myCamera.UpdateCameraMatrix(45.0f, aspectRatio, 0.1f, 1000.0f, windowSize);

 // Handle user inputs to control camera movement
 myCamera.HandleInputs(window, windowSize);

 // Get the up vector of the camera
 glm::vec3 upVector = myCamera.GetUpVector();
 */
	class FUSIONFRAME_EXPORT Camera3D : public Camera
	{
	public:

		Camera3D();

		void UpdateCameraMatrix(const float& fovDegree, const float& aspect, const float& near, const float& far, const glm::ivec2& windowSize);
		void SetOrientation(glm::vec3 Orien);
		void SetPosition(glm::vec3 Pos);
#ifndef FREE_INDUSTRY_STANDARD_CAMERA
		void SetTargetPosition(glm::vec3 TargetPos);
		void SetTarget(Object* object, float Distance);
		void SetTarget(Object* object, float Distance, glm::vec3 Offset);
		void SetMinMaxZoom(bool clampZoom = false, float minZoom = -1.0f, float maxZoom = 1.0f);
		inline float GetCameraZoom() { return this->Zoom; };
		void SetZoomSensitivity(float Speed);
#endif
		void Matrix(const GLuint& shaderprogram, const char* uniform);
		void HandleInputs(GLFWwindow* window, const glm::vec2& WindowSize,const int& CameraLayout = FF_CAMERA_LAYOUT_FIRST_PERSON,float speed = 0.03f);
		const glm::vec3 GetUpVector() { return this->Up; };
		inline const float& GetCameraFOV() { return this->FOV; };
		inline const float& GetCameraAspectRatio() { return this->Aspect; };

		/*void UpdateCameraClusters(Shader& CameraClusterComputeShader, Shader& CameraLightCullingComputeShader);
		void SendClustersShader(uint BindingPoint);
		inline glm::vec3 GetClusterGridSize() { return this->ClusterGridSize; };*/
		inline glm::vec2 GetWindowSize() { return { w_width,w_height }; };
		inline bool IsCameraMovedf() { return this->IsCameraMoved; };

		glm::vec3 Orientation;
		glm::vec3 PlanarOrientation;
		glm::vec3 Position;
		float FarPlane;
		float NearPlane;

	private:

		/*SSBO ClusterSSBO;
		glm::vec3 ClusterGridSize;*/

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
		bool IsCameraMoved = true;

		glm::vec3 targetPosition;
		glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);

		int CameraLayout;

		glm::vec3 PreviousOrientation;
		glm::vec3 PreviousPosition;
	};

	/*
    Checks if a model is completely inside the camera frustum using AABB (Axis Aligned Bounding Box) test.

    This function determines whether the specified model is completely inside the camera frustum
    using an AABB test (Axis Aligned Bounding Box). It returns true if the model is entirely within
    the frustum, otherwise false.

    @param model The model to check.
    @param camera The camera representing the view frustum.
    @return True if the model is completely inside the camera frustum, otherwise false.
    */
	FUSIONFRAME_EXPORT_FUNCTION bool IsModelInsideCameraFrustumAABB(FUSIONCORE::Model& model, FUSIONCORE::Camera3D& camera);

	/*
	 Checks if a model is completely inside the camera frustum using sphere test.

	 This function determines whether the specified model is completely inside the camera frustum
	 using a sphere test. It returns true if the model is entirely within the frustum, otherwise false.

	 @param model The model to check.
	 @param camera The camera representing the view frustum.
	 @param RadiusPadding Additional padding to the radius of the model's bounding sphere.
	 @return True if the model is completely inside the camera frustum, otherwise false.
	 */
	FUSIONFRAME_EXPORT_FUNCTION bool IsModelInsideCameraFrustumSphere(FUSIONCORE::Model& model, FUSIONCORE::Camera3D& camera, float RadiusPadding = 0.0f);

	//Requires an active quadtree world partitioning 
	FUSIONFRAME_EXPORT_FUNCTION bool IsObjectQuadInsideCameraFrustum(FUSIONCORE::Object& model, FUSIONCORE::Camera3D& camera);
	FUSIONFRAME_EXPORT_FUNCTION std::pair<glm::vec3, glm::vec3> GetCameraFrustum(FUSIONCORE::Camera3D& camera);
}


