#include "SDL_CUSTOM.hpp"
#include "Log.h"

std::pair<int, SDL_Window*> SDL_CUSTOM::init(const char* windowName , SDL_Renderer** Renderer, float width , float height, bool fullscreen, SDL_DisplayMode &dpm)
{
	SDL_Window* window = NULL;

	Uint32 flags = 0;
	if (fullscreen)
	{
		flags = SDL_WINDOW_FULLSCREEN;
	}

	Uint32 renderflags = SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED;

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		LOG_ERR("SDL could not initialize :: " << SDL_GetError());
		SDL_Quit();
		return { SDL_ERROR,nullptr };
	}
	
	LOG_INF("Subsystems were initialized!");

	if (SDL_GetDesktopDisplayMode(0, &dpm) != 0)
	{
		LOG_ERR("SDL could not initialize :: " << SDL_GetError());
		SDL_Quit();
		return { SDL_ERROR,nullptr };
	}

	window = SDL_CreateWindow(windowName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
	if (window == NULL)
	{
		LOG_ERR("Error feeding the window with screen size :: " << SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		return { SDL_ERROR,nullptr };
	}
	
	*Renderer = SDL_CreateRenderer(window, -1, renderflags);
	if (*Renderer == NULL)
	{
		LOG_ERR("Error initializing the renderer :: " << SDL_GetError());
		SDL_DestroyRenderer(*Renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}
	
	return { SDL_SUCCESS,window };
}

int SDL_CUSTOM::HandleEvent(bool &isRunning, SDL_Event& event)
{
	SDL_PollEvent(&event);

	if (event.type == SDL_QUIT)
	{
		isRunning = false;
		return SDL_QUIT;
	}

	return SDL_SUCCESS;
}

SDL_Texture* SDL_CUSTOM::LoadInTexture(const char* filesrc, Vec2<int>& TextureSize, SDL_Renderer* renderer)
{
	SDL_Surface *tempSurface = IMG_Load(filesrc);
	TextureSize({ tempSurface->w,tempSurface->h });
	SDL_Texture* Texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
	SDL_FreeSurface(tempSurface);
	
	return Texture;
}

SDL_CUSTOM::Object::Object()
{

}

void SDL_CUSTOM::Object::TranslateTo(int x ,int y)
{
	DestRec.x = x;
	DestRec.y = y;
}

void SDL_CUSTOM::Object::Rotate(float angles)
{
	float radians = angles * (std::_Pi_val / 180.0f);
	//glm::rotate()

	//DestRec.x = std::cos(radians) * DestRec.x - std::sin(radians) * DestRec.y;
	//DestRec.y = std::sin(radians) * DestRec.x + std::cos(radians) * DestRec.y;

	//w = 2 * max(abs(x1', x2'))
	//h = 2 * max(abs(y1', y2'))
}

void SDL_CUSTOM::Object::Scale()
{
}

void SDL_CUSTOM::Object::Draw()
{
}
