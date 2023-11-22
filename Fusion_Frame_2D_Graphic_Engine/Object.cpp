#include "Object.hpp"

void FUSIONOPENGL::Object::PushChild(Object* child)
{
	this->Children.push_back(child);
}

void FUSIONOPENGL::Object::PopChild()
{
	this->Children.pop_back();
}

void FUSIONOPENGL::Object::UpdateChildren()
{
	auto& lastScales = this->transformation.LastScales;
	auto& lastRotations = this->transformation.LastRotations;
	auto& lastTransforms = this->transformation.LastTransforms;

	for (auto& child : this->Children)
	{
		for (size_t i = 0; i < lastScales.size(); i++)
		{
			child->transformation.Scale(lastScales[i].Scale);
		}
		for (size_t i = 0; i < lastRotations.size(); i++)
		{
			child->transformation.Translate(this->transformation.Position);
			child->transformation.Rotate(lastRotations[i].Vector, lastRotations[i].Degree);
			child->transformation.Translate(-this->transformation.Position);
		}
		for (size_t i = 0; i < lastTransforms.size(); i++)
		{
			child->transformation.Translate(lastTransforms[i].Transformation);
		}
		child->UpdateChildren();
	}
	lastScales.clear();
	lastRotations.clear();
	lastTransforms.clear();
}

FUSIONOPENGL::Object* FUSIONOPENGL::Object::GetChild(int index)
{
	return Children.at(index);
}

int FUSIONOPENGL::Object::GetChildrenCount()
{
	return Children.size();
}
