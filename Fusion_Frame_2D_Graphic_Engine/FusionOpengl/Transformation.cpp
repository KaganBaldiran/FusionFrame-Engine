#include "Transformation.hpp"

void FUSIONOPENGL::WorldTransform::SetModelMatrixUniformLocation(GLuint shader, const char* uniform)
{
	glUniformMatrix4fv(glGetUniformLocation(shader, uniform), 1, GL_FALSE, glm::value_ptr(GetModelMat4()));
}

void FUSIONOPENGL::WorldTransform::Translate(glm::vec3 v)
{
	TranslationMatrix = glm::translate(TranslationMatrix, v);
	TransformAction action;
	action.Transformation = v;
	this->LastTransforms.push_back(action);

	Position.x = TranslationMatrix[3][0];
	Position.y = TranslationMatrix[3][1];
	Position.z = TranslationMatrix[3][2];
}

void FUSIONOPENGL::WorldTransform::Scale(glm::vec3 v)
{
	ScalingMatrix = glm::scale(ScalingMatrix, v);
	ObjectScales *= v;
	ScaleFactor *= v;
	scale_avg = (ObjectScales.x + ObjectScales.y + ObjectScales.z) / 3.0f;

	ScaleAction action;
	action.Scale = v;
	this->LastScales.push_back(action);
}

void FUSIONOPENGL::WorldTransform::Rotate(glm::vec3 v, float angle)
{
	RotationMatrix = glm::rotate(RotationMatrix, glm::radians(angle), v);
	RotateAction action;
	action.Degree = angle;
	action.Vector = v;
	this->LastRotations.push_back(action);
}

void FUSIONOPENGL::WorldTransform::TranslateNoTraceBack(glm::vec3 v)
{
	TranslationMatrix = glm::translate(TranslationMatrix, v);

	Position.x = TranslationMatrix[3][0];
	Position.y = TranslationMatrix[3][1];
	Position.z = TranslationMatrix[3][2];
}

void FUSIONOPENGL::WorldTransform::ScaleNoTraceBack(glm::vec3 v)
{
	ScalingMatrix = glm::scale(ScalingMatrix, v);
	ObjectScales *= v;
	ScaleFactor *= v;
	scale_avg = (ObjectScales.x + ObjectScales.y + ObjectScales.z) / 3.0f;
}

void FUSIONOPENGL::WorldTransform::RotateNoTraceBack(glm::vec3 v, float angle)
{
	RotationMatrix = glm::rotate(RotationMatrix, glm::radians(angle), v);
}

void FUSIONOPENGL::WorldTransformForLights::Translate(glm::vec3 v)
{
	TranslationMatrix = glm::translate(TranslationMatrix, v);
	TransformAction action;
	action.Transformation = v;
	this->LastTransforms.push_back(action);

	Position.x = TranslationMatrix[3][0];
	Position.y = TranslationMatrix[3][1];
	Position.z = TranslationMatrix[3][2];

	LightPositions->at(LightID) = Position;
}
