 #version 420 core

 layout(location = 0) in vec3 vertexdata;
 layout(location = 1) in vec3 aNormal;
 layout(location = 2) in vec2 textcoord;
 layout(location = 3) in vec3 tangentnormal;
 layout(location = 4) in vec3 bitangentnormal;
 layout(location = 5) in ivec4 boneIds; 
 layout(location = 6) in vec4 weights;

 uniform mat4 model;
 uniform mat4 proj;
 uniform mat4 view;

 const int MAX_BONES = 100;
 const int MAX_BONE_INFLUENCE = 4;

 layout (std140 , binding = 1) uniform AnimationMatrices
 {
    mat4 finalBonesMatrices[MAX_BONES];
 };

 uniform bool EnableAnimation;

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

      gl_Position = model * totalPosition;
   }
   else
   {
      gl_Position = model * vec4(vertexdata,1.0f);
   }
 }
