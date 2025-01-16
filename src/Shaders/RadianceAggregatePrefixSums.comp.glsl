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

void main()
{
   uint GlobalIndex = uint(gl_GlobalInvocationID.x);
   uint LocalIndex = uint(gl_LocalInvocationID.x);
   uint WorkGroupIndex = uint(gl_WorkGroupID.x);
   uint LocalSize = uint(gl_WorkGroupSize.x);
   uint GlobalSize = uint(gl_NumWorkGroups.x);

   if(GlobalIndex >= (BIN_COUNT - 1)) return;

   float WorkGroupValue = WorkGroupSums[WorkGroupIndex];
   float MaxValue = OutputRadiances[BIN_COUNT - 1] + WorkGroupSums[(BIN_COUNT / 256) - 1];

   float Value0 = (OutputRadiances[GlobalIndex] + WorkGroupValue);
   float Value1 = (OutputRadiances[GlobalIndex + 1] + WorkGroupValue);

   Radiances[GlobalIndex] = float(Value0 / MaxValue);
   Radiances[GlobalIndex + 1] = float(Value1 / MaxValue);
}