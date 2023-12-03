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
		void Translate(int x, int y);
		void Rotate(float angles);
		void Scale(float scaleX, float scaleY);
		virtual void Draw(SDL_Renderer* renderer);
	
		Vec3<float> Position;
		float angles = 0.0f;
		SDL_Rect DestRec;
		SDL_Rect SourceRec;

		Vec4<float> Color;

	protected:

		Vec2<int> Size;
	};


	class TextureObject : public Object
	{
	public:
		TextureObject(SDL_Texture* texture, Vec2<int> TextureSize);
		void Draw(SDL_Renderer* renderer);
		void Draw(std::vector<Vec2<int>> AtlasIndicies, int XincrementSize, int YincrementSize, int SubtextureCount, SDL_Renderer* renderer, int FrameDelay, SDL_RendererFlip flip, bool PlayAnimation = true);
	private:
		SDL_Texture* texture;
	};

}

