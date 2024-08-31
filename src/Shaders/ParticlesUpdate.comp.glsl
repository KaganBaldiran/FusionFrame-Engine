#version 460 core

struct Particle
{
    vec3 position;
    vec3 velocity;
    vec3 acceleration;
    vec4 color;
    float life;
};

layout(std430, binding = 1) restrict buffer ParticleBuffer0
{
    Particle particles[];
};

layout(std430, binding = 2) coherent restrict buffer ParticleBuffer1
{
    int count;
    int indices[];
};

layout(location = 0) uniform float dt;
layout(location = 1) uniform vec3 ForceDirection;

void UpdateParticle(inout Particle particle , uint index)
{
   if(particle.life > 0.0f)
   {
      particle.velocity += particle.acceleration * dt;
      particle.position += normalize(ForceDirection) * particle.velocity * dt;
      particle.life -= dt;
      
	  if(particle.life <= 0.0f)
	  {
		particle.color.a = 0.0f;
		indices[atomicAdd(count, 1)] = int(index);	  
	  }
   }
}

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main()
{
    uint index = gl_GlobalInvocationID.x;

    if (index >= particles.length())
	{
       return;
    }
	UpdateParticle(particles[index] ,index);
}