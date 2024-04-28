#include "Object.hpp"
#include "../FusionPhysics/Octtree.hpp"
#include <glew.h>
#include <glfw3.h>
#include "Buffer.h"

void FUSIONCORE::Object::SetAssociatedQuads(const std::vector<FUSIONPHYSICS::QuadNode*>& CurrentQuads)
{
	this->AssociatedQuads.assign(CurrentQuads.begin(), CurrentQuads.end());
};

std::unordered_set<FUSIONCORE::Object*, PointerHash<FUSIONCORE::Object*>> FUSIONCORE::Object::GetUniqueQuadsObjects()
{
	std::unordered_set<Object*, PointerHash<Object*>> UniqueBoxes;
	unsigned int ReserveCount = 0;
	for (size_t i = 0; i < AssociatedQuads.size(); i++)
	{
		ReserveCount += AssociatedQuads[i]->Objects.size();
	}
	UniqueBoxes.reserve(ReserveCount);
	for (auto& Quad : AssociatedQuads)
	{
		for (size_t i = 0; i < Quad->Objects.size(); i++)
		{
			auto& object = Quad->Objects[i]->Object;
			if (object != nullptr)
			{
				if (!this->IsSameObject(object))
				{
					UniqueBoxes.insert(object);
				}
			}
		}
	}
	return UniqueBoxes;
}

void FUSIONCORE::Object::PushBackIntoAssociatedQuads(FUSIONPHYSICS::QuadNode* AssociatedQuad)
{
	this->AssociatedQuads.push_back(AssociatedQuad);
};

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
