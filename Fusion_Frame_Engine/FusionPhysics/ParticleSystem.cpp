#include "ParticleSystem.hpp"

namespace FUSIONPHYSICS
{
	std::unique_ptr<FUSIONCORE::UBO> EmitterSettingsUBO;
}

void FUSIONPHYSICS::InitializeParticleEmitterUBO()
{
	EmitterSettingsUBO = std::make_unique<FUSIONCORE::UBO>();
	EmitterSettingsUBO->Bind();
	glBufferData(GL_UNIFORM_BUFFER, sizeof(EmitterSettings), nullptr, GL_DYNAMIC_COPY);
	EmitterSettingsUBO->BindUBO(3);
	FUSIONCORE::BindUBONull();
};

struct Particle
{
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	glm::vec4 color;
	float life;
};

FUSIONPHYSICS::ParticleEmitter::ParticleEmitter(unsigned int MaxParticleCount, FUSIONCORE::Shader& ParticleInitializeShader, glm::vec4 minColor, glm::vec4 maxColor,
	glm::vec3 minOffset, glm::vec3 maxOffset, glm::vec3 minVelocity, glm::vec3 maxVelocity, glm::vec3 minAccel, glm::vec3 maxAccel,
	glm::vec3 ForceOrigin, float minLife, float maxLife ,float spawnInterval)
{
	emitterSettings.ForceOrigin = ForceOrigin;
	emitterSettings.maxAccel = maxAccel;
	emitterSettings.maxColor = maxColor;
	emitterSettings.maxLife = maxLife;
	emitterSettings.maxOffset = maxOffset;
	emitterSettings.maxVelocity = maxVelocity;
	emitterSettings.minAccel = minAccel;
	emitterSettings.minColor = minColor;
	emitterSettings.minLife = minLife;
	emitterSettings.minOffset = minOffset;
	emitterSettings.minVelocity = minVelocity;
	emitterSettings.position = this->GetTransformation().Position;

	this->spawnInterval = spawnInterval;
	this->maxParticles = MaxParticleCount;
	timer = 0.0f;

	this->particlesBuffer.Bind();
	particlesBuffer.BufferDataFill(GL_SHADER_STORAGE_BUFFER, MaxParticleCount * sizeof(Particle), nullptr, GL_DYNAMIC_COPY);
	particlesBuffer.BindSSBO(1);

	this->freelistBuffer.Bind();
	freelistBuffer.BufferDataFill(GL_SHADER_STORAGE_BUFFER, (MaxParticleCount + 1) * sizeof(int), nullptr, GL_DYNAMIC_COPY);
	freelistBuffer.BindSSBO(2);
	FUSIONCORE::BindSSBONull();

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	ParticleInitializeShader.use();

	EmitterSettingsUBO->Bind();
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(EmitterSettings), &this->emitterSettings);
	EmitterSettingsUBO->BindUBO(3);
	FUSIONCORE::BindUBONull();

	particlesBuffer.BindSSBO(1);
	ParticleInitializeShader.setInt("TimeSeed", time(0));

	const unsigned int WorkGroupSize = 64;
	int numWorkGroups = (maxParticles + WorkGroupSize - 1) / WorkGroupSize;
	glDispatchCompute(numWorkGroups, 1, 1);

	FUSIONCORE::UseShaderProgram(0);
}

void FUSIONPHYSICS::ParticleEmitter::UpdateParticleEmitter(FUSIONCORE::Shader& ParticleUpdateShader, FUSIONCORE::Shader& ParticleSpawnShader, const float DeltaTime)
{
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	timer += DeltaTime;
	unsigned int SpawnCount = timer / spawnInterval;
	timer = glm::mod(timer, spawnInterval);

	const unsigned int WorkGroupSize = 64;
	if (SpawnCount > 0)
	{
		//LOG("SpawnCount: " << SpawnCount);

		ParticleSpawnShader.use();

		EmitterSettingsUBO->Bind();
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(EmitterSettings), &this->emitterSettings);
		EmitterSettingsUBO->BindUBO(3);
		FUSIONCORE::BindUBONull();

		particlesBuffer.BindSSBO(1);
		freelistBuffer.BindSSBO(2);

		ParticleSpawnShader.setInt("ParticleSpawnCount", SpawnCount);
		ParticleSpawnShader.setInt("TimeSeed", time(0));

		int numWorkGroups = (SpawnCount + WorkGroupSize - 1) / WorkGroupSize;
		glDispatchCompute(numWorkGroups, 1, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	ParticleUpdateShader.use();
	ParticleUpdateShader.setFloat("dt", DeltaTime);
	ParticleUpdateShader.setVec3("ForceOrigin", emitterSettings.ForceOrigin);
	
	particlesBuffer.BindSSBO(1);
	freelistBuffer.BindSSBO(2);

	int numWorkGroups = (maxParticles + WorkGroupSize - 1) / WorkGroupSize;
	glDispatchCompute(numWorkGroups, 1, 1);

	FUSIONCORE::UseShaderProgram(0);
}

void FUSIONPHYSICS::ParticleEmitter::DrawParticles(FUSIONCORE::Shader& ParticleShader , FUSIONCORE::Mesh& ParticleMesh, FUSIONCORE::WorldTransform& ParticleTranform, FUSIONCORE::Camera3D& camera)
{
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	ParticleShader.use();
	ParticleMesh.GetMeshBuffer().BindVAO();

	camera.SetProjMatrixUniformLocation(ParticleShader.GetID(), "proj");
	camera.SetViewMatrixUniformLocation(ParticleShader.GetID(), "view");
	
	glm::mat4 LookatMat = glm::lookAt(glm::vec3(0.0f), {-camera.Orientation.x,camera.Orientation.y,-camera.Orientation.z}, camera.GetUpVector());
	ParticleShader.setMat4("LookAtMat", LookatMat);
	ParticleShader.setMat4("model",ParticleTranform.GetModelMat4());

	particlesBuffer.BindSSBO(1);

	glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(ParticleMesh.GetIndices().size()), GL_UNSIGNED_INT, 0, maxParticles);

	FUSIONCORE::UseShaderProgram(0);
	FUSIONCORE::BindVAONull();
	glActiveTexture(GL_TEXTURE0);
}
