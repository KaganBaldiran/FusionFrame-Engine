#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "Buffer.h"
#include <map>
#define FF_ORIGIN glm::vec3(0.0f,0.0f,0.0f)

namespace FUSIONOPENGL
{
	struct TransformAction
	{
		glm::vec3 Transformation;
	};

	struct RotateAction
	{
		float Degree;
		glm::vec3 Vector;
	};

	struct ScaleAction
	{
		glm::vec3 Scale;
	};

	class WorldTransform
	{
	public:

		WorldTransform()
		{
			Position = glm::vec3(0.0f, 0.0f, 0.0f);
			ScaleFactor = glm::vec3(1.0f, 1.0f, 1.0f);
		}

		glm::mat4 TranslationMatrix = glm::mat4(1.0f);
		glm::mat4 RotationMatrix = glm::mat4(1.0f);
		glm::mat4 ScalingMatrix = glm::mat4(1.0f);

		glm::vec3 ObjectScales;
		glm::vec3 InitialObjectScales;

		glm::vec3 Position;
		glm::vec3* OriginPoint;

		float scale_avg;
		float dynamic_scale_avg;
		std::vector<TransformAction> LastTransforms;
		std::vector<RotateAction> LastRotations;
		std::vector<ScaleAction> LastScales;

		glm::vec3 ScaleFactor;

		void SetModelMatrixUniformLocation(GLuint shader, const char* uniform);
		virtual void Translate(glm::vec3 v);
		void Scale(glm::vec3 v);
		void Rotate(glm::vec3 v, float angle);

		//No transform history for children
		void TranslateNoTraceBack(glm::vec3 v);
		void ScaleNoTraceBack(glm::vec3 v);
		void RotateNoTraceBack(glm::vec3 v, float angle);

		glm::mat4 GetModelMat4()
		{
			return TranslationMatrix * RotationMatrix * ScalingMatrix;
		};
	};

	class WorldTransformForLights : public WorldTransform
	{
	public:

		WorldTransformForLights(std::vector<glm::vec3>& LightPositions , int LightID)
		{
			Position = glm::vec3(0.0f, 0.0f, 0.0f);
			ScaleFactor = glm::vec3(1.0f, 1.0f, 1.0f);
			this->LightPositions = &LightPositions;
			this->LightID = LightID;
		}
		void Translate(glm::vec3 v) override;

	private:
		std::vector<glm::vec3>* LightPositions;
		int LightID;
	};
}