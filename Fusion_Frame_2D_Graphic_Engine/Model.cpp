#include "Model.hpp"

void FUSIONOPENGL::Model::Draw(Camera3D& camera, Shader& shader, std::function<void()> &ShaderPreperations)
{
	for (size_t i = 0; i < Meshes.size(); i++)
	{
		Meshes[i].Draw(camera, shader, ShaderPreperations);
	}
}

void FUSIONOPENGL::Model::Draw(Camera3D& camera, Shader& shader, Material& material, std::function<void()>& ShaderPreperations)
{
	for (size_t i = 0; i < Meshes.size(); i++)
	{
		Meshes[i].Draw(camera, shader,material, ShaderPreperations);
	}
}

void FUSIONOPENGL::Model::FindGlobalMeshScales()
{

	float maxX = -std::numeric_limits<float>::infinity();
	float maxY = -std::numeric_limits<float>::infinity();
	float maxZ = -std::numeric_limits<float>::infinity();
	float minX = std::numeric_limits<float>::infinity();
	float minY = std::numeric_limits<float>::infinity();
	float minZ = std::numeric_limits<float>::infinity();

	for (size_t j = 0; j < Meshes.size(); j++)
	{
		auto& VertexArray = Meshes[j].GetVertexArray();
		Vertex origin = VertexArray[0]; 

		for (unsigned int k = 0; k < VertexArray.size(); k++) {

			Vertex vertex;
			vertex.Position.x = VertexArray[k].Position.x - origin.Position.x;
			vertex.Position.y = VertexArray[k].Position.y - origin.Position.y;
			vertex.Position.z = VertexArray[k].Position.z - origin.Position.z;

			maxX = std::max(maxX, vertex.Position.x);
			maxY = std::max(maxY, vertex.Position.y);
			maxZ = std::max(maxZ, vertex.Position.z);
			minX = std::min(minX, vertex.Position.x);
			minY = std::min(minY, vertex.Position.y);
			minZ = std::min(minZ, vertex.Position.z);
		}
	}


	float meshWidth = maxX - minX;
	float meshHeight = maxY - minY;
	float meshDepth = maxZ - minZ;

	transformation.ObjectScales.x = meshWidth;
	transformation.ObjectScales.y = meshHeight;
	transformation.ObjectScales.z = meshDepth;

	maxX = -std::numeric_limits<float>::infinity();
	maxY = -std::numeric_limits<float>::infinity();
	maxZ = -std::numeric_limits<float>::infinity();
	minX = std::numeric_limits<float>::infinity();
	minY = std::numeric_limits<float>::infinity();
	minZ = std::numeric_limits<float>::infinity();

	transformation.scale_avg = (transformation.ObjectScales.x + transformation.ObjectScales.y + transformation.ObjectScales.z) / 3;
	transformation.dynamic_scale_avg = transformation.scale_avg;

	transformation.ObjectScales.x = transformation.ObjectScales.x;
	transformation.ObjectScales.y = transformation.ObjectScales.y;
	transformation.ObjectScales.z = transformation.ObjectScales.z;


	std::cout << "Model width: " << transformation.ObjectScales.x << " Model height: " << transformation.ObjectScales.y << " Model Depth: " << transformation.ObjectScales.z << "\n";
	std::cout << "Scale avg: " << transformation.scale_avg << "\n";

}

