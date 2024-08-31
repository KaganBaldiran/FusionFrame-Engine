#include "BVH.hpp"
#include <glew.h>


FUSIONCORE::BVH::BVH(std::vector<Model*> Models)
{
	CombinedObjectBuffer = std::make_unique<SSBO>();
	BVHbuffer = std::make_unique<SSBO>();


	GLsizeiptr size = 0;
	for (auto& model : Models)
	{
		auto &meshes = model->Meshes;
		for (auto& mesh : meshes)
		{

		}
	}

	CombinedObjectBuffer->Bind();
	//CombinedObjectBuffer->BufferDataFill(GL_SHADER_STORAGE_BUFFER,)
}
