 #version 330 core

 layout(location = 0) in vec3 vertexdata;
 layout(location = 1) in vec3 aNormal;
 layout(location = 2) in vec2 textcoord;
 layout(location = 3) in vec3 tangentnormal;
 layout(location = 4) in vec3 bitangentnormal;
 layout(location = 7) in vec3 inputcolors;

 out vec3 Normal;
 out vec2 FinalTexCoord;
 out mat3 TBN;
 out vec3 CurrentPos;

 uniform mat4 model;
 uniform mat4 proj;
 uniform mat4 view;
 uniform mat4 ratioMat;

 uniform mat4 cameramatrix;

 void main()
 {
   CurrentPos = vec3(model * vec4(vertexdata,1.0f));
   gl_Position = proj * view * vec4(CurrentPos,1.0f);

   vec3 T = normalize(vec3(model* vec4(tangentnormal,0.0f)));
   vec3 B = normalize(vec3(model* vec4(bitangentnormal,0.0f)));
   vec3 N = normalize(vec3(model* vec4(aNormal,0.0f)));
   TBN = mat3(T,B,N);

   FinalTexCoord = textcoord;
   Normal = vec3(model * vec4(aNormal,0.0));
 }
