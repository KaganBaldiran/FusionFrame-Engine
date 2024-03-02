#include "Object.hpp"

void FUSIONCORE::Object::PushChild(Object* child)
{
	child->Parent = this;
	this->Children.push_back(child);
}

void FUSIONCORE::Object::PopChild()
{
	this->Children.pop_back();
}

void FUSIONCORE::Object::UpdateChildren()
{
	if (!Children.empty())
	{
		auto& lastScales = this->transformation.LastScales;
		auto& lastRotations = this->transformation.LastRotations;
		auto& lastTransforms = this->transformation.LastTransforms;

		for (auto& child : this->Children)
		{
			child->Update();
		}

		lastScales.clear();
		lastRotations.clear();
		lastTransforms.clear();
	}
}

void FUSIONCORE::Object::Update()
{
	auto& lastScales = this->Parent->transformation.LastScales;
	auto& lastRotations = this->Parent->transformation.LastRotations;
	auto& lastTransforms = this->Parent->transformation.LastTransforms;

	for (size_t i = 0; i < lastScales.size(); i++)
	{
		this->transformation.Scale(lastScales[i].Scale);
	}
	for (size_t i = 0; i < lastRotations.size(); i++)
	{
		this->transformation.Translate(FF_ORIGIN);
		this->transformation.Rotate(lastRotations[i].Vector, lastRotations[i].Degree);
		this->transformation.Translate(-FF_ORIGIN);
	}
	for (size_t i = 0; i < lastTransforms.size(); i++)
	{
		this->transformation.Translate(lastTransforms[i].Transformation);
	}

	UpdateChildren();
}

FUSIONCORE::Object* FUSIONCORE::Object::GetChild(int index)
{
	return Children.at(index);
}

int FUSIONCORE::Object::GetChildrenCount()
{
	return Children.size();
}
