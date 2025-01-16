#include <glew.h>
#include <glfw3.h>
#include "Cubemap.h"
#include "stb_image.h"
#include "Texture.h"
#include "../FusionUtility/Log.h"

namespace FUSIONCORE
{
    GLuint brdfLUT;
}

FUSIONCORE::CubeMap::CubeMap(std::vector<std::string> texture_faces ,Shader &CubeMapShader,int BinningSizeX,int BinningSizeY)
{
    this->texture_faces.assign(texture_faces.begin(),texture_faces.end());
	
	glGenTextures(1, &this->cubemaptextureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->cubemaptextureID);

    int width, height, nrchannels;
    unsigned char* data;

    for (size_t i = 0; i < texture_faces.size(); i++)
    {
        data = stbi_load(this->texture_faces[i].c_str(), &width, &height, &nrchannels, 0);

        if (data)
        {

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0,GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);

        }
        else
        {

            std::cout << "Cubemap error :: failed to load at path  " << this->texture_faces.at(i) << "\n";
            stbi_image_free(data);

        }
    }


    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    cubemapshader = &CubeMapShader;

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };



    vao.Bind();
    vbo.Bind();

    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),(void*)0);

    BindVAONull();
    BindVBONull();

    BinningSize = { BinningSizeX ,BinningSizeY };
    InitializeRadianceBuffers();
}

FUSIONCORE::CubeMap::CubeMap(GLuint CubeMap, Shader& CubeMapShader, int BinningSizeX, int BinningSizeY)
{
    this->cubemaptextureID = CubeMap;
    cubemapshader = &CubeMapShader;

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };



    vao.Bind();
    vbo.Bind();

    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    BindVAONull();
    BindVBONull();

    BinningSize = { BinningSizeX ,BinningSizeY };
    InitializeRadianceBuffers();
}

FUSIONCORE::CubeMap::CubeMap(Shader& CubeMapShader, int BinningSizeX , int BinningSizeY)
{
    cubemapshader = &CubeMapShader;

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };


    vao.Bind();
    vbo.Bind();

    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    BindVAONull();
    BindVBONull();

    BinningSize = { BinningSizeX ,BinningSizeY };
    InitializeRadianceBuffers();
}

FUSIONCORE::CubeMap::~CubeMap()
{
    glDeleteTextures(1, &this->cubemaptextureID);
}

void FUSIONCORE::CubeMap::Draw(Camera3D &camera, const glm::vec2& windowSize)
{
    
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);

    UseShaderProgram(cubemapshader->GetID());

    glm::mat4 view = glm::mat4(glm::mat3(camera.viewMat));

    glUniform1i(glGetUniformLocation(cubemapshader->GetID(), "skybox"), 0);
    glUniformMatrix4fv(glGetUniformLocation(cubemapshader->GetID(), "projection"), 1, GL_FALSE, glm::value_ptr(camera.projMat));
    glUniformMatrix4fv(glGetUniformLocation(cubemapshader->GetID(), "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(cubemapshader->GetID(), "ScreenRatio"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

    vao.Bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->cubemaptextureID);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    BindVAONull();

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    UseShaderProgram(0);

}

void FUSIONCORE::CubeMap::SetCubeMapTexture(GLuint CubeMapTexture)
{
    glDeleteTextures(1, &this->cubemaptextureID);
    this->cubemaptextureID = CubeMapTexture;
}

void FUSIONCORE::CubeMap::SetPreFilteredEnvMap(GLuint preFilteredEnvironmentMapID)
{
    this->PrefilteredEnvMap = preFilteredEnvironmentMapID;
}

void FUSIONCORE::CubeMap::SetConvDiffCubeMap(GLuint ConvDiffCubeMapID)
{
    this->ConvDiffCubeMap = ConvDiffCubeMapID;
}

void FUSIONCORE::CubeMap::CalculateBinRadiances(Shader& ShaderBinning, Shader& ShaderPrefixSum, Shader& ShaderGroupPrefixSum,Shader& ShaderAggregatePrefixSums)
{
    ShaderBinning.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, GetCubeMapTexture());
    ShaderBinning.setInt("EnvironmentCubeMap", 0);
    ShaderBinning.setVec2("BinSize", BinningSize);
    this->RadianceBuffer.BindSSBO(12);

    GLuint workGroupSizeX = 32;
    GLuint workGroupSizeY = 32;

    GLuint numGroupsX = (BinningSize.x + workGroupSizeX - 1) / workGroupSizeX;
    GLuint numGroupsY = (BinningSize.y + workGroupSizeY - 1) / workGroupSizeY;
    GLuint numGroupsZ = 6;

    glDispatchCompute(numGroupsX,numGroupsY,numGroupsZ);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    workGroupSizeX = 256;
    

    ShaderPrefixSum.use();

    this->RadianceBuffer.BindSSBO(12);
    this->RadianceSumBuffer.BindSSBO(13);
    this->SummedWorkgroupBuffer.BindSSBO(14);

    numGroupsX = ((BinningSize.x * BinningSize.y * 6) + workGroupSizeX - 1) / workGroupSizeX;

    glDispatchCompute(numGroupsX, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);



    ShaderGroupPrefixSum.use();

    this->RadianceBuffer.BindSSBO(12);
    this->RadianceSumBuffer.BindSSBO(13);
    this->SummedWorkgroupBuffer.BindSSBO(14);

    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);


    ShaderAggregatePrefixSums.use();

    this->RadianceBuffer.BindSSBO(12);
    this->RadianceSumBuffer.BindSSBO(13);
    this->SummedWorkgroupBuffer.BindSSBO(14);

    glDispatchCompute(numGroupsX, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    UseShaderProgram(0);
    
}

void FUSIONCORE::CubeMap::InitializeRadianceBuffers()
{
    if (this->BinningSize.x > 0 && this->BinningSize.y > 0)
    {
        size_t Size = 6 * BinningSize.x * BinningSize.y * (sizeof(float) + sizeof(int) + sizeof(glm::vec4));
        RadianceBuffer.Bind();
        RadianceBuffer.BufferDataFill(GL_SHADER_STORAGE_BUFFER, Size, NULL, GL_STATIC_COPY);
        RadianceBuffer.BindSSBO(12);
        RadianceBuffer.Unbind();

        RadianceSumBuffer.Bind();
        RadianceSumBuffer.BufferDataFill(GL_SHADER_STORAGE_BUFFER, ((6 * BinningSize.x * BinningSize.y)) * sizeof(float), NULL, GL_STATIC_COPY);
        RadianceSumBuffer.BindSSBO(13);
        RadianceSumBuffer.Unbind();

        SummedWorkgroupBuffer.Bind();
        SummedWorkgroupBuffer.BufferDataFill(GL_SHADER_STORAGE_BUFFER, ((6 * BinningSize.x * BinningSize.y) / 256) * sizeof(float), NULL, GL_STATIC_COPY);
        SummedWorkgroupBuffer.BindSSBO(14);
        SummedWorkgroupBuffer.Unbind();
    }
}

std::pair<GLuint, int> FUSIONCORE::HDRItoCubeMap(const char* HDRI, unsigned int CubeMapSize, GLuint HDRItoCubeMapShader)
{
    int width, height, components;

    stbi_set_flip_vertically_on_load(true);
    float *data = stbi_loadf(HDRI, &width, &height, &components, 0);
    
    GLuint HDRItexture;

    if (data)
    {
           GLenum Format = 0;

           if (components == 1)
           {
              Format = GL_RED;
           }
           else if (components == 4)
           {
             Format = GL_RGBA;
           }
           else if (components == 3)
           {
             Format = GL_RGB;
           }
           else
           {
               LOG_ERR("Cannot load the HDRI texture!");
               return { 0,FF_HDRI_INCOMPATIBLE_FILE };
           }

            glGenTextures(1, &HDRItexture);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, HDRItexture);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, Format, GL_FLOAT, data);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glGenerateMipmap(GL_TEXTURE_2D);

            stbi_image_free(data);

            glBindTexture(GL_TEXTURE_2D, 0); 

    }
    else
    {
        LOG_ERR("Cannot load the HDRI texture!");
        return { 0,FF_HDRI_INCOMPATIBLE_FILE };
    }


    GLuint fbo , rbo , CubeMapColorAttachment;

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, CubeMapSize, CubeMapSize);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        LOG_ERR("Error completing the framebuffer!");
        return { 0,FF_HDRI_ERROR };
    }

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenTextures(1, &CubeMapColorAttachment);
    glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMapColorAttachment);

    for (size_t i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, CubeMapSize, CubeMapSize, 0, GL_RGB, GL_FLOAT, nullptr);
        //glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, CubeMapSize, CubeMapSize, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    
    float skyboxVertices[] = {
                 
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    VBO vbo;
    VAO vao;

    vao.Bind();
    vbo.Bind();

    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    BindVBONull();
    BindVAONull();

    glm::mat4 Projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

    glm::mat4 fboViews[] =
    {
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };
    
    glUseProgram(HDRItoCubeMapShader);
    glUniformMatrix4fv(glGetUniformLocation(HDRItoCubeMapShader, "ProjectionMat"), 1, GL_FALSE, glm::value_ptr(Projection));

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, HDRItexture);
    glUniform1i(glGetUniformLocation(HDRItoCubeMapShader, "hdriTexture"), 0);

    glViewport(0,0, CubeMapSize, CubeMapSize);
    vao.Bind();

    for (size_t i = 0; i < 6; i++)
    {

        glUniformMatrix4fv(glGetUniformLocation(HDRItoCubeMapShader, "ViewMat"), 1, GL_FALSE, glm::value_ptr(fboViews[i]));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, CubeMapColorAttachment, 0);

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    BindVAONull();
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);

    glDeleteRenderbuffers(1 ,&rbo);
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &HDRItexture);

    return { CubeMapColorAttachment,FF_HDRI_COMPLETE };
}

std::pair<GLuint, int> FUSIONCORE::ConvolutateCubeMap(GLuint CubeMap, GLuint ConvolutateCubeMapShader)
{
    GLuint fbo, rbo, ConvCubeMap;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Frame buffer error :: " << status << "\n";
        return { 0,FF_HDRI_ERROR };
    }

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenTextures(1, &ConvCubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ConvCubeMap);

    for (size_t i = 0; i < 6; i++)
    {

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
       // glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            //0, GL_RGB, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    
    float skyboxVertices[] = {

       -1.0f,  1.0f, -1.0f,
       -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
       -1.0f,  1.0f, -1.0f,

       -1.0f, -1.0f,  1.0f,
       -1.0f, -1.0f, -1.0f,
       -1.0f,  1.0f, -1.0f,
       -1.0f,  1.0f, -1.0f,
       -1.0f,  1.0f,  1.0f,
       -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

       -1.0f, -1.0f,  1.0f,
       -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
       -1.0f, -1.0f,  1.0f,

       -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
       -1.0f,  1.0f,  1.0f,
       -1.0f,  1.0f, -1.0f,

       -1.0f, -1.0f, -1.0f,
       -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
       -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };

    VBO vbo;
    VAO vao;

    vao.Bind();
    vbo.Bind();

    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    BindVBONull();
    BindVAONull();

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    UseShaderProgram(ConvolutateCubeMapShader);
    glViewport(0, 0, 32, 32);
    vao.Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMap);
    glUniform1i(glGetUniformLocation(ConvolutateCubeMapShader, "CubeMap"), 0);

    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glUniformMatrix4fv(glGetUniformLocation(ConvolutateCubeMapShader, "proj"), 1, GL_FALSE, glm::value_ptr(captureProjection));

    glm::mat4 fboViews[] =
    {
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    for (size_t i = 0; i < 6; i++)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, ConvCubeMap, 0);
        glUniformMatrix4fv(glGetUniformLocation(ConvolutateCubeMapShader, "view"), 1, GL_FALSE, glm::value_ptr(fboViews[i]));
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    UseShaderProgram(0);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glDeleteRenderbuffers(1, &rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);

    return {ConvCubeMap, FF_HDRI_COMPLETE};
}

std::pair<GLuint, int> FUSIONCORE::PreFilterCubeMap(GLuint CubeMap, GLuint PreFilterCubeMapShader)
{

    GLuint preFilteredEnvMap , fbo , rbo;

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 128, 128);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Frame buffer error :: " << status << "\n";
        return { 0,FF_HDRI_ERROR };
    }

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    glGenTextures(1, &preFilteredEnvMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, preFilteredEnvMap);

    for (size_t i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    float skyboxVertices[] = {

       -1.0f,  1.0f, -1.0f,
       -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
       -1.0f,  1.0f, -1.0f,

       -1.0f, -1.0f,  1.0f,
       -1.0f, -1.0f, -1.0f,
       -1.0f,  1.0f, -1.0f,
       -1.0f,  1.0f, -1.0f,
       -1.0f,  1.0f,  1.0f,
       -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

       -1.0f, -1.0f,  1.0f,
       -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
       -1.0f, -1.0f,  1.0f,

       -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
       -1.0f,  1.0f,  1.0f,
       -1.0f,  1.0f, -1.0f,

       -1.0f, -1.0f, -1.0f,
       -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
       -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };

    VBO vbo;
    VAO vao;

    vao.Bind();
    vbo.Bind();

    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    BindVBONull();
    BindVAONull();

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    UseShaderProgram(PreFilterCubeMapShader);
    glViewport(0, 0, 32, 32);
    vao.Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMap);
    glUniform1i(glGetUniformLocation(PreFilterCubeMapShader, "CubeMap"), 0);

    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glUniformMatrix4fv(glGetUniformLocation(PreFilterCubeMapShader, "proj"), 1, GL_FALSE, glm::value_ptr(captureProjection));

    glm::mat4 fboViews[] =
    {
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    GLuint maxMipLevels = 5;

    for (size_t m = 0; m < maxMipLevels; m++)
    {
        unsigned int mipWidth = 128 * std::pow(0.5, m);
        unsigned int mipHeight = 128 * std::pow(0.5, m);

        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)m / (float)(maxMipLevels - 1);
        glUniform1f(glGetUniformLocation(PreFilterCubeMapShader, "roughness"), roughness);

        for (size_t i = 0; i < 6; i++)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, preFilteredEnvMap, m);
            glUniformMatrix4fv(glGetUniformLocation(PreFilterCubeMapShader, "view"), 1, GL_FALSE, glm::value_ptr(fboViews[i]));
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }

    UseShaderProgram(0);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glDeleteRenderbuffers(1, &rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);

    return { preFilteredEnvMap , FF_HDRI_COMPLETE };
}

std::pair<GLuint, int> FUSIONCORE::ComputeLUT(Shader& LUTshader)
{
    GLuint fbo, LUT, rbo;

    glGenTextures(1, &LUT);
    glBindTexture(GL_TEXTURE_2D, LUT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, LUT, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Frame buffer error :: " << status << "\n";
        return { 0,FF_HDRI_ERROR };
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

    GLuint quadVAO, quadVBO;

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glUseProgram(LUTshader.GetID());
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, 512, 512);
    glBindVertexArray(quadVAO);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glUseProgram(0);
    //glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glDeleteRenderbuffers(1, &rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);

    glDeleteBuffers(1, &quadVBO);
    glDeleteVertexArrays(1, &quadVAO);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    return { LUT, FF_HDRI_COMPLETE };
}

int FUSIONCORE::ImportCubeMap(const char* HDRIfilePath , unsigned int CubeMapSize , CubeMap& cubemap,FUSIONUTIL::DefaultShaders& shaders)
{
    std::pair<GLuint, int> CubeMapTexture = HDRItoCubeMap(HDRIfilePath, CubeMapSize, shaders.HDRIShader->GetID());

    if (CubeMapTexture.second == FF_HDRI_COMPLETE)
    {
        cubemap.SetConvDiffCubeMap(ConvolutateCubeMap(CubeMapTexture.first, shaders.ConvolutateCubeMapShader->GetID()).first);
        cubemap.SetPreFilteredEnvMap(PreFilterCubeMap(CubeMapTexture.first, shaders.PreFilterCubeMapShader->GetID()).first);
        cubemap.SetCubeMapTexture(CubeMapTexture.first);

        LOG_INF("Imported HDRI :: " + std::string(HDRIfilePath));
    }
    else if (CubeMapTexture.second == FF_HDRI_INCOMPATIBLE_FILE)
    {
        LOG_ERR("Error importing HDRI(Incompatible file extension)! :: " + std::string(HDRIfilePath));
    }
    else if (CubeMapTexture.second == FF_HDRI_ERROR)
    {
        LOG_ERR("Error importing HDRI(Unable to complete the Framebuffer)! :: " + std::string(HDRIfilePath));
    }
    return CubeMapTexture.second;
}


