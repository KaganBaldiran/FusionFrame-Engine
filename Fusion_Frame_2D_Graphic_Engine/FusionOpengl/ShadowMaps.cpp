#include "ShadowMaps.hpp"

FUSIONOPENGL::OmniDirectionalShadowMap::OmniDirectionalShadowMap(int width, int height)
{



}

void FUSIONOPENGL::OmniDirectionalShadowMap::LightMatrix(glm::vec3 lightPosition, Shader& shader)
{
}

void FUSIONOPENGL::OmniDirectionalShadowMap::Clean()
{
	ObjectBuffer.clean();
	glDeleteTextures(1, &this->ShadowMapId);
	glDeleteFramebuffers(1, &depthMapFBO);
}
