#include "Camera.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"

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
	this->CameraMat = glm::mat4(1.0f);

	targetPosition = glm::vec3(0.0f);
}

void Camera3D::UpdateCameraMatrix(float fovDegree, float aspect, float near, float far, Vec2<int> windowSize)
{
	this->projMat = glm::perspective(glm::radians(fovDegree), aspect, near, far);
	this->RatioMat = glm::scale(glm::mat4(1.0f), glm::vec3(GetScreenRatio(windowSize).x, GetScreenRatio(windowSize).y, 1.0f));
	this->viewMat = glm::lookAt(Position, Position + Orientation, this->Up);

	this->CameraMat = this->RatioMat * projMat * viewMat;
}

void Camera3D::Matrix(GLuint shaderprogram, const char* uniform)
{

	glUniformMatrix4fv(glGetUniformLocation(shaderprogram, uniform), 1, GL_FALSE, glm::value_ptr(CameraMat));
	glUniformMatrix4fv(glGetUniformLocation(shaderprogram, "proj"), 1, GL_FALSE, glm::value_ptr(projMat));
	glUniformMatrix4fv(glGetUniformLocation(shaderprogram, "view"), 1, GL_FALSE, glm::value_ptr(viewMat));

}


void Camera3D::SetOrientation(glm::vec3 Orien)
{
	this->Orientation = Orien;
}

void Camera3D::SetPosition(glm::vec3 Pos)
{
	this->Position = Pos;
}


void Camera3D::HandleInputs(GLFWwindow* window, Vec2<int> WindowSize)
{

	
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{

			Position += speed * Orientation;

		}

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{

			Position += speed * -glm::normalize(glm::cross(Orientation, Up));

		}

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{

			Position += speed * -Orientation;

		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{

			Position += speed * glm::normalize(glm::cross(Orientation, Up));;

		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{

			Position += speed * Up;

		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		{

			Position += speed * -Up;

		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{

			speed = 0.07f;

		}
		else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE)
		{

			speed = 0.03f;

		}


		if ((glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS))
		{
			int height = NULL, width = NULL;
			glfwGetWindowSize(window, &width, &height);

			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

			if (firstclick)
			{
				glfwSetCursorPos(window, (w_width / 2), (w_height / 2));
				firstclick = false;
			}

			Vec2<double> mousepos;

			glfwGetCursorPos(window, &mousepos.x, &mousepos.y);

			Vec2<float> rot;

			rot.x = sensitivity * (float)(mousepos.y - (w_height / 2)) / w_height;
			rot.y = sensitivity * (float)(mousepos.x - (w_width / 2)) / w_width;

			glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rot.x), glm::normalize(glm::cross(Orientation, Up)));

			if (glm::abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
			{

				Orientation = newOrientation;

			}

			Orientation = glm::rotate(Orientation, glm::radians(-rot.y), Up);

			//int height = NULL, width = NULL;
			//glfwGetWindowSize(window, &width, &height);

			glfwSetCursorPos(window, (w_width / 2), (w_height / 2));
		}

		else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_RELEASE)
		{

			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

			firstclick = true;

		}
	
	
}


