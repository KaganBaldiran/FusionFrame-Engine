#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_timer.h>
#include <iostream>
#include "VectorMath.h"

#define SDL_SUCCESS 1
#define SDL_ERROR -1

namespace SDL_CUSTOM
{
	std::pair<int,SDL_Window*> init(const char* windowName, SDL_Renderer** Renderer, float width, float height, bool fullscreen, SDL_DisplayMode &dpm);
	int HandleEvent(bool& isRunning , SDL_Event& event);
	SDL_Texture* LoadInTexture(const char* filesrc,Vec2<int> &TextureSize, SDL_Renderer* renderer);

	class Object
	{
	public:
		Object();
		void TranslateTo(int x, int y);
		void Rotate(float angles);
		void Scale();
		virtual void Draw();
	
		Vec3<float> Position;
		SDL_Rect DestRec;
		SDL_Rect SourceRec;

	private:

		SDL_Texture* texture;
		Vec2<int> Size;

	};

}

