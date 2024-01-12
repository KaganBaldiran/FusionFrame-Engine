#pragma once
#include <iostream>
#include "../FusionUtility/VectorMath.h"

namespace SDL_CAMERA2D
{
	class SDLCamera2D
	{
	public:

		void UpdateCamera(Vec2<float> &target , Vec2<int> &Windowsize , float zoom)
		{
			this->Target = Target + target;
			CameraPosition.SetValues(Target.x - (Windowsize.x / 2), (Target.x + (Windowsize.x / 2)),
				                     Target.y - (Windowsize.y / 2), Target.y + (Windowsize.y / 2));
		};


		Vec4<float> GetCameraFrustum() { return this->CameraPosition; };

	private:
		Vec2<float> Target;
		Vec4<float> CameraPosition;
		float zoom;
	};
}



