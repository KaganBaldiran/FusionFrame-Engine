#include "Transformation.hpp"
#include "Light.hpp"
#include <glew.h>
#include <glfw3.h>

void FUSIONCORE::WorldTransform::SetModelMatrixUniformLocation(GLuint shader, const char* uniform)
{
	glUniformMatrix4fv(glGetUniformLocation(shader, uniform), 1, GL_FALSE, glm::value_ptr(GetModelMat4()));
}

void FUSIONCORE::WorldTransform::Translate(glm::vec3 v)
{
	TranslationMatrix = glm::translate(TranslationMatrix, v);
	TransformAction action;
	action.Transformation = v;
	this->LastTransforms.push_back(action);

	Position.x = TranslationMatrix[3][0];
	Position.y = TranslationMatrix[3][1];
	Position.z = TranslationMatrix[3][2];
	IsTransformedQuadTree = true;
	IsTransformedCollisionBox = true;
}

void FUSIONCORE::WorldTransform::Scale(glm::vec3 v)
{
	ScalingMatrix = glm::scale(ScalingMatrix, v);
	ObjectScales *= v;
	ScaleFactor *= v;
	scale_avg = (ObjectScales.x + ObjectScales.y + ObjectScales.z) / 3.0f;

	ScaleAction action;
	action.Scale = v;
	this->LastScales.push_back(action);
	IsTransformedQuadTree = true;
	IsTransformedCollisionBox = true;
}

void FUSIONCORE::WorldTransform::Rotate(glm::vec3 v, float angle)
{
	RotationMatrix = glm::rotate(RotationMatrix, glm::radians(angle), v);
	RotateAction action;
	action.Degree = angle;
	action.Vector = v;
	this->LastRotations.push_back(action);
	IsTransformedQuadTree = true;
	IsTransformedCollisionBox = true;
}

void FUSIONCORE::WorldTransform::TranslateNoTraceBack(glm::vec3 v)
{
	TranslationMatrix = glm::translate(TranslationMatrix, v);

	Position.x = TranslationMatrix[3][0];
	Position.y = TranslationMatrix[3][1];
	Position.z = TranslationMatrix[3][2];
	IsTransformedQuadTree = true;
	IsTransformedCollisionBox = true;
}

void FUSIONCORE::WorldTransform::ScaleNoTraceBack(glm::vec3 v)
{
	ScalingMatrix = glm::scale(ScalingMatrix, v);
	ObjectScales *= v;
	ScaleFactor *= v;
	scale_avg = (ObjectScales.x + ObjectScales.y + ObjectScales.z) / 3.0f;
	IsTransformedQuadTree = true;
	IsTransformedCollisionBox = true;
}

void FUSIONCORE::WorldTransform::RotateNoTraceBack(glm::vec3 v, float angle)
{
	RotationMatrix = glm::rotate(RotationMatrix, glm::radians(angle), v);
	IsTransformedQuadTree = true;
	IsTransformedCollisionBox = true;
}

FUSIONCORE::WorldTransformForLights::WorldTransformForLights(LightData* Light, int LightID)
{
	Position = glm::vec3(0.0f, 0.0f, 0.0f);
	ScaleFactor = glm::vec3(1.0f, 1.0f, 1.0f);
	this->LightPosition = &Light->Position;
	this->LightID = LightID;
}

void FUSIONCORE::WorldTransformForLights::Translate(glm::vec3 v)
{
	TranslationMatrix = glm::translate(TranslationMatrix, v);
	TransformAction action;
	action.Transformation = v;
	this->LastTransforms.push_back(action);

	Position.x = TranslationMatrix[3][0];
	Position.y = TranslationMatrix[3][1];
	Position.z = TranslationMatrix[3][2];

	*LightPosition = glm::vec4(Position,1.0f);
	IsTransformedQuadTree = true;
	IsTransformedCollisionBox = true;
}
