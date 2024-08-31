 #version 420 core

 layout(location = 0) in vec3 vertexdata;
 layout(location = 1) in vec3 aNormal;
 layout(location = 2) in vec2 textcoord;
 layout(location = 3) in vec3 tangentnormal;
 layout(location = 4) in vec3 bitangentnormal;
 layout(location = 5) in ivec4 boneIds; 
 layout(location = 6) in vec4 weights;

 out vec3 Normal;
 out vec2 FinalTexCoord;
 out mat3 TBN;
 out vec3 CurrentPos;

 uniform mat4 model;
 uniform mat4 ProjView;
 uniform mat4 ViewMat;

 const int MAX_BONES = 100;
 const int MAX_BONE_INFLUENCE = 4;

 layout(std140 , binding = 1) uniform AnimationMatrices
 {
    mat4 finalBonesMatrices[MAX_BONES];
 };

 uniform bool EnableAnimation;

 void main()
 { 
   mat3 NormalMatrix = transpose(inverse(mat3(model)));
   FinalTexCoord = textcoord;

   vec3 ResultNormal = aNormal;
   vec3 ResultTangent = tangentnormal;
   vec3 ResultBiTangent = bitangentnormal;
   CurrentPos = vertexdata;

   if(EnableAnimation)
   {
    vec4 totalPosition = vec4(0.0f);
    vec3 localNormal = aNormal;
    vec3 totalNormal = vec3(0.0f);
    vec3 totalTangent = vec3(0.0f);
    vec3 totalBitangent = vec3(0.0f);
    vec3 localTangent = tangentnormal;
    vec3 localBitangent = bitangentnormal;
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneIds[i] == -1) 
            continue;
        if(boneIds[i] >=MAX_BONES) 
        {
            totalPosition = vec4(vertexdata,1.0f);
            break;
        }
        mat4 BoneMat = finalBonesMatrices[boneIds[i]];
        mat3 BoneNormalMat = transpose(inverse(mat3(BoneMat)));
        float weight = weights[i];
      
        vec4 localPosition = BoneMat * vec4(vertexdata,1.0f);
        totalPosition += localPosition * weight;

        localNormal = BoneNormalMat * localNormal;
        totalNormal += localNormal * weight;

        localTangent = BoneNormalMat * localTangent;
        totalTangent += localTangent * weight;

        localBitangent = BoneNormalMat * localBitangent;
        totalBitangent += localBitangent * weight;
   }
   
     CurrentPos = totalPosition.xyz;

     ResultNormal = totalNormal;
     ResultTangent = totalTangent;
     ResultBiTangent = totalBitangent;
   }
   
   CurrentPos = vec3(model * vec4(CurrentPos,1.0f));
   gl_Position = ProjView * vec4(CurrentPos,1.0f);

   vec3 T = normalize(vec3(NormalMatrix* ResultTangent));
   vec3 B = normalize(vec3(NormalMatrix* ResultBiTangent));
   //vec3 N = normalize(vec3(NormalMatrix * ResultNormal));
   vec3 N = normalize(NormalMatrix * vec3(ResultNormal));
   TBN = mat3(T,B,N);

   Normal = N;
 }
