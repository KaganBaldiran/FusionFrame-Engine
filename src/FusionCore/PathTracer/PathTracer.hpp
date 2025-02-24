#pragma once
#include <FusionFrame.h>
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
		PathTracer(const unsigned int& width, const unsigned int& height,std::vector<std::pair<Model*,Material*>>& ModelsToTrace, FUSIONUTIL::DefaultShaders& Shaders);
		inline GLuint GetTracedImage() { return image; };
		
		void PathTracerDashBoard(std::function<void()> OnImportModel, std::function<void()> OnSaveScreen,std::function<void()> AdditionalFunctionality);
		~PathTracer();
		void VisualizeBVH(FUSIONCORE::Camera3D& Camera, FUSIONCORE::Shader& Shader, glm::vec3 NodeColor);
		void Render(Window& window,Shader& shader,Camera3D& camera, CubeMap* Cubemap = nullptr,unsigned int DenoiseSampleCount = 60);
		void ConstructBVH(std::vector<std::pair<Model*, Material*>>& ModelsToTrace, FUSIONUTIL::DefaultShaders& Shaders);

		inline void SetShouldRestart(const bool& value) { ShouldRestart = value; };

		inline const bool& ShouldDisplayBVH() { return ShouldDisplayBVHv; };
	private:
		void Denoise(void* ColorBuffer, void* NormalBuffer, void* AlbedoBuffer, void* outputBuffer);
		void InitializeImages(const unsigned int& width, const unsigned int& height);

		FUSIONUTIL::Timer timer;
		double InitialTime;

		float TotalEmissiveArea = 0.0f;

		oidn::DeviceRef device;
		oidn::FilterRef filter;
		oidn::BufferRef colorBuf;
		oidn::BufferRef NormalBuf;
		oidn::BufferRef AlbedoBuf;

		GLuint image,NormalImage, AlbedoImage;
		glm::ivec2 ImageSize;

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

		TBO ClearCoatData;
		Texture2D ClearCoatTexture;

		TBO EmissiveObjectsData;
		Texture2D EmissiveObjectsTexture;

		TBO TracerTriangleUVdata;
		Texture2D TracerTriangleUVTexture;

		SSBO ModelMatricesData;
		SSBO ModelTextureHandlesData;
		SSBO MaterialFloatValuesData;
		SSBO BVHvec4Data;
		SSBO BVHfloatData;
		SSBO MeshData;

		int AlbedoCount;

		int TriangleCount;
		int NodeCount;
		int ModelNodeCount;
		size_t ModelCount;
		bool IsInitialized;
		bool ShouldRestart;
		bool ShouldDisplayBVHv;
		bool EnableDenoising;
		int EmissiveObjectCount;

		FUSIONCORE::CubeMap* PreviouslyUsedCubeMap;
		unsigned int PreviousImportedHDRIcount;

		glm::vec4 DoFattributes;
		bool IsDoFenabled;
		int DoFbounceCount;

		int TargetBounceCount;
		float EnvironmentLightIntensity;

		bool ShouldDisplayTheEnv;

		int TargetSampleCount;
		bool IsDenoised;
		int ProgressiveRenderedFrameCount;

		std::unique_ptr<std::uniform_real_distribution<float>> RandomSeed;

		std::vector<BVHnode> TopDownBVHnodes;
		std::vector<BVHnode> BottomUpBVHNodes;
	};

}