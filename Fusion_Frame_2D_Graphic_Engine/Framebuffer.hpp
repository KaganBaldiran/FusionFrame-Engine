#pragma once
#include <glew.h>
#include <glfw3.h>
#include "Texture.h"
#include "Shader.h"
#include "VectorMath.h"
#include "Log.h"
#include "Buffer.h"
#include <functional>

namespace FUSIONOPENGL
{
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
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, fboDepth, 0);

			unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(2, attachments);

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

		GLuint GetFBOimage() { return fboImage; };
		GLuint GetFBO() { return fbo; };
		GLuint GetFBODepth() { return fboDepth; };
		Vec2<int> GetFBOSize() { return FBOSize; };

		void Bind() { glBindFramebuffer(GL_FRAMEBUFFER, fbo); };
		void Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); };

		void Draw(Camera3D& camera,Shader& shader,std::function<void()> ShaderPrep , Vec2<int> WindowSize , float DOFdistance)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

			shader.setVec3("CamPos", camera.Position);
			shader.setFloat("FarPlane", camera.FarPlane);
			shader.setFloat("NearPlane", camera.NearPlane);
			shader.setFloat("DOFdistance", DOFdistance);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			ObjectBuffer.UnbindVAO();
			glActiveTexture(0);
			glBindTexture(GL_TEXTURE_2D, 0);
			UseShaderProgram(0);
			glEnable(GL_DEPTH_TEST);
		}

		void clean()
		{
			glDeleteTextures(1, &fboImage);
			glDeleteTextures(1, &fboDepth);
			glDeleteRenderbuffers(1, &rbo);
			glDeleteFramebuffers(1, &fbo);

			ObjectBuffer.clean();

			LOG_INF("Cleaned frameBuffer[ID:" << ID << "]!");
		};

	private:

		GLuint fbo, fboImage, fboDepth , rbo;
		Buffer ObjectBuffer;
		Vec2<int> FBOSize;
		int ID;
	};
}


