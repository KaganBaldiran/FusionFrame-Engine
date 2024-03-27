#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "Buffer.h"
#include <map>
#include "Transformation.hpp"
#include <unordered_set>

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
        //Mostly internal use
        inline void SetAssociatedQuads(const std::vector<FUSIONPHYSICS::QuadNode*> &CurrentQuads) 
        { 
            this->AssociatedQuads.assign(CurrentQuads.begin(), CurrentQuads.end());
        };

        //Returns the objects in the common quad node in a unique manner.
        std::unordered_set<Object*, PointerHash<Object*>> GetUniqueQuadsObjects();
        //Mostly internal use
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

        template<typename T>
        T DynamicObjectCast() { return dynamic_cast<T>(this); };

        //Checking if two objects memory addresses are the same(Doesn't check for object state) 
        inline bool IsSameObject(Object* other) {
            return this == other;
        }
    };
}