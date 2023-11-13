#include "Camera.h"

Camera::Camera()
{
	this->RatioMat = glm::mat4(1.0f);
	this->projMat = glm::mat4(1.0f);
	this->viewMat = glm::mat4(1.0f);
}

Camera::~Camera()
{
}

void Camera::SetViewMatrixUniformLocation(GLuint shader, const char* uniform)
{
	glUniformMatrix4fv(glGetUniformLocation(shader, uniform), 1, GL_FALSE, glm::value_ptr(this->viewMat));
}

void Camera::SetProjMatrixUniformLocation(GLuint shader, const char* uniform)
{
	glUniformMatrix4fv(glGetUniformLocation(shader, uniform), 1, GL_FALSE, glm::value_ptr(this->projMat));
}

void Camera::SetCameraMatrixUniformLocation(GLuint shader, const char* uniform)
{
	glUniformMatrix4fv(glGetUniformLocation(shader, uniform), 1, GL_FALSE, glm::value_ptr(this->CameraMat));
}

Vec2<float> Camera::GetScreenRatio(Vec2<int> windowSize)
{
	float ratio_minmax = NULL;

	float aspectRatios_wh = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
	float aspectRatios_hw = static_cast<float>(windowSize.y) / static_cast<float>(windowSize.x);

	float ratio_minmax_x = NULL, ratio_minmax_y = NULL;

	if (windowSize.y > windowSize.x)
	{
		ratio_minmax_x = aspectRatios_hw;
		ratio_minmax_y = 1.0f;

	}
	if (windowSize.y < windowSize.x)
	{
		ratio_minmax_x = aspectRatios_hw;
		ratio_minmax_y = 1.0f;

	}
	if (windowSize.y == windowSize.x)
	{
		ratio_minmax_x = 1.0f;
		ratio_minmax_y = 1.0f;
	}

	return Vec2<float>{ratio_minmax_x, ratio_minmax_y};
}

void Camera::SetRatioMatrixUniformLocation(GLuint shader, const char* uniform)
{
	glUniformMatrix4fv(glGetUniformLocation(shader, uniform), 1, GL_FALSE, glm::value_ptr(this->RatioMat));
}

Camera2D::Camera2D()
{
	this->RatioMat = glm::mat4(1.0f);
	this->projMat = glm::mat4(1.0f);
	this->viewMat = glm::mat4(1.0f);
}

void Camera2D::UpdateCameraMatrix(glm::vec3 target, float zoom, Vec2<int> windowSize)
{
	RatioMat = glm::mat4(1.0f);
	projMat = glm::ortho((-1.0f + target.x) / zoom, (1.0f + target.x) / zoom, (-1.0f + target.y) / zoom, (1.0f + target.y) / zoom, -5.0f, 5.0f);
	RatioMat = glm::scale(RatioMat, glm::vec3(GetScreenRatio(windowSize).x, GetScreenRatio(windowSize).y, 1.0f));

	this->CameraMat = RatioMat * projMat;
}

Camera3D::Camera3D()
{
	this->RatioMat = glm::mat4(1.0f);
	this->projMat = glm::mat4(1.0f);
	this->viewMat = glm::mat4(1.0f);
}

void Camera3D::UpdateCameraMatrix(float fovDegree, float aspect, float near, float far,glm::vec3 up, Vec2<int> windowSize)
{
	this->projMat = glm::perspective(fovDegree, aspect, near, far);
	this->RatioMat = RatioMat = glm::scale(RatioMat, glm::vec3(GetScreenRatio(windowSize).x, GetScreenRatio(windowSize).y, 1.0f));
	this->viewMat = glm::lookAt(Position, Position + Orientation, up);

	this->CameraMat = RatioMat * projMat * viewMat;
}


