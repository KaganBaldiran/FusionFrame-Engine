#version 460 core

struct Particle
{
    vec3 position;
    vec3 velocity;
    vec3 acceleration;
    vec4 color;
    float life;
};

const uint UINT_MAX = 4294967295u; // 2^32 - 1

uint PcgHash(uint inputSeed)
{
    uint state = inputSeed * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

uint RandPcg(inout uint rng_state)
{
    uint state = rng_state;
    rng_state = rng_state * 74779u ^ 5628913u;
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
    return vec3(RandPcgRange(seed, min.x, max.x),
        RandPcgRange(seed, min.y, max.y),
        RandPcgRange(seed, min.z, max.z));
}

vec4 RandPcgRangeVec4(inout uint seed, vec4 min, vec4 max)
{
    return vec4(RandPcgRange(seed, min.x, max.x),
        RandPcgRange(seed, min.y, max.y),
        RandPcgRange(seed, min.z, max.z),
        RandPcgRange(seed, min.w, max.w));
}

float rand(inout uint state)
{
    uint x = RandPcg(state);
    state = x;
    return float(x) * uintBitsToFloat(0x2f800004u);
}

