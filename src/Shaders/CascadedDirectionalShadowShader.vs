#version 460 core

 layout(location = 0) in vec3 vertexdata;
 layout(location = 1) in vec3 aNormal;
 layout(location = 2) in vec2 textcoord;
 layout(location = 3) in vec3 tangentnormal;
 layout(location = 4) in vec3 bitangentnormal;
 layout(location = 5) in ivec4 boneIds; 
 layout(location = 6) in vec4 weights;
 layout(location = 7) in vec3 InstanceOffset;

 out vec3 Normal;
 out vec2 FinalTexCoord;
 out mat3 TBN;
 out vec3 CurrentPos;

 uniform mat4 model;
 uniform mat4 proj;
 uniform mat4 view;
 uniform mat4 ratioMat;

 const int MAX_BONES = 100;
 const int MAX_BONE_INFLUENCE = 4;
 layout(std140 , binding = 1) uniform AnimationMatrices
 {
    mat4 finalBonesMatrices[MAX_BONES];
 };
 uniform mat4 cameramatrix;
 uniform bool EnableAnimation;
 uniform bool EnableInstancing;

 void main()
 { 
   if(EnableAnimation)
   {
    vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneIds[i] == -1) 
            continue;
        if(boneIds[i] >=MAX_BONES) 
        {
            totalPosition = vec4(vertexdata,1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(vertexdata,1.0f);
        totalPosition += localPosition * weights[i];
   }
   
     CurrentPos = vec3(model * totalPosition);     
   }
   else if(EnableInstancing)
   {
     CurrentPos = vec3(model * vec4(vertexdata,1.0f)) + InstanceOffset;     
   }
   else
   {
     CurrentPos = vec3(model * vec4(vertexdata,1.0f));
   }
   gl_Position = vec4(CurrentPos,1.0f);
   FinalTexCoord = textcoord;
 }