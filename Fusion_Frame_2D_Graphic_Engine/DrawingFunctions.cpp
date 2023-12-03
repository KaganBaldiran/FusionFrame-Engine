#include "DrawingFunctions.hpp"
#include <memory>
#include <map>
#include <string>
#include <math.h>

std::vector<FusionDrawOpengl::pixel> pixels;

std::map<std::string,FusionDrawSDL::pixelSDL> pixelsSDL;

std::unique_ptr<Buffer> pixelBuffer;

void FusionDrawOpengl::Initialize()
{
	glPointSize(1.0f);
	pixelBuffer = std::make_unique<Buffer>();
	pixelBuffer->Bind();
	//float pixel[] = 
	//{
		//0.5f,0.5f,1.0f 
	//};
	//pixelBuffer->BufferDataFill(GL_ARRAY_BUFFER, sizeof(pixel), pixel, GL_STATIC_DRAW);
	//pixelBuffer->AttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	pixelBuffer->Unbind();
}
void FusionDrawOpengl::PutPixel(int x, int y, Vec4<float> Color)
{
	pixel pixeltemp({ Vec2<float>(x,y), Color });
	pixels.push_back(pixeltemp);
}

void FusionDrawOpengl::PopPixel()
{
	pixels.pop_back();
}

void FusionDrawOpengl::ClearPixelBuffer()
{
	pixels.clear();
}

void FusionDrawOpengl::DrawPixel(GLuint shader, FUSIONOPENGL::Camera2D& camera , Vec2<int> WindowSize)
{
	pixelBuffer->Bind();
	UseShaderProgram(shader);

	pixelBuffer->BufferDataFill(GL_ARRAY_BUFFER, sizeof(pixel) * pixels.size(), pixels.data(), GL_STATIC_DRAW);
	pixelBuffer->AttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	pixelBuffer->AttribPointer(0, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)2);

	camera.SetCameraMatrixUniformLocation(shader, "CamMat");
	glUniform2i(glGetUniformLocation(shader, "ScreenSize"), WindowSize.x, WindowSize.y);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	pixelBuffer->Unbind();
	UseShaderProgram(0);
};





FusionDrawSDL::pixelSDL::pixelSDL(Vec2<float> Position, Vec4<float> Color)
{
	this->Color = Color;
	this->Position = Position;
}

FusionDrawSDL::pixelSDL::pixelSDL()
{
}

void FusionDrawSDL::InsertPixel(int x, int y, Vec4<float> Color)
{
	FusionDrawSDL::pixelSDL temp;
	temp.Color = Color;
	temp.Position = { (float)x, (float)y };
	std::string key = std::to_string(x) + std::to_string(y);
	pixelsSDL.emplace(std::make_pair(key, temp));
	
}

void FusionDrawSDL::PopPixel(int x, int y)
{
	pixelsSDL.erase(std::to_string(x) + std::to_string(y));
}

FusionDrawSDL::pixelSDL* FusionDrawSDL::ReadPixel(int x, int y)
{
	return &pixelsSDL[std::to_string(x) + std::to_string(y)];
}

void FusionDrawSDL::DrawPixelBuffer(SDL_Renderer* renderer)
{
	Vec4<float> ColorTemp;
	Vec2<float> PosTemp;

	for (const auto& pixel : pixelsSDL)
	{
		ColorTemp = pixel.second.Color;
		PosTemp = pixel.second.Position;
		SDL_SetRenderDrawColor(renderer, ColorTemp.x, ColorTemp.y, ColorTemp.z, ColorTemp.w);
		SDL_RenderDrawPoint(renderer, PosTemp.x, PosTemp.y);
	}
}

void FusionDrawSDL::ClearPixelBuffer()
{
	pixelsSDL.clear();
}

void FusionDrawSDL::DrawCircle(int centerPosx, int centerPosy, int R , Vec4<float> Color)
{
	int x = R;
	int y = 0;
	int radiusError = 1 - x;

	while (x >= y)
	{
		FusionDrawSDL::InsertPixel(centerPosx + x, centerPosy - y, Color);
		FusionDrawSDL::InsertPixel(centerPosx - x, centerPosy - y, Color);
		FusionDrawSDL::InsertPixel(centerPosx + x, centerPosy + y, Color);
		FusionDrawSDL::InsertPixel(centerPosx - x, centerPosy + y, Color);
		FusionDrawSDL::InsertPixel(centerPosx + y, centerPosy - x, Color);
		FusionDrawSDL::InsertPixel(centerPosx - y, centerPosy - x, Color);
		FusionDrawSDL::InsertPixel(centerPosx + y, centerPosy + x, Color);
		FusionDrawSDL::InsertPixel(centerPosx - y, centerPosy + x, Color);

		y++;

		if (radiusError < 0)
		{
			radiusError += 2 * y + 1;
		}
		else
		{
			x--;
			radiusError += 2 * (y - x) + 1;
		}
	}
}

void FusionDrawSDL::DrawLine(Vec2<int> point0, Vec2<int> point1, Vec4<float> Color)
{
    Vec2<int> difference = point1 - point0;

    if (difference.y == 0)
    {
        int LeftMostVertexX = std::min(point0.x, point1.x);
        for (size_t x = 0; x < std::abs(difference.x); x++)
        {
            FusionDrawSDL::InsertPixel(LeftMostVertexX + x, point0.y, Color);
        }
    }
    else if (difference.x == 0)
    {
        int LeftMostVertexY = std::min(point0.y, point1.y);
        for (size_t y = 0; y < std::abs(difference.y); y++)
        {
            FusionDrawSDL::InsertPixel(point0.x, LeftMostVertexY + y, Color);
        }
    }
    else
    {
        float slope = static_cast<float>(difference.y) / static_cast<float>(difference.x);

        int startX = std::min(point0.x, point1.x);
        int endX = std::max(point0.x, point1.x);

		int startY;
		if (startX == point0.x)
		{
		   startY = point0.y;  
		}
		else
		{
			startY = point1.y;
		}

        for (int x = startX; x <= endX; x++)
        {
            int y = static_cast<int>(startY + slope * (x - startX));
            FusionDrawSDL::InsertPixel(x, y, Color);
        }
    }
}


void FusionDrawSDL::DrawPolygon(std::vector<Vec2<int>> vertices, std::vector<unsigned int> indices, Vec4<float> Color)
{
	if (vertices.size() >= 2)
	{
		for (size_t i = 0; i < vertices.size() - 1; i++)
		{
			Vec2<int> vertex0 = vertices.at(i);
			Vec2<int> vertex1 = vertices.at(i + 1);

			DrawLine(vertex0, vertex1, Color);
		}
	}
}

void FusionDrawSDL::FloodFill()
{


}

