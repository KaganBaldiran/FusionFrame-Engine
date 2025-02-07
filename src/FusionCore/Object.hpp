#pragma once
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include <map>
#include "Transformation.hpp"
#include <unordered_set>
#include "../FusionUtility/FusionDLLExport.h"
#include <variant>

namespace FUSIONPHYSICS
{
    class QuadNode;
}

namespace FUSIONCORE
{
    //Forward declarations
    //{
        class Light;
        class Model;
    //}

    enum FF_OBJECT_TYPE
    {
        FF_OBJECT_TYPE_MODEL = 0x00313,
        FF_OBJECT_TYPE_LIGHT = 0x00314,
        FF_OBJECT_TYPE_CUSTOM = 0x00315
    };
     /*
     Represents a basic object in a scene.

     The Object class serves as a base class for objects within a scene, providing
     functionality for managing transformations, parent-child relationships, and associated
     quad nodes for collision detection. It allows adding and removing child objects,
     updating transformations, and retrieving information about children and associated quad nodes.

     Example usage:
     // Create a new object
     Object myObject;

     // Add a child object
     Object childObject;
     myObject.PushChild(&childObject);

     // Update object children transformations
     myObject.UpdateChildren();

     // Retrieve the transformation of the object
     WorldTransform transform = myObject.GetTransformation();
     */
    class FUSIONFRAME_EXPORT Object
    {
    protected:
        FF_OBJECT_TYPE ObjectType;
        size_t ObjectID;
        std::vector<Object*> Children;
        WorldTransform transformation;
        Object* Parent;
        std::vector<FUSIONPHYSICS::QuadNode*> AssociatedQuads;
    public:
        Object();
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
        inline WorldTransform& GetTransformation() { return this->transformation; };
        inline size_t GetObjectID() { return this->ObjectID; };

        template<typename T>
        T DynamicObjectCast() { return dynamic_cast<T>(this); };
       
        std::variant<std::monostate, FUSIONCORE::Model*, FUSIONCORE::Light*> ObjectTypeCast();
        const FF_OBJECT_TYPE& GetObjectType();

        //Checking if two objects memory addresses are the same(Doesn't check for object state) 
        inline bool IsSameObject(Object* other) {
            return this == other;
        }
    };

    void ClearObjectUpToDateBoundingBoxes();
    std::unordered_map<int, std::pair<glm::vec3, glm::vec3>>& GetCalculatedObjectBoundingBoxes();
    FUSIONFRAME_EXPORT_FUNCTION Object* GetObject(size_t ObjectID);
}