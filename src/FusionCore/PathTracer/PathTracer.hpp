#pragma once
#include "../../FusionUtility/Definitions.hpp"
#include "../Buffer.h"
#include "../Shader.h"
#include "../Camera.h"
#include "../Model.hpp"
#include "../../FusionPhysics/Physics.hpp"
#include "../Texture.h"

struct Node;

namespace FUSIONCORE
{

	class PathTracer
	{
	public:
		PathTracer(unsigned int width, unsigned int height,std::vector<Model*>& ModelsToTrace, Shader& shader);
		inline GLuint GetTracedImage() { return image; };
		~PathTracer();
		void VisualizeBVH(FUSIONCORE::Camera3D& Camera, FUSIONCORE::Shader& Shader, glm::vec3 NodeColor);
		void Render(glm::vec2 WindowSize,Shader& shader,Camera3D& camera);
	private:
		GLuint image;
		//SSBO TracerMetaDataBuffer;
		TBO MinBoundData;
		Texture2D MinBoundTexture;

		TBO MaxBoundData;
		Texture2D MaxBoundTexture;

		TBO ChildIndexData;
		Texture2D ChildIndexTexture;

		TBO TriangleIndexData;
		Texture2D TriangleIndexTexture;

		TBO TriangleCountData;
		Texture2D TriangleCountTexture;

		SSBO TracerTriangleDataBuffer;
		int TriangleCount;
		int NodeCount;
		std::vector<Node> PreviouslyBoundingBoxes;
		std::vector<Node> BVHnodes;
	};

}