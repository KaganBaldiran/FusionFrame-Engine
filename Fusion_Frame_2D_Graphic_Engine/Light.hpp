#pragma once
#include <glew.h>
#include <glfw3.h>
#include "Log.h"
#include "VectorMath.h"
#include "Buffer.h"
#include "Mesh.h"
#include "Object.hpp"
#include <memory>

namespace FUSIONOPENGL
{
	class Light : public Object
	{
	public:
		Light();
		WorldTransform* GetTransformation() { return &this->transformation; };

	private:
		WorldTransform transformation;
		std::unique_ptr<Mesh3D> LightMesh;
	};
}