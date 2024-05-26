#pragma once
#include "../FusionUtility/FusionGL.hpp"
#include "../FusionUtility/FusionDLLExport.h"
#include "Framebuffer.hpp"

namespace FUSIONCORE
{
	FUSIONFRAME_EXPORT_FUNCTION void InitializeDecalUnitBox();

	class DecalDeferred : public Object
	{
	public:
		//Only effects the objects drawn before it since it uses the position buffer of the given geometry buffer.
		void Draw(Gbuffer& GeometryBuffer, Material Material, Camera3D& camera, FUSIONUTIL::DefaultShaders& shaders);
	private:

	};

}

