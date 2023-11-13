#pragma once
#include <glew.h>
#include <glfw3.h>
#include "Log.h"
#include "VectorMath.h"

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

protected:
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

	void UpdateCameraMatrix(float fovDegree, float aspect, float near, float far, glm::vec3 up, Vec2<int> windowSize);
	
private:
	glm::vec3 Orientation;
	glm::vec3 Position;
};



