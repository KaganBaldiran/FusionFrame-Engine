#pragma once
#include <glew.h>
#include <glfw3.h>
#include "Texture.h"
#include "Shader.h"
#include "../FusionUtility/VectorMath.h"
#include "../FusionUtility/Log.h"
#include "ShadowMaps.hpp"
#include "Cubemap.h"
#include "Light.hpp"
#include "Buffer.h"
#include <functional>

namespace FUSIONCORE
{

	//Meant for forward rendering pipeline
	class FrameBuffer
	{
	public:
		FrameBuffer(int width , int height)
		{
			static int itr = 0;
			ID = itr;
			FBOSize.SetValues(width, height);

			glGenFramebuffers(1, &fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);

			glGenTextures(1, &fboImage);
			glBindTexture(GL_TEXTURE_2D, fboImage);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboImage, 0);

			glGenTextures(1, &fboDepth);
			glBindTexture(GL_TEXTURE_2D, fboDepth);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, fboDepth, 0);

			glGenTextures(1, &SSLStexture);
			glBindTexture(GL_TEXTURE_2D, SSLStexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, SSLStexture, 0);

			glGenTextures(1, &fboID);
			glBindTexture(GL_TEXTURE_2D, fboID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, width, height, 0, GL_RED, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, fboID, 0);

			unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 ,GL_COLOR_ATTACHMENT2  , GL_COLOR_ATTACHMENT3 };
			glDrawBuffers(4, attachments);

			glGenRenderbuffers(1, &rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, rbo);

			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				LOG_ERR("Error Completing the frameBuffer[ID:"<< ID <<"]!");
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			LOG_INF("Completed the frameBuffer[ID:" << ID << "]!");

			float quadVertices[] = {
				// positions   // texCoords
				-1.0f,  1.0f,  0.0f, 1.0f,
				-1.0f, -1.0f,  0.0f, 0.0f,
				 1.0f, -1.0f,  1.0f, 0.0f,

				-1.0f,  1.0f,  0.0f, 1.0f,
				 1.0f, -1.0f,  1.0f, 0.0f,
				 1.0f,  1.0f,  1.0f, 1.0f
			};

			ObjectBuffer.Bind();
			ObjectBuffer.BufferDataFill(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
			ObjectBuffer.AttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			ObjectBuffer.AttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
			ObjectBuffer.Unbind();

			itr++;
		};

		inline GLuint GetFBOimage() { return fboImage; };
		inline GLuint GetFBO() { return fbo; };
		inline GLuint GetFBODepth() { return fboDepth; };
		inline GLuint GetFBOID() { return fboID; };
		inline void SetFBOimage(GLuint Texture) { this->fboImage = Texture; };
		inline Vec2<int> GetFBOSize() { return FBOSize; };

		inline void Bind() { glBindFramebuffer(GL_FRAMEBUFFER, fbo); };
		inline void Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); };

		inline void Draw(Camera3D& camera,Shader& shader,std::function<void()> ShaderPrep , Vec2<int> WindowSize,CascadedDirectionalShadowMap& sunMap,bool DOFenabled = false, float DOFdistanceFar = 0.09f , float DOFdistanceClose = 0.02f, float DOFintensity = 1.0f)
		{
			//glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, FBOSize.x, FBOSize.y);
			glClearColor(1.0f,1.0f,1.0f,1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glDisable(GL_DEPTH_TEST);
			shader.use();
			ObjectBuffer.BindVAO();
			ShaderPrep();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, fboImage);
			shader.setInt("Viewport", 0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, fboDepth);
			shader.setInt("DepthAttac", 1);

			/*glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, fboID);
			shader.setInt("ID", 2);*/
		
			shader.setVec3("CamPos", camera.Position);
			shader.setFloat("FarPlane", camera.FarPlane);
			shader.setFloat("NearPlane", camera.NearPlane);
			shader.setFloat("DOFdistanceFar", DOFdistanceFar);
			shader.setFloat("DOFdistanceClose", DOFdistanceClose);
			shader.setBool("DOFenabled", DOFenabled);
			shader.setFloat("DOFintensity", DOFintensity);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			ObjectBuffer.UnbindVAO();
			glActiveTexture(0);
			glBindTexture(GL_TEXTURE_2D, 0);
			UseShaderProgram(0);
			glEnable(GL_DEPTH_TEST);
		}

		inline void clean()
		{
			glDeleteTextures(1, &fboImage);
			glDeleteTextures(1, &fboDepth);
			glDeleteTextures(1, &fboID);
			glDeleteTextures(1, &SSLStexture);
			glDeleteRenderbuffers(1, &rbo);
			glDeleteFramebuffers(1, &fbo);

			ObjectBuffer.clean();

			LOG_INF("Cleaned frameBuffer[ID:" << ID << "]!");
		};

	private:

		GLuint fbo, fboImage, fboDepth ,fboID, rbo , SSLStexture;
		Buffer ObjectBuffer;
		Vec2<int> FBOSize;
		int ID;
	};


	//Meant for deferred rendering pipeline
	class Gbuffer
	{
	public:
		Gbuffer(int width, int height)
		{
			static int itr = 0;
			ID = itr;
			FBOSize.SetValues(width, height);

			glGenFramebuffers(1, &fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);

			glGenTextures(1, &AlbedoSpecularPass);
			glBindTexture(GL_TEXTURE_2D, AlbedoSpecularPass);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, AlbedoSpecularPass, 0);

			glGenTextures(1, &NormalMetalicPass);
			glBindTexture(GL_TEXTURE_2D, NormalMetalicPass);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, NormalMetalicPass, 0);

			glGenTextures(1, &PositionDepthPass);
			glBindTexture(GL_TEXTURE_2D, PositionDepthPass);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, PositionDepthPass, 0);

			glGenTextures(1, &MetalicRoughnessPass);
			glBindTexture(GL_TEXTURE_2D, MetalicRoughnessPass);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, MetalicRoughnessPass, 0);

			unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 ,GL_COLOR_ATTACHMENT2 , GL_COLOR_ATTACHMENT3 };
			glDrawBuffers(4, attachments);

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

			float quadVertices[] = {
				// positions   // texCoords
				-1.0f,  1.0f,  0.0f, 1.0f,
				-1.0f, -1.0f,  0.0f, 0.0f,
				 1.0f, -1.0f,  1.0f, 0.0f,

				-1.0f,  1.0f,  0.0f, 1.0f,
				 1.0f, -1.0f,  1.0f, 0.0f,
				 1.0f,  1.0f,  1.0f, 1.0f
			};

			ObjectBuffer.Bind();
			ObjectBuffer.BufferDataFill(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
			ObjectBuffer.AttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			ObjectBuffer.AttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
			ObjectBuffer.Unbind();

			itr++;
		};

		GLuint GetAlbedoSpecularPass() { return AlbedoSpecularPass; };
		GLuint GetFBO() { return fbo; };
		GLuint GetNormalMetalicPass() { return NormalMetalicPass; };
		GLuint GetPositionDepthPass() { return PositionDepthPass; };
		Vec2<int> GetFBOSize() { return FBOSize; };

		void Bind() { glBindFramebuffer(GL_FRAMEBUFFER, fbo); };
		void Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); };

		void Draw(Camera3D& camera, Shader& shader, std::function<void()> ShaderPrep, Vec2<int> WindowSize, std::vector<OmniShadowMap*> ShadowMaps,CascadedDirectionalShadowMap& sunMap,CubeMap& cubeMap,float EnvironmentAmbientAmount = 0.2f)
		{
			//glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, FBOSize.x, FBOSize.y);
			glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glDisable(GL_DEPTH_TEST);
			shader.use();
			ObjectBuffer.BindVAO();
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
		
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap.GetConvDiffCubeMap());
			shader.setInt("ConvDiffCubeMap", 4);

			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap.GetPreFilteredEnvMap());
			shader.setInt("prefilteredMap", 5);

			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, FUSIONCORE::brdfLUT);
			shader.setInt("LUT", 6);

			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D_ARRAY, sunMap.GetShadowMap());
			shader.setInt("SunShadowMap", 7);

			shader.setMat4("ViewMatrix", camera.viewMat);

			auto& CascadeLevels = sunMap.GetCascadeLevels();
			shader.setInt("CascadeCount", CascadeLevels.size());
			shader.setInt("CascadeShadowMapLightID", sunMap.GetBoundLightID());
			for (size_t i = 0; i < CascadeLevels.size(); i++)
			{
				shader.setFloat("CascadeShadowPlaneDistances[" + std::to_string(i) + "]", CascadeLevels[i]);
			}

			shader.setBool("EnableIBL", true);
			shader.setFloat("ao", EnvironmentAmbientAmount);
			shader.setInt("OmniShadowMapCount", ShadowMaps.size());

			for (size_t i = 0; i < ShadowMaps.size(); i++)
			{
				glActiveTexture(GL_TEXTURE0 + 8 + i);
				glBindTexture(GL_TEXTURE_CUBE_MAP, ShadowMaps[i]->GetShadowMap());
				glUniform1i(glGetUniformLocation(shader.GetID(), ("OmniShadowMaps[" + std::to_string(i) + "]").c_str()), 8 + i);
				glUniform1f(glGetUniformLocation(shader.GetID(), ("ShadowMapFarPlane[" + std::to_string(i) + "]").c_str()), ShadowMaps[i]->GetFarPlane());
				glUniform1i(glGetUniformLocation(shader.GetID(), ("OmniShadowMapsLightIDS[" + std::to_string(i) + "]").c_str()), ShadowMaps[i]->GetBoundLightID());	
			}

			FUSIONCORE::SendLightsShader(shader);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			ObjectBuffer.UnbindVAO();
			glActiveTexture(0);
			glBindTexture(GL_TEXTURE_2D, 0);
			UseShaderProgram(0);
			glEnable(GL_DEPTH_TEST);
		}

		void clean()
		{
			glDeleteTextures(1, &AlbedoSpecularPass);
			glDeleteTextures(1, &NormalMetalicPass);
			glDeleteTextures(1, &PositionDepthPass);
			glDeleteTextures(1, &MetalicRoughnessPass);
			glDeleteRenderbuffers(1, &rbo);
			glDeleteFramebuffers(1, &fbo);

			ObjectBuffer.clean();

			LOG_INF("Cleaned G-buffer[ID:" << ID << "]!");
		};

	private:

		GLuint fbo, AlbedoSpecularPass, NormalMetalicPass, rbo, PositionDepthPass , MetalicRoughnessPass;
		Buffer ObjectBuffer;
		Vec2<int> FBOSize;
		int ID;
	};

	inline void CopyDepthInfoFBOtoFBO(GLuint src,glm::vec2 srcSize, GLuint dest) 
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, src);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest); 
		glBlitFramebuffer(0, 0, srcSize.x, srcSize.y, 0, 0, srcSize.x, srcSize.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	};

}


