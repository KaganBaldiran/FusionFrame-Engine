#pragma once
#include <glew.h>
#include <glfw3.h>
#include "Log.h"
#include "VectorMath.h"

class Camera2D
{
public:
	Camera2D();
	~Camera2D();

	void UpdateCameraMatrix(glm::vec3 target, float zoom, Vec2<int> windowSize);
	void SetViewMatrixUniformLocation(GLuint shader, const char* uniform);
	void SetProjMatrixUniformLocation(GLuint shader, const char* uniform);
	void SetCameraMatrixUniformLocation(GLuint shader, const char* uniform);
	Vec2<float> GetScreenRatio(Vec2<int> windowSize);
	void SetRatioMatrixUniformLocation(GLuint shader, const char* uniform);
    

private:
	glm::mat4 viewMat;
	glm::mat4 projMat;
	glm::mat4 CameraMat;
	glm::mat4 RatioMat;

	glm::vec3 target;
};

