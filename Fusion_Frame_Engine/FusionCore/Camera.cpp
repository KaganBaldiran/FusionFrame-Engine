#include "Camera.h"
#include "../FusionUtility/glm/glm.hpp"
#include "../FusionUtility/glm/gtc/matrix_transform.hpp"
#include "../FusionUtility/glm/gtc/type_ptr.hpp"
#include "../FusionUtility/glm/gtx/rotate_vector.hpp"
#include "../FusionUtility/glm/gtx/vector_angle.hpp"
#include "Model.hpp"

Vec2<double> ScrollAmount;
Vec2<double> MousePosCamera;

FUSIONCORE::Camera::Camera()
{
	this->RatioMat = glm::mat4(1.0f);
	this->projMat = glm::mat4(1.0f);
	this->viewMat = glm::mat4(1.0f);
}

FUSIONCORE::Camera::~Camera()
{
}

void FUSIONCORE::Camera::SetViewMatrixUniformLocation(GLuint shader, const char* uniform)
{
	glUniformMatrix4fv(glGetUniformLocation(shader, uniform), 1, GL_FALSE, glm::value_ptr(this->viewMat));
}

void FUSIONCORE::Camera::SetProjMatrixUniformLocation(GLuint shader, const char* uniform)
{
	glUniformMatrix4fv(glGetUniformLocation(shader, uniform), 1, GL_FALSE, glm::value_ptr(this->projMat));
}

void FUSIONCORE::Camera::SetCameraMatrixUniformLocation(GLuint shader, const char* uniform)
{
	glUniformMatrix4fv(glGetUniformLocation(shader, uniform), 1, GL_FALSE, glm::value_ptr(this->CameraMat));
}

Vec2<float> FUSIONCORE::Camera::GetScreenRatio(Vec2<int> windowSize)
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

void FUSIONCORE::Camera::SetRatioMatrixUniformLocation(GLuint shader, const char* uniform)
{
	glUniformMatrix4fv(glGetUniformLocation(shader, uniform), 1, GL_FALSE, glm::value_ptr(this->RatioMat));
}

FUSIONCORE::Camera2D::Camera2D()
{
	this->RatioMat = glm::mat4(1.0f);
	this->projMat = glm::mat4(1.0f);
	this->viewMat = glm::mat4(1.0f);
}

void FUSIONCORE::Camera2D::UpdateCameraMatrix(glm::vec3 target, float zoom, Vec2<int> windowSize)
{
	RatioMat = glm::mat4(1.0f);
	projMat = glm::ortho((-1.0f + target.x) / zoom, (1.0f + target.x) / zoom, (-1.0f + target.y) / zoom, (1.0f + target.y) / zoom, -5.0f, 5.0f);
	RatioMat = glm::scale(RatioMat, glm::vec3(GetScreenRatio(windowSize).x, GetScreenRatio(windowSize).y, 1.0f));

	this->CameraMat = RatioMat * projMat;
}

FUSIONCORE::Camera3D::Camera3D()
{
	this->RatioMat = glm::mat4(1.0f);
	this->projMat = glm::mat4(1.0f);
	this->viewMat = glm::mat4(1.0f);
	this->CameraMat = glm::mat4(1.0f);

	targetPosition = glm::vec3(0.0f);
}

void FUSIONCORE::Camera3D::UpdateCameraMatrix(float fovDegree, float aspect, float near, float far, Vec2<int> windowSize)
{
	this->FarPlane = far;
	this->NearPlane = near;
	this->FOV = fovDegree;

	this->projMat = glm::perspective(glm::radians(fovDegree), aspect, near, far);
	this->viewMat = glm::lookAt(Position, Position + Orientation, this->Up);

	this->CameraMat = projMat * viewMat;
}

void FUSIONCORE::Camera3D::Matrix(GLuint shaderprogram, const char* uniform)
{

	glUniformMatrix4fv(glGetUniformLocation(shaderprogram, uniform), 1, GL_FALSE, glm::value_ptr(CameraMat));
	glUniformMatrix4fv(glGetUniformLocation(shaderprogram, "proj"), 1, GL_FALSE, glm::value_ptr(projMat));
	glUniformMatrix4fv(glGetUniformLocation(shaderprogram, "view"), 1, GL_FALSE, glm::value_ptr(viewMat));

}


void FUSIONCORE::Camera3D::SetOrientation(glm::vec3 Orien)
{
	this->Orientation = Orien;
}

void FUSIONCORE::Camera3D::SetPosition(glm::vec3 Pos)
{
	this->Position = Pos;
}
#ifndef FREE_INDUSTRY_STANDARD_CAMERA
void FUSIONCORE::Camera3D::SetTargetPosition(glm::vec3 TargetPos)
{
	this->targetPosition = TargetPos;
}

void FUSIONCORE::Camera3D::SetTarget(Object* object, float Distance)
{
	if (CameraLayout == FF_CAMERA_LAYOUT_INDUSTRY_STANDARD)
	{
		SetPosition(object->GetTransformation().Position + (Distance * -this->Orientation));
		SetTargetPosition(object->GetTransformation().Position);
	}
}
void FUSIONCORE::Camera3D::SetTarget(Object* object, float Distance, glm::vec3 Offset)
{
	if (CameraLayout == FF_CAMERA_LAYOUT_INDUSTRY_STANDARD)
	{
		SetPosition(object->GetTransformation().Position + (Distance * -this->Orientation) + Offset + (Zoom * Orientation));
		SetTargetPosition(object->GetTransformation().Position + Offset);
	}
}
void FUSIONCORE::Camera3D::SetMinMaxZoom(bool clampZoom, float minZoom, float maxZoom)
{
	this->ClampZoom = clampZoom;
	this->MinZoom = minZoom;
	this->MaxZoom = maxZoom;
}
void FUSIONCORE::Camera3D::SetZoomSensitivity(float Speed)
{
	this->ZoomSpeed = Speed;
}
#endif

void FUSIONCORE::Camera3D::HandleInputs(GLFWwindow* window, Vec2<int> WindowSize, int CameraLayout, float speed)
{
	this->CameraLayout = CameraLayout;
	if (CameraLayout == FF_CAMERA_LAYOUT_FIRST_PERSON)
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
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			if (firstclick)
			{
				glfwSetCursorPos(window, (WindowSize.x / 2), (WindowSize.y / 2));
				firstclick = false;
			}

			Vec2<double> mousepos;

			glfwGetCursorPos(window, &mousepos.x, &mousepos.y);

			Vec2<float> rot;

			rot.x = sensitivity * (float)(mousepos.y - (WindowSize.y / 2)) / WindowSize.y;
			rot.y = sensitivity * (float)(mousepos.x - (WindowSize.x / 2)) / WindowSize.x;

			glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rot.x), glm::normalize(glm::cross(Orientation, Up)));

			if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
			{
				Orientation = newOrientation;
			}

			Orientation = glm::rotate(Orientation, glm::radians(-rot.y), Up);
			glfwSetCursorPos(window, (WindowSize.x / 2), (WindowSize.y / 2));
		}

		else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_RELEASE)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			firstclick = true;
		}
	}
	else if (CameraLayout == FF_CAMERA_LAYOUT_INDUSTRY_STANDARD)
	{
		float CameraSensitivity = 10.0f;
		Vec2<double> CurrentMousePos;
		glfwGetCursorPos(window, &CurrentMousePos.x, &CurrentMousePos.y);

		Vec2<double> deltaMouse(CurrentMousePos - MousePosCamera);

		if (ClampZoom)
		{
			if (ScrollAmount.y == 1)
			{
				Zoom += ZoomSpeed;
				Zoom = glm::clamp(Zoom, MinZoom, MaxZoom);
			}
			if (ScrollAmount.y == -1)
			{
				Zoom -= ZoomSpeed;
				Zoom = glm::clamp(Zoom, MinZoom, MaxZoom);
			}
		}

		if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			Position += (float)deltaMouse.y * (speed * 0.5f) * Orientation;
		}
#ifdef FREE_INDUSTRY_STANDARD_CAMERA

		if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
		{
			glm::vec3 PositionVecLength(this->Position.length());

			if (deltaMouse.x > 0)
			{
				glm::vec3 deltaPosition(PositionVecLength * glm::vec3(deltaMouse.x / WindowSize.x) * -glm::normalize(glm::cross(Orientation, Up)));
				Position += deltaPosition;
				targetPosition += deltaPosition;
			}
			if (deltaMouse.x < 0)
			{
				glm::vec3 deltaPosition(PositionVecLength * glm::vec3(-deltaMouse.x / WindowSize.x) * glm::normalize(glm::cross(Orientation, Up)));
				Position += deltaPosition;
				targetPosition += deltaPosition;
			}
			if (deltaMouse.y < 0)
			{
				glm::vec3 deltaPosition(PositionVecLength * glm::vec3(-deltaMouse.y / WindowSize.y) * -Up);
				Position += deltaPosition;
				targetPosition += deltaPosition;
			}
			if (deltaMouse.y > 0)
			{
				glm::vec3 deltaPosition(PositionVecLength * glm::vec3(deltaMouse.y / WindowSize.y) * Up);
				Position += deltaPosition;
				targetPosition += deltaPosition;
	}
}

#endif 
#ifdef FF_CAMERA_LAYOUT_INDUSTRY_STANDARD_FREE_ROTATION
		static bool Break = false;
		static bool AllowPressBreak = false;

		if (!Break)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

			if (firstclick)
			{
				glfwSetCursorPos(window, (WindowSize.x / 2), (WindowSize.y / 2));
				firstclick = false;
			}

			Vec2<double> mousepos;
			glfwGetCursorPos(window, &mousepos.x, &mousepos.y);

			Vec2<float> rot;

			rot.x = CameraSensitivity * (float)(mousepos.y - (WindowSize.y / 2)) / WindowSize.y;
			rot.y = CameraSensitivity * (float)(mousepos.x - (WindowSize.x / 2)) / WindowSize.x;

			glm::vec3 cameraToTarget = targetPosition - Position;
			float distanceToTarget = glm::length(cameraToTarget);
			glm::quat rotation = glm::quat(glm::vec3(-rot.x, -rot.y, 0.0f));
			cameraToTarget = glm::rotate(rotation, cameraToTarget);
			Position = targetPosition - distanceToTarget * glm::normalize(cameraToTarget);
			Orientation = glm::normalize(targetPosition - Position);

			glfwSetCursorPos(window, (WindowSize.x / 2), (WindowSize.y / 2));
		}

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE && !AllowPressBreak)
		{
			AllowPressBreak = true;
		}
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && AllowPressBreak)
		{
			Break = !Break;
			if (Break)
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			firstclick = true;
			AllowPressBreak = false;
		}
#else
		if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS && (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS))
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

			if (firstclick)
			{
				glfwSetCursorPos(window, (WindowSize.x / 2), (WindowSize.y / 2));
				firstclick = false;
			}

			Vec2<double> mousepos;
			glfwGetCursorPos(window, &mousepos.x, &mousepos.y);

			Vec2<float> rot;

			rot.x = CameraSensitivity * (float)(mousepos.y - (WindowSize.y / 2)) / WindowSize.y;
			rot.y = CameraSensitivity * (float)(mousepos.x - (WindowSize.x / 2)) / WindowSize.x;

			glm::vec3 cameraToTarget = targetPosition - Position;
			float distanceToTarget = glm::length(cameraToTarget);
			glm::quat rotation = glm::quat(glm::vec3(-rot.x, -rot.y, 0.0f));
			cameraToTarget = glm::rotate(rotation, cameraToTarget);
			Position = targetPosition - distanceToTarget * glm::normalize(cameraToTarget);
			Orientation = glm::normalize(targetPosition - Position);

			glfwSetCursorPos(window, (WindowSize.x / 2), (WindowSize.y / 2));
		}

		else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_RELEASE)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			firstclick = true;
		}
#endif // FF_CAMERA_LAYOUT_INDUSTRY_STANDARD_FREE_ROTATION

		MousePosCamera(CurrentMousePos);
	}


	ScrollAmount({ 0,0 });
}

void FUSIONCORE::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	ScrollAmount({ xoffset, yoffset });
}

bool FUSIONCORE::IsModelInsideCameraFrustum(FUSIONCORE::Model& model, FUSIONCORE::Camera3D& camera)
{
	auto CameraMatrix = camera.CameraMat;
	auto& ModelScales = model.GetTransformation().InitialObjectScales;
	auto ModelMatrix = model.GetTransformation().GetModelMat4();
	auto& ModelPosition = *model.GetTransformation().OriginPoint;
	auto HalfScales = (ModelScales / 2.0f);
	for (size_t x = 0; x < 2; x++)
	{
		for (size_t y = 0; y < 2; y++)
		{
			for (size_t z = 0; z < 2; z++)
			{
				glm::vec3 Vertex(ModelPosition.x + ((2.0f * x - 1.0f) * HalfScales.x),
					ModelPosition.y + ((2.0f * y - 1.0f) * HalfScales.y),
					ModelPosition.z + ((2.0f * z - 1.0f) * HalfScales.z));

				Vertex = ModelMatrix * glm::vec4(Vertex, 1.0f);
				auto TransformedVertex = CameraMatrix * glm::vec4(Vertex, 1.0f);
				TransformedVertex = TransformedVertex / TransformedVertex.w;
				if ((TransformedVertex.x >= -1.0f && TransformedVertex.x <= 1.0f) && (TransformedVertex.y >= -1.0f && TransformedVertex.y <= 1.0f))
				{
					if (glm::dot(glm::normalize(Vertex - camera.Position), glm::normalize(camera.Orientation)) >= 0.0f)
					{
						return true;
					}
				}
			}
		}
	}
	return false;
};


