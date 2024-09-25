#pragma once
#include "../../FusionUtility/Definitions.hpp"
#include "../Buffer.h"
#include "../Shader.h"
#include "../Camera.h"
#include "../Model.hpp"
#include "../../FusionPhysics/Physics.hpp"
#include "../Texture.h"

struct BVHnode;
struct BottomUpNode;

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

		TBO TriangleNormalsData;
		Texture2D TriangleNormalsTexture;

		SSBO TracerTriangleDataBuffer;
		int TriangleCount;
		int NodeCount;

		int ModelNodeCount;

		bool IsInitialized;

		std::vector<BVHnode> PreviouslyBoundingBoxes;

		std::vector<BVHnode> TopDownBVHnodes;
		std::vector<BVHnode> BottomUpBVHNodes;
	};

}