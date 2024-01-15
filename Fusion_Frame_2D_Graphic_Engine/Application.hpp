#pragma once

#define TARGET_FPS 144
#define ENGINE_DEBUG

#ifndef ENGINE_DEBUG
#define ENGINE_RELEASE
#endif 

class Application
{
public:

	int Run();

private:

};
