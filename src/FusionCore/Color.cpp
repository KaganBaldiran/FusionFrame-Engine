#include "Color.hpp"

FUSIONCORE::Color::Color(glm::vec4 Value)
{
	this->Value = Value;
}

void FUSIONCORE::Color::SetRGBA(glm::vec4 Value)
{
	this->Value = Value;
}

void FUSIONCORE::Color::SetRGB(glm::vec3 Value)
{
	this->Value.x = Value.x;
	this->Value.y = Value.y;
	this->Value.z = Value.z;
}

void FUSIONCORE::Color::SetRed(float Value)
{
	this->Value.x = Value;
}

void FUSIONCORE::Color::SetBlue(float Value)
{
	this->Value.z = Value;
}

void FUSIONCORE::Color::SetGreen(float Value)
{
	this->Value.y = Value;
}

void FUSIONCORE::Color::SetAlpha(float Value)
{
	this->Value.w = Value;
}

void FUSIONCORE::Color::Brighter(float IncrementBy)
{
	this->Value.x += IncrementBy;
	this->Value.y += IncrementBy;
	this->Value.z += IncrementBy;
}

void FUSIONCORE::Color::Darker(float DecrementBy)
{
	this->Value.x -= DecrementBy;
	this->Value.y -= DecrementBy;
	this->Value.z -= DecrementBy;
}

glm::vec3 FUSIONCORE::Color::GetRGB()
{
	return { this->Value.x , this->Value.y , this->Value.z };
}

float FUSIONCORE::Color::GetRed()
{
	return this->Value.x;
}

float FUSIONCORE::Color::GetBlue()
{
	return this->Value.z;
}

float FUSIONCORE::Color::GetGreen()
{
	return this->Value.y;
}

float FUSIONCORE::Color::GetAlpha()
{
	return this->Value.w;
}

glm::vec4 FUSIONCORE::Color::GetRBGA()
{
	return { this->Value.x , this->Value.z , this->Value.y , this->Value.w };
}

glm::vec4 FUSIONCORE::Color::GetRGBA()
{
	return { this->Value.x , this->Value.y , this->Value.z , this->Value.w };
}
