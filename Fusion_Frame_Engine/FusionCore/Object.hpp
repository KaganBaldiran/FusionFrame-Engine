#pragma once
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include <map>
#include "Transformation.hpp"
#include <unordered_set>
#include "../FusionUtility/FusionDLLExport.h"

namespace FUSIONPHYSICS
{
    class QuadNode;
}

namespace FUSIONCORE
{
    class FUSIONFRAME_EXPORT Object
    {
    protected:
        std::vector<Object*> Children;
        WorldTransform transformation;
        Object* Parent;
        std::vector<FUSIONPHYSICS::QuadNode*> AssociatedQuads;
    public:
        //Mostly internal use
        void SetAssociatedQuads(const std::vector<FUSIONPHYSICS::QuadNode*>& CurrentQuads);
        
        //Returns the objects in the common quad node in a unique manner.
        std::unordered_set<Object*, PointerHash<Object*>> GetUniqueQuadsObjects();
        //Mostly internal use
        void PushBackIntoAssociatedQuads(FUSIONPHYSICS::QuadNode* AssociatedQuad);
        
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