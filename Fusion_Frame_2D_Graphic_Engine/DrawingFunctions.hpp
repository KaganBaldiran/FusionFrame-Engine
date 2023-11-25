#pragma once
#include <glew.h>
#include "VectorMath.h"
#include "Log.h"
#include <glfw3.h>
#include "Buffer.h"
#include <vector>
#include "Shader.h"
#include "Camera.h"
#include <SDL.h>
#define PI 3.14159265359

namespace FusionDrawOpengl
{	
	class pixel
	{
	public:

		

		void operator()(const pixel& other)
		{
			this->Position.x = other.Position.x;
			this->Position.y = other.Position.y;

			this->Color.x = other.Color.x;
			this->Color.y = other.Color.y;
			this->Color.z = other.Color.z;
			this->Color.w = other.Color.w;
		}

		Vec2<float> Position;
		Vec4<float> Color;
	};

	void Initialize();
	void PutPixel(int x, int y, Vec4<float> Color);
	void PopPixel();
	void ClearPixelBuffer();
	void DrawPixel(GLuint shader, FUSIONOPENGL::Camera2D& camera, Vec2<int> WindowSize);
}


namespace FusionDrawSDL
{

	class pixelSDL
	{
	public:

		pixelSDL(Vec2<float> Position, Vec4<float> Color);
		pixelSDL();

		void operator()(const pixelSDL& other)
		{
			this->Position.x = other.Position.x;
			this->Position.y = other.Position.y;

			this->Color.x = other.Color.x;
			this->Color.y = other.Color.y;
			this->Color.z = other.Color.z;
			this->Color.w = other.Color.w;
		}

		void Draw(SDL_Renderer* renderer)
		{
			SDL_SetRenderDrawColor(renderer, Color.x, Color.y, Color.z, Color.w);
			SDL_RenderDrawPoint(renderer, Position.x, Position.y);
		}

		Vec2<float> Position;
		Vec4<float> Color;
	};
    
	void InsertPixel(int x , int y , Vec4<float> Color);
	void PopPixel(int x, int y);
	pixelSDL* ReadPixel(int x, int y);
	void DrawPixelBuffer(SDL_Renderer* renderer);
	void ClearPixelBuffer();

	void DrawCircle(int centerPosx, int centerPosy, int R , Vec4<float> Color);
	void DrawLine(Vec2<int> point0, Vec2<int> point1, Vec4<float> Color);
	void DrawPolygon(std::vector<Vec2<int>> vertices, std::vector<unsigned int> indices, Vec4<float> Color);
	void FloodFill();
	

}
