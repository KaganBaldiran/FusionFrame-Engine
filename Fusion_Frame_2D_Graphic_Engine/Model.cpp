#include "Model.hpp"

void FUSIONOPENGL::Model::Draw(Camera3D& camera, Shader& shader, std::function<void()> &ShaderPreperations)
{
	for (size_t i = 0; i < Meshes.size(); i++)
	{
		Meshes[i].Draw(camera, shader, ShaderPreperations);
	}
}


