#pragma once
#include <glew.h>
#include <glfw3.h>
#include "Log.h"
#include "VectorMath.h"
#include "Buffer.h"
#include "Mesh.h"

class Light
{
public:
	Light();
	FUSIONOPENGL::WorldTransform* GetTransformation() { return &this->transformation; };

private:
	FUSIONOPENGL::WorldTransform transformation;
	Buffer objectBuffer;
};