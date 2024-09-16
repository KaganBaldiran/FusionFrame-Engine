#pragma once
#include "../FusionCore/Window.hpp"
#include "../FusionCore/EventManager.hpp"
#include "Scene.hpp"

namespace FUSIONENGINE
{
	class Engine
	{
	public:
		Engine();
		~Engine();
		void InitializeEngine();

	private:
		std::vector<FUSIONCORE::Window> Windows;
		FUSIONCORE::EventManager EventManager;
		Scene* ActiveScene;
	};
}



