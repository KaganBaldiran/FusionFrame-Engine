#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "Buffer.h"
#include <map>
#include "Transformation.hpp"

namespace FUSIONPHYSICS
{
    class QuadNode;
}

namespace FUSIONCORE
{
    class Object
    {
    protected:
        std::vector<Object*> Children;
        WorldTransform transformation;
        Object* Parent;
        std::vector<FUSIONPHYSICS::QuadNode*> AssociatedQuads;
    public:
        inline void SetAssociatedQuads(const std::vector<FUSIONPHYSICS::QuadNode*> &CurrentQuads) 
        { 
            this->AssociatedQuads.assign(CurrentQuads.begin(), CurrentQuads.end());
        };
        inline void PushBackIntoAssociatedQuads(FUSIONPHYSICS::QuadNode* AssociatedQuad)
        {
            this->AssociatedQuads.push_back(AssociatedQuad);
        };
        inline const std::vector<FUSIONPHYSICS::QuadNode*>& GetAssociatedQuads() { return this->AssociatedQuads; };
        void PushChild(Object* child);
        void PopChild();
        void UpdateChildren();
        virtual void Update();
        Object* GetChild(int index);
        int GetChildrenCount();
        WorldTransform& GetTransformation() { return this->transformation; };

        inline bool IsSameObject(Object* other) {
            return this == other;
        }
    };
}