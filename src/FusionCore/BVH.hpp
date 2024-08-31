#pragma once
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "Model.hpp"

//BVH data structure on GPU for path tracing

namespace FUSIONCORE
{
	class BVH
	{
	public:
		BVH(std::vector<Model*> Models);
		~BVH();

	private:
		std::unique_ptr<FUSIONCORE::SSBO> CombinedObjectBuffer;
		std::unique_ptr<FUSIONCORE::SSBO> BVHbuffer;
	};

    void InitializeBHVbuffers();
    void ConstructBVH(std::vector<Model*> Models);
}