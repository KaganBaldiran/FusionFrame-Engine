#include "Color.hpp"

FUSIONOPENGL::Color::Color(glm::vec4 Value)
{
	this->Value = Value;
}

void FUSIONOPENGL::Color::SetRGBA(glm::vec4 Value)
{
	this->Value = Value;
}

void FUSIONOPENGL::Color::SetRGB(glm::vec3 Value)
{
	this->Value.x = Value.x;
	this->Value.y = Value.y;
	this->Value.z = Value.z;
}

void FUSIONOPENGL::Color::SetRed(float Value)
{
	this->Value.x = Value;
}

void FUSIONOPENGL::Color::SetBlue(float Value)
{
	this->Value.z = Value;
}

void FUSIONOPENGL::Color::SetGreen(float Value)
{
	this->Value.y = Value;
}

void FUSIONOPENGL::Color::SetAlpha(float Value)
{
	this->Value.w = Value;
}

void FUSIONOPENGL::Color::Brighter(float IncrementBy)
{
	this->Value.x += IncrementBy;
	this->Value.y += IncrementBy;
	this->Value.z += IncrementBy;
}

void FUSIONOPENGL::Color::Darker(float DecrementBy)
{
	this->Value.x -= DecrementBy;
	this->Value.y -= DecrementBy;
	this->Value.z -= DecrementBy;
}

glm::vec3 FUSIONOPENGL::Color::GetRGB()
{
	return { this->Value.x , this->Value.y , this->Value.z };
}

float FUSIONOPENGL::Color::GetRed()
{
	return this->Value.x;
}

float FUSIONOPENGL::Color::GetBlue()
{
	return this->Value.z;
}

float FUSIONOPENGL::Color::GetGreen()
{
	return this->Value.y;
}

float FUSIONOPENGL::Color::GetAlpha()
{
	return this->Value.w;
}

glm::vec4 FUSIONOPENGL::Color::GetRBGA()
{
	return { this->Value.x , this->Value.z , this->Value.y , this->Value.w };
}

glm::vec4 FUSIONOPENGL::Color::GetRGBA()
{
	return { this->Value.x , this->Value.y , this->Value.z , this->Value.w };
}
