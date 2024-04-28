#include "Shader.h"
#include <glew.h>
#include <glfw3.h>
#include <string>
#include <fstream>
#include "../FusionUtility/glm/glm.hpp"
#include "../FusionUtility/glm/gtc/matrix_transform.hpp"
#include "../FusionUtility/glm/gtc/type_ptr.hpp"
#include "../FusionUtility/glm/gtx/rotate_vector.hpp"
#include "../FusionUtility/glm/gtx/vector_angle.hpp"

std::string FUSIONCORE::ReadTextFile(const char* filepath)
{
    std::ifstream inp(filepath, std::ios::binary);

    if (inp)
    {
        std::string texts;
        inp.seekg(0, std::ios::end);
        texts.resize(inp.tellg());
        inp.seekg(0, std::ios::beg);
        inp.read(&texts[0], texts.size());
        inp.close();
        return texts;
    }
    else
    {
        std::cerr << "Error reading the text file :: " << filepath << "\n";
    }
}

GLuint FUSIONCORE::CompileVertShader(const char* vertexsource)
{
    GLuint vertexshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexshader, 1, &vertexsource, nullptr);
    glCompileShader(vertexshader);

    GLint status;
    glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        char buffer[512];
        glGetShaderInfoLog(vertexshader, 512, nullptr, buffer);
        std::cerr << "Failed to compile vertex shader :: " << buffer << "\n";

    }

    return vertexshader;

}

GLuint FUSIONCORE::CompileGeoShader(const char* Geosource)
{
    GLuint Geoshader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(Geoshader, 1, &Geosource, nullptr);
    glCompileShader(Geoshader);

    GLint status;
    glGetShaderiv(Geoshader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        char buffer[512];
        glGetShaderInfoLog(Geoshader, 512, nullptr, buffer);
        std::cerr << "Failed to compile geometry shader :: " << buffer << "\n";

    }

    return Geoshader;
}

GLuint FUSIONCORE::CompileComputeShader(const char* Computesource)
{
    GLuint ComputeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(ComputeShader, 1, &Computesource, nullptr);
    glCompileShader(ComputeShader);

    GLint status;
    glGetShaderiv(ComputeShader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        char buffer[512];
        glGetShaderInfoLog(ComputeShader, 512, nullptr, buffer);
        std::cerr << "Failed to compile compute shader :: " << buffer << "\n";
    }

    return ComputeShader;
}

GLuint FUSIONCORE::CompileShaderProgram(GLuint ComputeShader)
{
    GLuint m_program;
    m_program = glCreateProgram();
    glAttachShader(m_program, ComputeShader);
    glLinkProgram(m_program);

    glDeleteShader(ComputeShader);
  
    GLint status;
    glGetProgramiv(m_program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetProgramInfoLog(m_program, 512, nullptr, buffer);
        std::cerr << "Failed to link program :: " << buffer << "\n";
    }

    return m_program;
}

GLuint FUSIONCORE::CompileShaderProgram(GLuint vertexshader, GLuint geoshader, GLuint fragmentshader)
{
    GLuint m_program;
    m_program = glCreateProgram();
    glAttachShader(m_program, vertexshader);
    glAttachShader(m_program, fragmentshader);
    glAttachShader(m_program, geoshader);
    glLinkProgram(m_program);

    glDeleteShader(vertexshader);
    glDeleteShader(fragmentshader);
    glDeleteShader(geoshader);

    GLint status;
    glGetProgramiv(m_program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetProgramInfoLog(m_program, 512, nullptr, buffer);
        std::cerr << "Failed to link program :: " << buffer << "\n";
    }

    return m_program;
}

GLuint FUSIONCORE::CompileFragShader(const char* fragmentsource)
{
    GLuint fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentshader, 1, &fragmentsource, nullptr);
    glCompileShader(fragmentshader);

    GLint status;
    glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        char buffer[512];
        glGetShaderInfoLog(fragmentshader, 512, nullptr, buffer);
        std::cerr<< "Failed to compile fragment shader :: " << buffer << "\n";

    }

    return fragmentshader;
}

GLuint FUSIONCORE::CompileShaderProgram(GLuint vertexshader , GLuint fragmentshader)
{
    GLuint m_program;
    m_program = glCreateProgram();
    glAttachShader(m_program, vertexshader);
    glAttachShader(m_program, fragmentshader);
    glLinkProgram(m_program);

    glDeleteShader(vertexshader);
    glDeleteShader(fragmentshader);

    GLint status;
    glGetProgramiv(m_program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetProgramInfoLog(m_program, 512, nullptr, buffer);
        std::cerr << "Failed to link program :: " << buffer << "\n";
    }
    return m_program;
}

void FUSIONCORE::UseShaderProgram(GLuint program)
{
    glUseProgram(program);
}

void FUSIONCORE::DeleteShaderProgram(GLuint program)
{
    glDeleteProgram(program);
}

FUSIONCORE::Shader::Shader(const char* ComputeShaderSourcePath)
{
    std::string ComputeSource = ReadTextFile(ComputeShaderSourcePath);

    GLuint ComputeShader = CompileComputeShader(ComputeSource.c_str());
    shaderID = CompileShaderProgram(ComputeShader);
}

FUSIONCORE::Shader::Shader(const char* vertsourcepath, const char* fragsourcepath)
{
    std::string vertsource = ReadTextFile(vertsourcepath);
    std::string fragsource = ReadTextFile(fragsourcepath);

    GLuint vertexshader = CompileVertShader(vertsource.c_str());
    GLuint fragmentshader = CompileFragShader(fragsource.c_str());
    shaderID = CompileShaderProgram(vertexshader, fragmentshader);
}

FUSIONCORE::Shader::Shader(const char* vertsourcepath, const char* geosourcepath, const char* fragsourcepath)
{
    std::string vertsource = ReadTextFile(vertsourcepath);
    std::string geosource = ReadTextFile(geosourcepath);
    std::string fragsource = ReadTextFile(fragsourcepath);

    GLuint vertexshader = CompileVertShader(vertsource.c_str());
    GLuint fragmentshader = CompileFragShader(fragsource.c_str());
    GLuint geoshader = CompileGeoShader(geosource.c_str());
    shaderID = CompileShaderProgram(vertexshader,geoshader, fragmentshader);
}

GLuint FUSIONCORE::Shader::GetID()
{
    return shaderID;
}

void FUSIONCORE::Shader::use()
{
    glUseProgram(shaderID);
}

void FUSIONCORE::Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(shaderID, name.c_str()), (int)value);
}

void FUSIONCORE::Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(shaderID, name.c_str()), value);
}

void FUSIONCORE::Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(shaderID, name.c_str()), value);
}

void FUSIONCORE::Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
    glUniform2fv(glGetUniformLocation(shaderID, name.c_str()), 1, &value[0]);
}

void FUSIONCORE::Shader::setVec2(const std::string& name, float x, float y) const
{
    glUniform2f(glGetUniformLocation(shaderID, name.c_str()), x, y);
}

void FUSIONCORE::Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(glGetUniformLocation(shaderID, name.c_str()), 1, &value[0]);
}

void FUSIONCORE::Shader::setVec3(const std::string& name, float x, float y, float z) const
{
    glUniform3f(glGetUniformLocation(shaderID, name.c_str()), x, y, z);
}

void FUSIONCORE::Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
    glUniform4fv(glGetUniformLocation(shaderID, name.c_str()), 1, &value[0]);
}

void FUSIONCORE::Shader::setVec4(const std::string& name, float x, float y, float z, float w)
{
    glUniform4f(glGetUniformLocation(shaderID, name.c_str()), x, y, z, w);
}

void FUSIONCORE::Shader::setMat2(const std::string& name, const glm::mat2& mat) const
{
    glUniformMatrix2fv(glGetUniformLocation(shaderID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void FUSIONCORE::Shader::setMat3(const std::string& name, const glm::mat3& mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(shaderID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void FUSIONCORE::Shader::setMat4(const std::string& name, const glm::mat4& mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(shaderID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
