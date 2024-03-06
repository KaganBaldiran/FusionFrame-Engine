#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "../FusionCore/Model.hpp"
#include <memory>

namespace FUSIONCORE
{
	namespace MESHOPERATIONS
	{
	  bool ExportObj(const char* FilePath, FUSIONCORE::Model& model);
	  bool ImportObj(const char* FilePath, FUSIONCORE::Model& model);

	  void SmoothObject(FUSIONCORE::Mesh& mesh);
	  void LoopSubdivision(FUSIONCORE::Mesh& Mesh, int level);
	  void CalculateTangentBitangent(std::vector<std::shared_ptr<FUSIONCORE::Vertex>>& vertices, std::vector<unsigned int>& indices);
	  void CollapseDecimation(FUSIONCORE::Mesh& Mesh, int level);

	  std::vector<glm::vec3> DistributePointsOnMeshSurface(FUSIONCORE::Mesh& Mesh, unsigned int PointCount , unsigned int seed);
	  std::vector<glm::vec3> DistributePointsOnMeshSurface(FUSIONCORE::Mesh& Mesh,FUSIONCORE::WorldTransform& Transformation, unsigned int PointCount, unsigned int seed);
	  
	  //Filling instance vbo with given data.
	  void FillInstanceDataVBO(FUSIONCORE::VBO& DestVBO ,std::vector<glm::vec3> &InstanceData);
	}
}