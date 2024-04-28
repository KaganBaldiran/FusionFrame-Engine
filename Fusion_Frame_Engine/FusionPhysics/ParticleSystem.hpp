#pragma once
#include "../FusionCore/Object.hpp"
#include "../FusionCore/Buffer.h"
#include "../FusionCore/Shader.h"
#include "../FusionCore/Model.hpp"
#include "../FusionUtility/FusionDLLExport.h"

namespace FUSIONPHYSICS
{
	FUSIONFRAME_EXPORT_FUNCTION std::unique_ptr<FUSIONCORE::UBO> EmitterSettingsUBO;
	FUSIONFRAME_EXPORT_FUNCTION void InitializeParticleEmitterUBO();

	 struct FUSIONFRAME_EXPORT alignas(16) EmitterSettings {
		glm::vec4 minColor;
		glm::vec4 maxColor;
		glm::vec3 minOffset;
		float padding1;
		glm::vec3 maxOffset;
		float padding2;
		glm::vec3 minVelocity;
		float padding3;
		glm::vec3 maxVelocity;
		float padding4;
		glm::vec3 minAccel;
		float padding5;
		glm::vec3 maxAccel;
		float padding6;
		glm::vec3 ForceDirection;
		float padding7;
		float minLife;
		float maxLife;
		glm::vec3 position;
		float padding10;
	};

	//Particle system calculated on GPU using compute shaders
	class FUSIONFRAME_EXPORT ParticleEmitter : public FUSIONCORE::Object
	{
	public:
		ParticleEmitter(unsigned int MaxParticleCount,
			FUSIONCORE::Shader& ParticleInitializerShader,
			glm::vec4 minColor = glm::vec4(0.5f, 0.0f, 0.0f, 0.5f),
			glm::vec4 maxColor = glm::vec4(1.0f,0.0f,0.0f,1.0f),    
			glm::vec3 minOffset = glm::vec3(0.0f),   
			glm::vec3 maxOffset = glm::vec3(4.0f),   
			glm::vec3 minVelocity = glm::vec3(0.0f), 
			glm::vec3 maxVelocity = glm::vec3(0.1f), 
			glm::vec3 minAccel = glm::vec3(0.0f),    
			glm::vec3 maxAccel = glm::vec3(1.0f),    
			glm::vec3 ForceDirection = glm::vec3(1.0f),
			float minLife = 1.0f,                    
			float maxLife = 5.0f,                    
			float spawnInterval = 0.0001f              
		);
		void UpdateParticleEmitter(FUSIONCORE::Shader& ParticleUpdateShader, FUSIONCORE::Shader& ParticleInitializerShader, const float DeltaTime);
		void DrawParticles(FUSIONCORE::Shader& ParticleShader, FUSIONCORE::Mesh& ParticleMesh,FUSIONCORE::WorldTransform& ParticleTranform, FUSIONCORE::Camera3D& camera);
		EmitterSettings& GetEmitterSettings() { return this->emitterSettings; };
	
	private:
		EmitterSettings emitterSettings;
		unsigned int InitialSpawnCount;
		float spawnInterval, timer;
		uint32_t maxParticles;
		FUSIONCORE::SSBO particlesBuffer;
		FUSIONCORE::SSBO freelistBuffer;
	};   
};