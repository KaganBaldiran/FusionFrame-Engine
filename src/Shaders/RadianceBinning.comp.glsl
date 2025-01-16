#version 460 core

layout(local_size_x=32,local_size_y=32,local_size_z=1) in;

layout(location=0) uniform vec2 BinSize;
layout(location=1) uniform samplerCube EnvironmentCubeMap;

#define BIN_COUNT 64 * 64 * 6
const float PI = 3.14159265359;

layout(std430,binding=12) restrict buffer RadianceBins
{
   float Radiances[BIN_COUNT];
   int Face[BIN_COUNT];
   vec4 Bins[BIN_COUNT];
};

vec3 GetDirection(in vec2 txc, in int face)
{
  vec3 v;
  switch(face)
  {
    case 0: v = vec3( 1.0, -txc.x, txc.y); break; 
    case 1: v = vec3(-1.0,  txc.x, txc.y); break; 
    case 2: v = vec3( txc.x,  1.0, txc.y); break; 
    case 3: v = vec3(-txc.x, -1.0, txc.y); break; 
    case 4: v = vec3(txc.x, -txc.y,  1.0); break; 
    case 5: v = vec3(txc.x,  txc.y, -1.0); break; 
  }
  return normalize(v);
}

float GetSolidAngle(in vec3 Direction)
{
   float theta = acos(Direction.y); 
   
   float DeltaAzimuth = (2.0f * PI) / BinSize.x;
   float DeltaPolar = PI / BinSize.y;

   float HighTheta = cos(clamp(theta + (0.5f * DeltaPolar),0.0f,PI));
   float LowTheta = cos(clamp(theta - (0.5f * DeltaPolar),0.0f,PI));

   return (LowTheta - HighTheta) * DeltaAzimuth;
}

void main()
{
    ivec3 pos = ivec3( gl_GlobalInvocationID.xyz );
    if(pos.x >= BinSize.x || pos.y >= BinSize.y) return;

    ivec2 CubeMapSize = ivec2(textureSize(EnvironmentCubeMap,0));

    ivec2 PixelCountPerBin = ivec2(CubeMapSize.xy / BinSize.xy);
    ivec2 BinStart = pos.xy * PixelCountPerBin;
    ivec2 BinEnd = (pos.xy + 1) * PixelCountPerBin;

    float Radiance = 0.0f;
    for (int y = BinStart.y; y < BinEnd.y; ++y) 
    {
        for (int x = BinStart.x; x < BinEnd.x; ++x) 
        {
            vec2 UV = vec2(x,y) / vec2(CubeMapSize.xy);
            vec3 Direction = GetDirection(UV,pos.z);
            vec3 texel = texture(EnvironmentCubeMap,Direction).xyz * GetSolidAngle(Direction);

            Radiance += clamp(dot(texel,vec3(0.2126, 0.7152, 0.0722)),0.0f,1.0f);
        }
    }
    //Radiance /= float(PixelCountPerBin.x * PixelCountPerBin.y);

    int Index = pos.z * int(BinSize.x) * int(BinSize.y) + pos.y * int(BinSize.x) + pos.x;
    Radiances[Index] = Radiance; 
    Bins[Index] = vec4(vec2(BinStart) / vec2(CubeMapSize.xy),vec2(BinEnd) / vec2(CubeMapSize.xy));
    Face[Index] = pos.z;
}