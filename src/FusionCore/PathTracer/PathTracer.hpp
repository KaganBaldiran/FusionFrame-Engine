#pragma once
#include "../../FusionUtility/Definitions.hpp"
#include "../../FusionUtility/StopWatch.h"
#include "../Buffer.h"
#include "../Shader.h"
#include "../Camera.h"
#include "../Model.hpp"
#include "../../FusionPhysics/Physics.hpp"
#include "../Texture.h"
#include "../Window.hpp"
#include "../../FusionUtility/Hashing.hpp"
#include <OpenImageDenoise/oidn.hpp>

static struct BVHnode;
namespace oidn
{
	class DeviceRef;
	class FilterRef;
}

namespace FUSIONCORE
{
	class PathTracer
	{
	public:
		PathTracer(unsigned int width, unsigned int height,std::vector<std::pair<Model*,Material*>>& ModelsToTrace);
		inline GLuint GetTracedImage() { return image; };
		GLint IsPathTracingDone();
		void PathTracerDashBoard();
		~PathTracer();
		void VisualizeBVH(FUSIONCORE::Camera3D& Camera, FUSIONCORE::Shader& Shader, glm::vec3 NodeColor);
		void Render(Window& window,Shader& shader,Camera3D& camera, CubeMap* Cubemap = nullptr,unsigned int DenoiseSampleCount = 60);
	private:
		void Denoise(void* ColorBuffer, void* outputBuffer);

		FUSIONUTIL::Timer timer;

		oidn::DeviceRef device;
		oidn::FilterRef filter;
		oidn::BufferRef colorBuf;

		GLuint image,pbo,queryObject;
		glm::ivec2 ImageSize;

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

		TBO AlbedoData;
		Texture2D AlbedoTexture;

		TBO RoughnessData;
		Texture2D RoughnessTexture;

		TBO MetallicData;
		Texture2D MetallicTexture;

		TBO AlphaData;
		Texture2D AlphaTexture;

		TBO EmissiveData;
		Texture2D EmissiveTexture;

		TBO EmissiveObjectsData;
		Texture2D EmissiveObjectsTexture;

		TBO TracerTriangleUVdata;
		Texture2D TracerTriangleUVTexture;

		TBO TracerTriangleNormalData;
		Texture2D TracerTriangleNormalsTexture;

		TBO TracerTriangleTangentBitangentData;
		Texture2D TracerTriangleTangentBitangentTexture;

		TBO TracerTrianglePositionsData;
		Texture2D TracerTrianglePositionsTexture;

		SSBO ModelMatricesData;
		SSBO ModelTextureHandlesData;

		int TriangleCount;
		int NodeCount;
		int ModelNodeCount;
		size_t ModelCount;
		bool IsInitialized;
		bool ShouldPathTrace;
		int EmissiveObjectCount;

		int ProgressiveRenderedFrameCount;

		std::vector<BVHnode> TopDownBVHnodes;
		std::vector<BVHnode> BottomUpBVHNodes;

		std::vector<Model*> Models;

	};

}