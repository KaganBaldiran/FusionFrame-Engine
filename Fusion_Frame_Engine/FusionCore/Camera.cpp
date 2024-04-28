#include "Camera.h"
#include "../FusionUtility/glm/glm.hpp"
#include "../FusionUtility/glm/gtc/matrix_transform.hpp"
#include "../FusionUtility/glm/gtc/type_ptr.hpp"
#include "../FusionUtility/glm/gtx/rotate_vector.hpp"
#include "../FusionUtility/glm/gtx/vector_angle.hpp"
#include "Model.hpp"
#include "../FusionPhysics/Octtree.hpp"
#include "Light.hpp"
#include <glew.h>
#include <glfw3.h>

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

struct alignas(16) Cluster
{
	glm::vec4 Min;
	glm::vec4 Max;
	uint Count;
	uint LightIndices[MAX_LIGHT_PER_CLUSTER];
};

FUSIONCORE::Camera3D::Camera3D(glm::vec3 ClusterGridSize)
{
	this->RatioMat = glm::mat4(1.0f);
	this->projMat = glm::mat4(1.0f);
	this->viewMat = glm::mat4(1.0f);
	this->CameraMat = glm::mat4(1.0f);

	targetPosition = glm::vec3(0.0f);

	this->ClusterGridSize = ClusterGridSize;

	ClusterSSBO.Bind();
	ClusterSSBO.BufferDataFill(GL_SHADER_STORAGE_BUFFER, sizeof(Cluster) * ClusterGridSize.x * ClusterGridSize.y * ClusterGridSize.z, nullptr, GL_DYNAMIC_COPY);
	BindSSBONull();
}

void FUSIONCORE::Camera3D::UpdateCameraMatrix(float fovDegree, float aspect, float near, float far, Vec2<int> windowSize)
{
	this->FarPlane = far;
	this->NearPlane = near;
	this->FOV = fovDegree;
	this->Aspect = aspect;

	this->w_width = windowSize.x;
	this->w_height = windowSize.y;

	this->projMat = glm::perspective(glm::radians(fovDegree), aspect, near, far);
	this->viewMat = glm::lookAt(Position, Position + Orientation, this->Up);

	this->CameraMat = projMat * viewMat;
	this->ProjectionViewMat = CameraMat;
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
	this->PlanarOrientation = { Orien.x , 0.0f , Orien.z };
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

void FUSIONCORE::Camera3D::UpdateCameraClusters(Shader& CameraClusterComputeShader, Shader& CameraLightCullingComputeShader)
{
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	CameraClusterComputeShader.use();

	CameraClusterComputeShader.setMat4("InverseProjectionMatrix", glm::inverse(projMat));
	CameraClusterComputeShader.setVec3("GridSize", ClusterGridSize);
	CameraClusterComputeShader.setVec2("ScreenSize", glm::vec2(1920, 1080));
	CameraClusterComputeShader.setFloat("FarPlane", this->FarPlane);
	CameraClusterComputeShader.setFloat("ClosePlane", this->NearPlane);

	this->ClusterSSBO.BindSSBO(0);

    glDispatchCompute(ClusterGridSize.x, ClusterGridSize.y, ClusterGridSize.z);

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	CameraLightCullingComputeShader.use();
	CameraLightCullingComputeShader.setMat4("viewMat", this->viewMat);
	CameraLightCullingComputeShader.setVec3("GridSize", ClusterGridSize);
	CameraLightCullingComputeShader.setVec2("ScreenSize", glm::vec2(1920,1080));
	CameraLightCullingComputeShader.setFloat("FarPlane", this->FarPlane);
	CameraLightCullingComputeShader.setFloat("ClosePlane", this->NearPlane);

	this->ClusterSSBO.BindSSBO(0);
	SendLightsShader(CameraLightCullingComputeShader);

	const uint LOCAL_SIZE = 128;
	glDispatchCompute((ClusterGridSize.x * ClusterGridSize.y * ClusterGridSize.z) / LOCAL_SIZE, 1, 1);

	FUSIONCORE::UseShaderProgram(0);
}

void FUSIONCORE::Camera3D::SendClustersShader(uint BindingPoint)
{
	this->ClusterSSBO.BindSSBO(BindingPoint);
}

void FUSIONCORE::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	ScrollAmount({ xoffset, yoffset });
}

bool FUSIONCORE::IsModelInsideCameraFrustumAABB(FUSIONCORE::Model& model, FUSIONCORE::Camera3D& camera)
{
	auto CameraMatrix = camera.CameraMat;
	auto& ModelScales = model.GetTransformation().InitialObjectScales;
	auto ModelMatrix = model.GetTransformation().GetModelMat4();
	auto& ModelPosition = *model.GetTransformation().OriginPoint;
	auto HalfScales = (ModelScales * 0.5f);
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
}

float computeProjectedRadius(float fovy, float d, float r) 
{
	float fov = fovy / 2.0f * std::_Pi_val / 180.0f;
	return 1.0f / std::tan(fov) * r / std::sqrt(d * d - r * r);
}

bool FUSIONCORE::IsModelInsideCameraFrustumSphere(FUSIONCORE::Model& model, FUSIONCORE::Camera3D& camera, float RadiusPadding)
{
	glm::vec3 ScaledModelSize = model.GetTransformation().ObjectScales;
	auto SpherePosition = *model.GetTransformation().OriginPoint;

	glm::vec3 WorldSpherePosition = model.GetTransformation().GetModelMat4() * glm::vec4(SpherePosition, 1.0f);
	if (glm::dot(glm::normalize(WorldSpherePosition - camera.Position), glm::normalize(camera.Orientation)) >= 0.0f)
	{
		auto TransformedSpherePosition = camera.CameraMat * glm::vec4(WorldSpherePosition, 1.0f);
		TransformedSpherePosition = TransformedSpherePosition / TransformedSpherePosition.w;

		float SphereProjectedRadius = glm::max(ScaledModelSize.x, glm::max(ScaledModelSize.y, ScaledModelSize.z));
		SphereProjectedRadius = computeProjectedRadius(camera.GetCameraFOV(), glm::distance(WorldSpherePosition, camera.Position), SphereProjectedRadius * 0.5f);

		if (RadiusPadding > 0.0f)
		{
			SphereProjectedRadius += SphereProjectedRadius * RadiusPadding;
		}

		return (TransformedSpherePosition.x - SphereProjectedRadius <= 1.0f && TransformedSpherePosition.x + SphereProjectedRadius >= -1.0f) &&
			   (TransformedSpherePosition.y - SphereProjectedRadius <= 1.0f && TransformedSpherePosition.y + SphereProjectedRadius >= -1.0f);
	}
	return false;
}

bool FUSIONCORE::IsObjectQuadInsideCameraFrustum(FUSIONCORE::Object& model, FUSIONCORE::Camera3D& camera)
{
	auto& Quads = model.GetAssociatedQuads();
	auto CameraMatrix = camera.CameraMat;

	for (auto& Quad : Quads)
	{
		auto& QuadScales = Quad->Size;
		auto& QuadCenter = Quad->Center;
		auto HalfScales = QuadScales * 0.5f;

		for (size_t x = 0; x < 2; x++)
		{
			for (size_t y = 0; y < 2; y++)
			{
				for (size_t z = 0; z < 2; z++)
				{
					glm::vec3 Vertex(QuadCenter.x + ((2.0f * x - 1.0f) * HalfScales.x),
						             QuadCenter.y + ((2.0f * y - 1.0f) * HalfScales.y),
						             QuadCenter.z + ((2.0f * z - 1.0f) * HalfScales.z));

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
	}
	return false;
}

 std::pair<glm::vec3, glm::vec3> FUSIONCORE::GetCameraFrustum(FUSIONCORE::Camera3D& camera)
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

	return std::make_pair<glm::vec3, glm::vec3>({ minX,minY,minZ }, { maxX,maxY,maxZ });
}





