#version 460 core

layout(local_size_x=256) in;

#define BIN_COUNT 64 * 64 * 6
layout(std430,binding=12) restrict buffer RadianceBins
{
   float Radiances[BIN_COUNT];
   int Face[BIN_COUNT];
   vec4 Bins[BIN_COUNT];
};

layout(std430,binding=13) restrict buffer RadianceSumBuffer
{
   float OutputRadiances[BIN_COUNT];
};

layout(std430,binding=14) restrict buffer SummedWorkgroupBuffer
{
   float WorkGroupSums[];
};

#define N 512

shared float SharedData[N];

void main()
{
   uint GlobalIndex = uint(gl_GlobalInvocationID.x);
   uint LocalIndex = uint(gl_LocalInvocationID.x);
   uint WorkGroupIndex = uint(gl_WorkGroupID.x);
   uint LocalSize = uint(gl_WorkGroupSize.x);
   uint GlobalSize = uint(gl_NumWorkGroups.x);

   if(GlobalIndex >= BIN_COUNT) return;

   SharedData[2 * LocalIndex] = Radiances[2 * GlobalIndex];
   SharedData[2 * LocalIndex + 1] = Radiances[2 * GlobalIndex + 1];
   barrier();

   int Offset = 1;
   for(uint d = N>>1; d > 0; d >>= 1)
   {
      if(LocalIndex < d)
      {
         uint Index0 = Offset*(2*LocalIndex+1)-1;     
         uint Index1 = Offset*(2*LocalIndex+2)-1;
         SharedData[Index1] += SharedData[Index0]; 
      }
      Offset *= 2;
      barrier();
   }

   if(LocalIndex == 0)
   {
      WorkGroupSums[2 * WorkGroupIndex] = SharedData[(N>>1)- 1];
      WorkGroupSums[2 * WorkGroupIndex + 1] = SharedData[N - 1];
      SharedData[N - 1] = 0.0f;
   }
   barrier();

   for(uint d = 1; d < N; d *= 2)
   {
      Offset >>= 1;
      if(LocalIndex < d)
      {
         uint Index0 = Offset*(2*LocalIndex+1)-1;     
         uint Index1 = Offset*(2*LocalIndex+2)-1;
         float Temp = SharedData[Index0]; 
         SharedData[Index0] = SharedData[Index1]; 
         SharedData[Index1] += Temp;
      }
      barrier();
   }

   if(LocalIndex < LocalSize)
   {
      OutputRadiances[2 * GlobalIndex] = SharedData[2 * LocalIndex];
      OutputRadiances[2 * GlobalIndex + 1] = SharedData[2 * LocalIndex + 1];
   }
}