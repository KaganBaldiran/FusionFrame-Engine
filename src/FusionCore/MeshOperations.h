#pragma once
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "../FusionCore/Model.hpp"
#include "../FusionPhysics/Physics.hpp"
#include <memory>
#include "../FusionUtility/FusionDLLExport.h"

namespace FUSIONCORE
{
	namespace MESHOPERATIONS
	{
	  FUSIONFRAME_EXPORT_FUNCTION bool ExportObj(const char* FilePath, FUSIONCORE::Model& model);
	  FUSIONFRAME_EXPORT_FUNCTION bool ImportObj(const char* FilePath, FUSIONCORE::Model& model);

	  FUSIONFRAME_EXPORT_FUNCTION void SmoothObject(FUSIONCORE::Mesh& mesh);
	  FUSIONFRAME_EXPORT_FUNCTION void LoopSubdivision(FUSIONCORE::Model& Model, int level);
	  FUSIONFRAME_EXPORT_FUNCTION void CalculateTangentBitangent(std::vector<std::shared_ptr<FUSIONCORE::Vertex>>& vertices, std::vector<unsigned int>& indices);
	  FUSIONFRAME_EXPORT_FUNCTION void CollapseDecimation(FUSIONCORE::Mesh& Mesh, int level);

	  FUSIONFRAME_EXPORT_FUNCTION std::vector<glm::vec3> DistributePointsOnMeshSurface(FUSIONCORE::Mesh& Mesh, unsigned int PointCount , unsigned int seed);
	  FUSIONFRAME_EXPORT_FUNCTION std::vector<glm::vec3> DistributePointsOnMeshSurface(FUSIONCORE::Mesh& Mesh,FUSIONCORE::WorldTransform& Transformation, unsigned int PointCount, unsigned int seed);
	  
	  //Distribution is made using randomly chosen faces each time unlike standard version where faces are chosen in order. Might give better randomization for lesser point count. 
	  FUSIONFRAME_EXPORT_FUNCTION std::vector<glm::vec3> DistributePointsOnMeshSurfaceRandomized(FUSIONCORE::Mesh& Mesh, FUSIONCORE::WorldTransform& Transformation, unsigned int PointCount, unsigned int seed);

	  FUSIONFRAME_EXPORT_FUNCTION std::vector<FUSIONPHYSICS::CollisionBox> GridSubdivideCollisionBox(FUSIONPHYSICS::CollisionBox& collisionBox , unsigned int DivisionCountX , unsigned int DivisionCountY, unsigned int DivisionCountZ);
	  //Filling instance vbo with given data.
	  FUSIONFRAME_EXPORT_FUNCTION void FillInstanceDataVBO(FUSIONCORE::VBO& DestVBO ,std::vector<glm::vec3> &InstanceData);
	}
}