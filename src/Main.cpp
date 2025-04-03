#include "Scenes/Application.hpp"
#include "FusionFrame.h"
#include "FusionCore/Renderer.hpp"
#include <iostream>
//#include "PathTracerApplication.hpp"
//#include "Scenes/PathTracerApplication.hpp"

int main()
{
	FUSIONCORE::RendererCommand UpdateWindow;
	UpdateWindow.CommandType = FUSIONCORE::FF_RENDERER_COMMAND_TYPE_CHANGE_STATE;

	FUSIONCORE::RendererBackendInstanceCreateInfo BackEndInfo;
	BackEndInfo.ResourceType = FUSIONCORE::FF_RENDERER_CREATE_COMMAND_RESOURCE_TYPE_BACKEND_INSTANCE;
	BackEndInfo.BackEndType = FUSIONCORE::FF_RENDERER_BACKEND_TYPE_OPENGL;
	BackEndInfo.EnableDebug = true;
	BackEndInfo.MajorVersion = 4;
	BackEndInfo.MinorVersion = 6;
	BackEndInfo.WindowWidth = 1000;
	BackEndInfo.WindowHeight = 1000;
	BackEndInfo.WindowName = "Idk man";

	FUSIONCORE::RendererCommand CreateBackEnd;
	CreateBackEnd.CommandType = FUSIONCORE::FF_RENDERER_COMMAND_TYPE_CREATE_RESOURCE;
	CreateBackEnd.Info = BackEndInfo;

	FUSIONCORE::Renderer renderer(256);
	renderer.Submit(CreateBackEnd);

	while (true)
	{
		
		renderer.Submit(UpdateWindow);
	}
	renderer.SetTerminate();
	renderer.WaitRendererThread();
	return 0;
}