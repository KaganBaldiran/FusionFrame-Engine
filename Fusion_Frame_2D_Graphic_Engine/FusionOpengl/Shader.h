#pragma once
#include <glew.h>
#include <vector>
#include <iostream>
#include <string>
#include "../FusionUtility/glm/glm.hpp"
#include "../FusionUtility/glm/gtc/matrix_transform.hpp"
#include "../FusionUtility/glm/gtc/type_ptr.hpp"
#include "../FusionUtility/glm/gtx/rotate_vector.hpp"
#include "../FusionUtility/glm/gtx/vector_angle.hpp"

typedef float GLfloat;
typedef int GLint;
typedef unsigned int GLuint;

#ifndef SHADER
#define SHADER 1

#if SHADER

namespace FUSIONOPENGL
{
    std::string ReadTextFile(const char* filepath);
    GLuint CompileVertShader(const char* vertexsource);
    GLuint CompileFragShader(const char* fragmentsource);
    GLuint CompileShaderProgram(GLuint vertexshader, GLuint fragmentshader);
    void UseShaderProgram(GLuint program);
    void DeleteShaderProgram(GLuint program);


    class Shader
    {
    public:

        Shader(const char* vertsource, const char* fragsource);

        GLuint GetID();

        void use()
        {
            glUseProgram(shaderID);
        }
        // utility uniform functions
        // ------------------------------------------------------------------------
        void setBool(const std::string& name, bool value) const
        {
            glUniform1i(glGetUniformLocation(shaderID, name.c_str()), (int)value);
        }
        // ------------------------------------------------------------------------
        void setInt(const std::string& name, int value) const
        {
            glUniform1i(glGetUniformLocation(shaderID, name.c_str()), value);
        }
        // ------------------------------------------------------------------------
        void setFloat(const std::string& name, float value) const
        {
            glUniform1f(glGetUniformLocation(shaderID, name.c_str()), value);
        }
        // ------------------------------------------------------------------------
        void setVec2(const std::string& name, const glm::vec2& value) const
        {
            glUniform2fv(glGetUniformLocation(shaderID, name.c_str()), 1, &value[0]);
        }
        void setVec2(const std::string& name, float x, float y) const
        {
            glUniform2f(glGetUniformLocation(shaderID, name.c_str()), x, y);
        }
        // ------------------------------------------------------------------------
        void setVec3(const std::string& name, const glm::vec3& value) const
        {
            glUniform3fv(glGetUniformLocation(shaderID, name.c_str()), 1, &value[0]);
        }
        void setVec3(const std::string& name, float x, float y, float z) const
        {
            glUniform3f(glGetUniformLocation(shaderID, name.c_str()), x, y, z);
        }
        // ------------------------------------------------------------------------
        void setVec4(const std::string& name, const glm::vec4& value) const
        {
            glUniform4fv(glGetUniformLocation(shaderID, name.c_str()), 1, &value[0]);
        }
        void setVec4(const std::string& name, float x, float y, float z, float w)
        {
            glUniform4f(glGetUniformLocation(shaderID, name.c_str()), x, y, z, w);
        }
        // ------------------------------------------------------------------------
        void setMat2(const std::string& name, const glm::mat2& mat) const
        {
            glUniformMatrix2fv(glGetUniformLocation(shaderID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }
        // ------------------------------------------------------------------------
        void setMat3(const std::string& name, const glm::mat3& mat) const
        {
            glUniformMatrix3fv(glGetUniformLocation(shaderID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }
        // ------------------------------------------------------------------------
        void setMat4(const std::string& name, const glm::mat4& mat) const
        {
            glUniformMatrix4fv(glGetUniformLocation(shaderID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }

    private:

        GLuint shaderID;
    };

}
#endif // INITIALIZE

#endif // !1

