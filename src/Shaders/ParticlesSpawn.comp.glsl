#version 460 core

struct Particle
{
  vec3 position;
  vec3 velocity;
  vec3 acceleration;
  vec4 color;
  float life;
};

layout(std430 , binding=1) restrict buffer ParticleBuffer0
{
    Particle particles[];
};

layout(std430 , binding=2) coherent restrict buffer ParticleBuffer1
{
    int count;
    int indices[];
};

layout(location = 0) uniform int ParticleSpawnCount;
layout(location = 1) uniform int TimeSeed;

const uint UINT_MAX = 4294967295u; // 2^32 - 1

layout(std140 , binding = 3) uniform EmitterSetting
{
    vec4 minColor;
    vec4 maxColor;
    vec3 minOffset;
    float padding1;
    vec3 maxOffset;
    float padding2;
    vec3 minVelocity;
    float padding3;
    vec3 maxVelocity;
    float padding4;
    vec3 minAccel;
    float padding5;
    vec3 maxAccel;
    float padding6;
    vec3 ForceOrigin;
    float padding7;
    float minLife;
    float maxLife;
    vec3 position;
    float padding10;
}EmitterSettings;

uint PcgHash(uint inputSeed)
{
    uint state = inputSeed * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

uint RandPcg(inout uint rng_state)
{
    uint state = rng_state;
    rng_state = rng_state * 747794u + 5628913u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float RandPcgRange(inout uint seed, float min, float max)
{
    float NormalizedFloat = float(RandPcg(seed)) / float(UINT_MAX);
    return min + (NormalizedFloat * (max - min));
}

vec3 RandPcgRangeVec3(inout uint seed, vec3 min, vec3 max)
{
    return vec3(RandPcgRange(seed,min.x,max.x),
	            RandPcgRange(seed,min.y,max.y),
	            RandPcgRange(seed,min.z,max.z));
}

vec4 RandPcgRangeVec4(inout uint seed, vec4 min, vec4 max)
{
    return vec4(RandPcgRange(seed,min.x,max.x),
	            RandPcgRange(seed,min.y,max.y),
	            RandPcgRange(seed,min.z,max.z),
				RandPcgRange(seed,min.w,max.w));
}

float rand(inout uint state)
{
    uint x = RandPcg(state);
    state = x;
    return float(x) * uintBitsToFloat(0x2f800004u);
}

void SpawnParticle(inout Particle particle , uint index)
{
   uint RandomSeed = index;
   particle.life = RandPcgRange(RandomSeed ,EmitterSettings.minLife,EmitterSettings.maxLife);
   particle.color = RandPcgRangeVec4(RandomSeed, EmitterSettings.minColor, EmitterSettings.maxColor);
   particle.acceleration = RandPcgRangeVec3(RandomSeed,EmitterSettings.minAccel,EmitterSettings.maxAccel);                         
   particle.velocity = RandPcgRangeVec3(RandomSeed ,EmitterSettings.minVelocity,EmitterSettings.maxVelocity);                    
   particle.position = RandPcgRangeVec3(RandomSeed ,EmitterSettings.minOffset,EmitterSettings.maxOffset);
}

layout(local_size_x = 64 , local_size_y = 1, local_size_z = 1) in;
void main()
{
   uint index = gl_GlobalInvocationID.x;
   
   if (index >= ParticleSpawnCount)
   {
       return;
   }
   
   int SpawnIndex = atomicAdd(count,-1) - 1;
   if (SpawnIndex < 0) 
   {
       atomicAdd(count, 1);
       return;
   }

   int particleIndex = indices[SpawnIndex];
   SpawnParticle(particles[particleIndex] , uint(particleIndex) * TimeSeed * uint(index));
}