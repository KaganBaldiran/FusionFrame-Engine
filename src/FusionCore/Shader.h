#pragma once
#include <vector>
#include <iostream>
#include <string>
#include "../FusionUtility/VectorMath.h"
#include "../FusionUtility/FusionDLLExport.h"
#include <unordered_map>
#include <filesystem>

#ifndef SHADER
#define SHADER 1

#define FF_ALTERED 0x24001
#define FF_NO_LONGER_EXIST 0x24002
#define FF_NOT_ALTERED 0x24003

#if SHADER

namespace FUSIONCORE
{
    enum FUSIONFRAME_EXPORT FF_SHADER_SOURCE {
        FF_GEOMETRY_SHADER_SOURCE = 0x9001,
        FF_VERTEX_SHADER_SOURCE = 0x9002,
        FF_FRAGMENT_SHADER_SOURCE = 0x9003,
        FF_COMPUTE_SHADER_SOURCE = 0x9004
    };

    class FUSIONFRAME_EXPORT ShaderData
    {
    public:
        ShaderData(const ShaderData&) = default;
        ShaderData() = default;
        ShaderData(const FF_SHADER_SOURCE& ShaderType, const std::filesystem::file_time_type& LastModifed,
            const std::string& FilePath = std::string(), const std::string& SourceData = std::string()) : ShaderType(ShaderType), LastModified(LastModifed),
                                                                          FilePath(FilePath), SourceData(SourceData)
        {}
        ~ShaderData() = default; 
        FF_SHADER_SOURCE ShaderType;
        std::filesystem::file_time_type LastModified;
        std::string FilePath;
        std::string SourceData;
    };

    enum FUSIONFRAME_EXPORT FF_SHADER_LAYOUT_QUALIFIER {
        FF_LOCATION_SHADER_LAYOUT_QUALIFIER = 0x10001,
        FF_INVOCATION_SHADER_LAYOUT_QUALIFIER = 0x10002,
        FF_BINDING_SHADER_LAYOUT_QUALIFIER = 0x10003,
        FF_MEMORY_LAYOUT_SHADER_LAYOUT_QUALIFIER = 0x10004,
        FF_MAX_TRIANGLE_SHADER_LAYOUT_QUALIFIER = 0x10005
    };

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

     /*
     Manages shader programs used for rendering.

     The Shader class provides functionalities for creating, compiling, and managing shader programs
     used in rendering. It supports different types of shaders, including vertex, geometry, and fragment shaders,
     and allows for deferred compilation and alteration of shader code.

     Key functionalities include:
     - Compiling shader programs from source files or strings.
     - Deferred compilation to support altering shader code before final compilation.
     - Setting uniform values such as bools, ints, floats, vectors, and matrices.

     Example usage:
     // Create a shader program from vertex and fragment shader source files
     Shader myShader("vertex_shader.vert", "fragment_shader.frag");

     // Set uniform values
     myShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
     myShader.setFloat("ambientIntensity", 0.5f);

     // Use the shader program for rendering
     myShader.use();
    */
    class FUSIONFRAME_EXPORT Shader
    {
    public:

        Shader() = default;
        Shader(const char* ComputeShaderSourcePath);
        Shader(const char* vertsourcepath, const char* fragsourcepath);
        Shader(const char* vertsourcepath, const char* geosourcepath, const char* fragsourcepath);

        void Clear();

        void Compile(const char* ComputeShaderSourcePath);
        void Compile(const char* vertsourcepath, const char* fragsourcepath);
        void Compile(const char* vertsourcepath, const char* geosourcepath, const char* fragsourcepath);
        void Compile(std::string ComputeShaderSource);
        void Compile(std::string VertexShaderSource,std::string FragmentShaderSource);
        void Compile(std::string VertexShaderSource,std::string GeometryShaderSource,std::string FragmentShaderSource);

        //Recompiles the shader in case any of the source files is altered.
        void HotReload(int CheckGapInMiliseconds);
        bool IsRecompiled();

        void PushShaderSource(FF_SHADER_SOURCE Usage,const char* ShaderSourcePath);
        ShaderData GetShaderSource(FF_SHADER_SOURCE Usage);

        void AlterShaderUniformArrayValue(FF_SHADER_SOURCE ShaderUsage, std::string uniformName, int ArrayCount);
        void AlterShaderMacroDefinitionValue(FF_SHADER_SOURCE ShaderUsage, std::string MacroName,std::string Value);
        void AlterShaderLayoutQualifierValue(FF_SHADER_SOURCE ShaderUsage, std::string MacroName, std::string Value);

        //Used to compile the shader once the shader sources are pushed using "PushShaderSource" function.
        //Main purpose of this defered compilation is to change shader code using shader altering functions.
        void Compile();

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

        std::chrono::steady_clock::time_point LastTimeChecked;
        GLuint shaderID;
        std::unordered_map<int, ShaderData> ShaderDatas;
        bool IsRecompiledFlag;
    };

}
#endif // INITIALIZE

#endif // !1

