#pragma once
#define TARGET_FPS 144
#define ENGINE_DEBUG

#ifndef ENGINE_DEBUG
#define ENGINE_RELEASE
#endif 

struct GLFWwindow;

//#define FF_MAX_CASCADES 6
//#define FF_MAX_CASCADED_SHADOW_MAP_COUNT 2
#include "FusionFrame.h"

class Application
{
public:
	int Run();
	float RoundNonZeroToOne(float input);

private:

};
