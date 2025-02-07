#include "Engine.hpp"

void FUSIONENGINE::Engine::InitializeEngine()
{
	FUSIONCORE::InitializeCascadedShadowMapTextureArray(4096, 1, 1024);
	FUSIONUTIL::DefaultShaders Shaders;
}
