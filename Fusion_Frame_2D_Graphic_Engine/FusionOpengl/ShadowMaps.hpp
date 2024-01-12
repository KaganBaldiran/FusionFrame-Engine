#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/VectorMath.h"
#include "../FusionUtility/Log.h"
#include "Buffer.h"
#include "Texture.h"
#include "Shader.h"
#include "Camera.h"

namespace FUSIONOPENGL
{
	class OmniDirectionalShadowMap
	{
	public:

		OmniDirectionalShadowMap(int width, int height);
		void LightMatrix(glm::vec3 lightPosition , Shader &shader);
		void Clean();
		//void Draw(GLuint shader, glm::vec3 lightPos_i, std::vector<Model*>& models, Camera3D& camera);

		GLuint& GetShadowMap() { return this->ShadowMapId; };
		Vec2<float> GetShadowMapSize() { this->ShadowMapSize; };
		float GetFarPlane() { return far; };

	private:
		Buffer3D ObjectBuffer;
		GLuint ShadowMapId, depthMapFBO;
		Vec2<float> ShadowMapSize;
		glm::mat4 shadowProj;
		const float far = 25.0f;
	};




}