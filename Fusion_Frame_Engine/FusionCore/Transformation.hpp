#pragma once
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "Buffer.h"
#include <map>
#include "../FusionUtility/FusionDLLExport.h"
#define FF_ORIGIN glm::vec3(0.0f,0.0f,0.0f)

namespace FUSIONCORE
{
	struct LightData;

	struct FUSIONFRAME_EXPORT TransformAction
	{
		glm::vec3 Transformation;
	};

	struct FUSIONFRAME_EXPORT RotateAction
	{
		float Degree;
		glm::vec3 Vector;
	};

	struct FUSIONFRAME_EXPORT ScaleAction
	{
		glm::vec3 Scale;
	};

	class FUSIONFRAME_EXPORT WorldTransform
	{
	public:

		WorldTransform()
		{
			Position = glm::vec3(0.0f, 0.0f, 0.0f);
			ScaleFactor = glm::vec3(1.0f, 1.0f, 1.0f);
			IsTransformedQuadTree = false;
			IsTransformedCollisionBox = false;
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

		//Optimization for QuadTree world partitioning 
		bool IsTransformedQuadTree;
		//Optimization for collision box attribute update 
		bool IsTransformedCollisionBox;

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

	class FUSIONFRAME_EXPORT WorldTransformForLights : public WorldTransform
	{
	public:

		WorldTransformForLights(LightData* Light, int LightID);
		void Translate(glm::vec3 v) override;

	private:
		glm::vec4* LightPosition;
		int LightID;
	};
}