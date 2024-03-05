#pragma once
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
	  void TestAssimp(const char* FilePathImport, const char* FilePathExport);
	  bool ExportObj(const char* FilePath, FUSIONCORE::Model& model);
	  bool ImportObj(const char* FilePath, FUSIONCORE::Model& model);
	  void SmoothObject(FUSIONCORE::Mesh& mesh);
	  void LoopSubdivision(FUSIONCORE::Mesh& Mesh, int level);
	  void CollapseDecimation(FUSIONCORE::Mesh& Mesh, int level);
	}
}