#version 460 core

layout(location = 0) in vec3 vertexdata;
layout(location = 2) in vec2 textcoord;

out vec3 CurrentPos;
out vec4 ParticleColor;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 ProjView;
uniform vec3 Emitterposition;

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
 
void main()
{ 
    TexCoords = textcoord;

    Particle TempParticle = particles[gl_InstanceID];
    CurrentPos = vec3(model * vec4(vertexdata,1.0f));
    CurrentPos = CurrentPos + TempParticle.position + Emitterposition;
    gl_Position = ProjView * vec4(CurrentPos,1.0f);
    ParticleColor = TempParticle.color;
}