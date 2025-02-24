#include <FusionFrame.h>
#include "PathTracer/PathTracer.hpp"
#include <memory>
#include "imgui.h"       
#include "imgui_internal.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "ImGuizmo.h"  

#define SPEED 6.0f
#define CAMERA_CLOSE_PLANE 0.5f
#define CAMERA_FAR_PLANE 800.0f

class PathTracingDemo
{
public:
	PathTracingDemo();
	~PathTracingDemo();

	void Run();
private:
	void RandomizeLights(FUSIONUTIL::DefaultShaders& Shaders, FUSIONCORE::Shader& Destination);

	const int width = 1000;
	const int height = 1000;

	FUSIONCORE::Window ApplicationWindow;
	std::shared_ptr <FUSIONUTIL::DefaultShaders> Shaders;

	std::shared_ptr<FUSIONCORE::Shader> PathTraceComputeShader;
	std::shared_ptr<FUSIONCORE::Shader> RadianceBinningComputeShader;
	std::shared_ptr<FUSIONCORE::Shader> RadiancePrefixSumComputeShader;
	std::shared_ptr<FUSIONCORE::Shader> RadianceGroupPrefixSumComputeShader;
	std::shared_ptr<FUSIONCORE::Shader> RadianceAggregatePrefixSumsComputeShader;
	std::shared_ptr<FUSIONCORE::Shader> BVHvisualizeShader;

	std::vector<FUSIONCORE::Light> Lights;

	std::shared_ptr<FUSIONCORE::CubeMap> cubemap;

	std::shared_ptr<FUSIONCORE::GeometryBuffer> Gbuffer;
	std::shared_ptr<FUSIONCORE::ScreenFrameBuffer> ScreenFrameBuffer;

	std::vector<std::pair<FUSIONCORE::Model*, FUSIONCORE::Material*>> models;

	std::shared_ptr<FUSIONCORE::Model> ImportedModel;
	FUSIONCORE::Model* SelectedModel;

	glm::ivec2 WindowSize;
	std::vector<FUSIONCORE::OmniShadowMap*> shadowMaps;
	std::function<void()> shaderPrep = []() {};

	std::shared_ptr<FUSIONCORE::Camera3D> camera3d;

	glm::ivec2 PrevWindowSize;
	glm::ivec2 WindowPos;
	bool IsFullScreen = false;

	double DeltaTime = 0.0;
	double StartTime = 0.0;
	bool Debug = false;

	unsigned int HDRIsize;

	std::function<void()> OnSaveScreen;
	std::function<void()> OnImportModel;
	std::function<void()> AdditionalFunctionality;

	float FOV;

	std::shared_ptr<FUSIONCORE::PathTracer> pathtracer;
	glm::dvec2 CurrentMousePos;
	glm::dvec2 PrevMousePos;
	bool AllowPathTrace;

	float Speed;

	ImGuizmo::OPERATION SelectedOperation;
	bool IsSceneAltered;

	FUSIONUTIL::VideoMode mode;
};
