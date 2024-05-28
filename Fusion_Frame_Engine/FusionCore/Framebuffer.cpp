#include "Framebuffer.hpp"
#include <glew.h>
#include <glfw3.h>

std::unique_ptr<FUSIONCORE::VBO> ObjectBufferVBO;
std::unique_ptr<FUSIONCORE::VAO> ObjectBufferVAO;

void FUSIONCORE::InitializeFBObuffers()
{
	ObjectBufferVBO = std::make_unique<VBO>();
	ObjectBufferVAO = std::make_unique<VAO>();

	float quadVertices[] = {
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	ObjectBufferVAO->Bind();
	ObjectBufferVBO->Bind();
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	BindVAONull();
	BindVBONull();
}

void FUSIONCORE::CopyDepthInfoFBOtoFBO(GLuint src, glm::vec2 srcSize, GLuint dest)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest);
	glBlitFramebuffer(0, 0, srcSize.x, srcSize.y, 0, 0, srcSize.x, srcSize.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
};

FUSIONCORE::Color FUSIONCORE::ReadFrameBufferPixel(int Xcoord, int Ycoord,unsigned int FramebufferAttachmentMode,GLenum AttachmentFormat,glm::vec2 CurrentWindowSize)
{
	glReadBuffer((GLenum)FramebufferAttachmentMode);
	float pixel[4];
	glReadPixels(Xcoord ,CurrentWindowSize.y - Ycoord, 1, 1, AttachmentFormat, GL_FLOAT, &pixel);
	//LOG("PIXEL PICKED: " << pixel[0] << " " << pixel[1] << " " << pixel[2] << " " << pixel[3] << " " << Xcoord << " " << Ycoord);
	glReadBuffer(GL_NONE);
	Color PixelColor;
	PixelColor.SetRGBA({ pixel[0], pixel[1], pixel[2], pixel[3] });
    return PixelColor;
}

FUSIONCORE::FrameBuffer::FrameBuffer(int width, int height)
{
	static int itr = 0;
	ID = itr;
	FBOSize.SetValues(width, height);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &fboImage);
	glBindTexture(GL_TEXTURE_2D, fboImage);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboImage, 0);

	glGenTextures(1, &fboDepth);
	glBindTexture(GL_TEXTURE_2D, fboDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, fboDepth, 0);

	glGenTextures(1, &SSRtexture);
	glBindTexture(GL_TEXTURE_2D, SSRtexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, SSRtexture, 0);

	glGenTextures(1, &IDtexture);
	glBindTexture(GL_TEXTURE_2D, IDtexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, width, height, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, IDtexture, 0);

	unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 ,GL_COLOR_ATTACHMENT2  , GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, attachments);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOG_ERR("Error Completing the frameBuffer[ID:" << ID << "]!");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	LOG_INF("Completed the frameBuffer[ID:" << ID << "]!");

	itr++;
}

void FUSIONCORE::FrameBuffer::Bind() 
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo); 
}
void FUSIONCORE::FrameBuffer::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
};

void FUSIONCORE::FrameBuffer::Draw(Camera3D& camera, Shader& shader,std::function<void()> ShaderPrep, Vec2<int> WindowSize, bool DOFenabled, float DOFdistanceFar, float DOFdistanceClose, float DOFintensity, float Gamma, float Exposure)
{
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, FBOSize.x, FBOSize.y);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	shader.use();
	ObjectBufferVAO->Bind();
	ShaderPrep();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fboImage);
	shader.setInt("Viewport", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fboDepth);
	shader.setInt("DepthAttac", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, SSRtexture);
	shader.setInt("SSRtexture", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, IDtexture);
	shader.setInt("IDtexture", 3);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D_ARRAY, GetCascadedShadowMapTextureArray());
	shader.setInt("CascadeShadowMaps1024", 4);

	shader.setVec3("CamPos", camera.Position);
	shader.setFloat("FarPlane", camera.FarPlane);
	shader.setFloat("NearPlane", camera.NearPlane);
	shader.setFloat("DOFdistanceFar", DOFdistanceFar);
	shader.setFloat("DOFdistanceClose", DOFdistanceClose);
	shader.setBool("DOFenabled", DOFenabled);
	shader.setFloat("DOFintensity", DOFintensity);
	shader.setFloat("Gamma", Gamma);
	shader.setFloat("Exposure", Exposure);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	BindVAONull();
	UseShaderProgram(0);
	glEnable(GL_DEPTH_TEST);
}

void FUSIONCORE::FrameBuffer::clean()
{
	glDeleteTextures(1, &fboImage);
	glDeleteTextures(1, &fboDepth);
	glDeleteTextures(1, &IDtexture);
	glDeleteTextures(1, &SSRtexture);
	glDeleteRenderbuffers(1, &rbo);
	glDeleteFramebuffers(1, &fbo);

	LOG_INF("Cleaned frameBuffer[ID:" << ID << "]!");
};

FUSIONCORE::Gbuffer::Gbuffer(int width, int height)
{
	static int itr = 0;
	ID = itr;
	FBOSize.x = width;
	FBOSize.y = height;

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &AlbedoSpecularPass);
	glBindTexture(GL_TEXTURE_2D, AlbedoSpecularPass);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, AlbedoSpecularPass, 0);

	glGenTextures(1, &NormalMetalicPass);
	glBindTexture(GL_TEXTURE_2D, NormalMetalicPass);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, NormalMetalicPass, 0);

	glGenTextures(1, &PositionDepthPass);
	glBindTexture(GL_TEXTURE_2D, PositionDepthPass);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, PositionDepthPass, 0);

	glGenTextures(1, &MetalicRoughnessPass);
	glBindTexture(GL_TEXTURE_2D, MetalicRoughnessPass);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, MetalicRoughnessPass, 0);

	glGenTextures(1, &DecalNormalPass);
	glBindTexture(GL_TEXTURE_2D, DecalNormalPass);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, DecalNormalPass, 0);

	SetDrawModeDefault();

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOG_ERR("Error Completing the G-buffer[ID:" << ID << "]!");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	LOG_INF("Completed the G-buffer[ID:" << ID << "]!");

	itr++;
}


void FUSIONCORE::Gbuffer::SetDrawModeDefault()
{
	unsigned int attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 ,GL_COLOR_ATTACHMENT2 , GL_COLOR_ATTACHMENT3 , GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(5, attachments);
}

void FUSIONCORE::Gbuffer::SetDrawModeDecalPass()
{
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT3,GL_COLOR_ATTACHMENT4 };
	glDrawBuffers(3, attachments);
}

void FUSIONCORE::Gbuffer::SetDrawModeDefaultRestricted()
{
	unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 ,GL_COLOR_ATTACHMENT2 , GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, attachments);
}

void FUSIONCORE::Gbuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}
void FUSIONCORE::Gbuffer::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
};

void FUSIONCORE::Gbuffer::DrawSceneDeferred(Camera3D& camera, Shader& shader, std::function<void()> ShaderPrep, Vec2<int> WindowSize, std::vector<OmniShadowMap*> &ShadowMaps,CubeMap& cubeMap, glm::vec4 BackgroundColor, float EnvironmentAmbientAmount)
{
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 , GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(3, attachments);
	glViewport(0, 0, FBOSize.x, FBOSize.y);
	glClearColor(BackgroundColor.x, BackgroundColor.y, BackgroundColor.z, BackgroundColor.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	shader.use();
	ObjectBufferVAO->Bind();
	ShaderPrep();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, AlbedoSpecularPass);
	shader.setInt("AlbedoSpecularPass", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, NormalMetalicPass);
	shader.setInt("NormalPass", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, PositionDepthPass);
	shader.setInt("PositionDepthPass", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, MetalicRoughnessPass);
	shader.setInt("MetalicRoughnessModelIDPass", 3);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, DecalNormalPass);
	shader.setInt("DecalNormalPass", 4);

	shader.setVec3("CameraPos", camera.Position);
	shader.setFloat("FarPlane", camera.FarPlane);
	shader.setFloat("NearPlane", camera.NearPlane);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap.GetConvDiffCubeMap());
	shader.setInt("ConvDiffCubeMap", 5);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap.GetPreFilteredEnvMap());
	shader.setInt("prefilteredMap", 6);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, FUSIONCORE::brdfLUT);
	shader.setInt("LUT", 7);

	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D_ARRAY, FUSIONCORE::GetCascadedShadowMapTextureArray());
	shader.setInt("CascadeShadowMaps", 8);

	auto CascadedShadowMapsMetaData = GetCascadedShadowMapMetaDataSSBO();
	CascadedShadowMapsMetaData->BindSSBO(10);

	shader.setMat4("ViewMatrix", camera.viewMat);

	shader.setBool("EnableIBL", true);
	shader.setFloat("ao", EnvironmentAmbientAmount);
	shader.setInt("OmniShadowMapCount", ShadowMaps.size());

	for (size_t i = 0; i < ShadowMaps.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + 9 + i);
		glBindTexture(GL_TEXTURE_CUBE_MAP, ShadowMaps[i]->GetShadowMap());
		glUniform1i(glGetUniformLocation(shader.GetID(), ("OmniShadowMaps[" + std::to_string(i) + "]").c_str()), 9 + i);
		glUniform1f(glGetUniformLocation(shader.GetID(), ("ShadowMapFarPlane[" + std::to_string(i) + "]").c_str()), ShadowMaps[i]->GetFarPlane());
		glUniform1i(glGetUniformLocation(shader.GetID(), ("OmniShadowMapsLightIDS[" + std::to_string(i) + "]").c_str()), ShadowMaps[i]->GetBoundLightID());
	}

	FUSIONCORE::SendLightsShader(shader);

	shader.setVec2("screenSize", glm::vec2(1920, 1080));

	glDrawArrays(GL_TRIANGLES, 0, 6);

	BindVAONull();
	UseShaderProgram(0);
	glEnable(GL_DEPTH_TEST);
}

void FUSIONCORE::Gbuffer::DrawSSR(Camera3D& camera, Shader& shader, std::function<void()> ShaderPrep, Vec2<int> WindowSize)
{
	unsigned int attachments[1] = { GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(1, attachments);
	glViewport(0, 0, WindowSize.x, WindowSize.y);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	shader.use();
	ObjectBufferVAO->Bind();
	ShaderPrep();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, AlbedoSpecularPass);
	shader.setInt("AlbedoSpecularPass", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, NormalMetalicPass);
	shader.setInt("NormalMetalicPass", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, PositionDepthPass);
	shader.setInt("PositionDepthPass", 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, MetalicRoughnessPass);
	shader.setInt("MetalicRoughnessPass", 3);

	shader.setVec3("CameraPos", camera.Position);
	shader.setFloat("FarPlane", camera.FarPlane);
	shader.setFloat("NearPlane", camera.NearPlane);

	shader.setMat4("ViewMatrix", camera.viewMat);
	shader.setMat4("InverseViewMatrix", glm::inverse(camera.viewMat));
	shader.setMat4("InverseProjectionMatrix", glm::inverse(camera.projMat));
	shader.setMat4("ProjectionMatrix", camera.projMat);

	shader.setVec2("WindowSize", { WindowSize.x,WindowSize.y });

	glDrawArrays(GL_TRIANGLES, 0, 6);

	BindVAONull();
	glActiveTexture(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	UseShaderProgram(0);
	glEnable(GL_DEPTH_TEST);
}

void FUSIONCORE::Gbuffer::clean()
{
	glDeleteTextures(1, &AlbedoSpecularPass);
	glDeleteTextures(1, &NormalMetalicPass);
	glDeleteTextures(1, &PositionDepthPass);
	glDeleteTextures(1, &MetalicRoughnessPass);
	glDeleteTextures(1, &DecalNormalPass);
	glDeleteRenderbuffers(1, &rbo);
	glDeleteFramebuffers(1, &fbo);

	LOG_INF("Cleaned G-buffer[ID:" << ID << "]!");
};
