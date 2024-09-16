#version 460 core

layout(local_size_x=32) in;

layout(std430,binding=7) restrict buffer TriangleData
{
  vec4 TrianglePositions[]; 
};

layout(location=0) uniform int TriangleCount;

void main()
{
    ivec2 pos = ivec2( gl_GlobalInvocationID.xy );
    /*
    if(pos.x < TriangleCount)
    {
      vec4 TrianglePosition = TrianglePositions[pos.x];
      TrianglePositions[pos.x] *= ModelMatrix[int(TrianglePosition.w)];
    }
    */
}