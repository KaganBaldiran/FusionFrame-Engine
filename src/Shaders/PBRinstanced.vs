 #version 330 core

 layout(location = 0) in vec3 vertexdata;
 layout(location = 1) in vec3 aNormal;
 layout(location = 2) in vec2 textcoord;
 layout(location = 3) in vec3 tangentnormal;
 layout(location = 4) in vec3 bitangentnormal;
 layout(location = 7) in vec3 InstanceOffset;

 out vec3 Normal;
 out vec2 FinalTexCoord;
 out mat3 TBN;
 out vec3 CurrentPos;

 uniform mat4 model;
 uniform mat4 ProjView;

 void main()
 { 
     CurrentPos = vec3(model * vec4(vertexdata,1.0f)) + InstanceOffset;
     gl_Position = ProjView * vec4(CurrentPos ,1.0f);

     mat3 NormalMatrix = transpose(inverse(mat3(model)));
     vec3 T = normalize(vec3(NormalMatrix* tangentnormal));
     vec3 B = normalize(vec3(NormalMatrix* bitangentnormal));
     vec3 N = normalize(vec3(NormalMatrix* aNormal));
     TBN = mat3(T,B,N);

     FinalTexCoord = textcoord;
     Normal = N;
 }
