#pragma once
#include <glew.h>
#include <glfw3.h>
#include "Log.h"
#include "VectorMath.h"
#include "Buffer.h"
#include "Camera.h"
#include <map>
#include "Mesh.h"

namespace FUSIONOPENGL
{
    class Object
    {
    protected:
        std::vector<Object*> Children;
        WorldTransform transformation;
        Object* Parent;
    public:
        void PushChild(Object* child);
        void PopChild();
        void UpdateChildren();
        virtual void Update();
        Object* GetChild(int index);
        int GetChildrenCount();
        WorldTransform& GetTransformation() { return this->transformation; };
    };
}