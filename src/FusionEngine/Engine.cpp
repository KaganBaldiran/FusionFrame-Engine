#include "Engine.hpp"

void FUSIONENGINE::Engine::InitializeEngine()
{
	FUSIONUTIL::InitializeEngineBuffers();
	FUSIONCORE::InitializeCascadedShadowMapTextureArray(4096, 1, 1024);
	FUSIONUTIL::DefaultShaders Shaders;
	FUSIONUTIL::InitializeDefaultShaders(Shaders);
}
