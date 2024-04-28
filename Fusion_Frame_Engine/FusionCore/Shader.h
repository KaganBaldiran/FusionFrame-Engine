#pragma once
#include <vector>
#include <iostream>
#include <string>
#include "../FusionUtility/VectorMath.h"
#include "../FusionUtility/FusionDLLExport.h"

#ifndef SHADER
#define SHADER 1

#if SHADER

namespace FUSIONCORE
{
    FUSIONFRAME_EXPORT_FUNCTION std::string ReadTextFile(const char* filepath);
    FUSIONFRAME_EXPORT_FUNCTION GLuint CompileVertShader(const char* vertexsource);
    FUSIONFRAME_EXPORT_FUNCTION GLuint CompileFragShader(const char* fragmentsource);
    FUSIONFRAME_EXPORT_FUNCTION GLuint CompileGeoShader(const char* Geosource);
    FUSIONFRAME_EXPORT_FUNCTION GLuint CompileComputeShader(const char* Computesource);
    FUSIONFRAME_EXPORT_FUNCTION GLuint CompileShaderProgram(GLuint ComputeShader);
    FUSIONFRAME_EXPORT_FUNCTION GLuint CompileShaderProgram(GLuint vertexshader, GLuint fragmentshader);
    FUSIONFRAME_EXPORT_FUNCTION GLuint CompileShaderProgram(GLuint vertexshader, GLuint geoshader, GLuint fragmentshader);
    FUSIONFRAME_EXPORT_FUNCTION void UseShaderProgram(GLuint program);
    FUSIONFRAME_EXPORT_FUNCTION void DeleteShaderProgram(GLuint program);

    class FUSIONFRAME_EXPORT Shader
    {
    public:

        Shader(const char* ComputeShaderSourcePath);
        Shader(const char* vertsourcepath, const char* fragsourcepath);
        Shader(const char* vertsourcepath, const char* geosourcepath, const char* fragsourcepath);

        GLuint GetID();

        void use();
        
        // utility uniform functions
        // ------------------------------------------------------------------------
        void setBool(const std::string& name, bool value) const;
        // ------------------------------------------------------------------------
        void setInt(const std::string& name, int value) const;
        // ------------------------------------------------------------------------
        void setFloat(const std::string& name, float value) const;
        // ------------------------------------------------------------------------
        void setVec2(const std::string& name, const glm::vec2& value) const;
        void setVec2(const std::string& name, float x, float y) const;
        // ------------------------------------------------------------------------
        void setVec3(const std::string& name, const glm::vec3& value) const;
        void setVec3(const std::string& name, float x, float y, float z) const;
        // ------------------------------------------------------------------------
        void setVec4(const std::string& name, const glm::vec4& value) const;
        void setVec4(const std::string& name, float x, float y, float z, float w);
        // ------------------------------------------------------------------------
        void setMat2(const std::string& name, const glm::mat2& mat) const;
        // ------------------------------------------------------------------------
        void setMat3(const std::string& name, const glm::mat3& mat) const;
        // ------------------------------------------------------------------------
        void setMat4(const std::string& name, const glm::mat4& mat) const;
    private:

        GLuint shaderID;
    };

}
#endif // INITIALIZE

#endif // !1

