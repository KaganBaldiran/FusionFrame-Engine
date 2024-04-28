#pragma once
#define TARGET_FPS 144
#define ENGINE_DEBUG

#ifndef ENGINE_DEBUG
#define ENGINE_RELEASE
#endif 

struct GLFWwindow;

class Application
{
public:
	int Run();
	bool IsKeyPressedOnce(GLFWwindow* window , int Key, bool& Signal);
	float RoundNonZeroToOne(float input);

private:

};
