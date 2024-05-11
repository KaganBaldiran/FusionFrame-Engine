#include "Shader.h"
#include <fstream>
#include <sstream>
#include <glew.h>
#include <glfw3.h>
#include <string>
#include <fstream>
#include "../FusionUtility/glm/glm.hpp"
#include "../FusionUtility/glm/gtc/matrix_transform.hpp"
#include "../FusionUtility/glm/gtc/type_ptr.hpp"
#include "../FusionUtility/glm/gtx/rotate_vector.hpp"
#include "../FusionUtility/glm/gtx/vector_angle.hpp"
#include "../FusionUtility/Log.h"

std::string FUSIONCORE::ReadTextFile(const char* filepath)
{
	std::ifstream InputFile(filepath, std::ios::binary);
	std::stringstream buffer;
	if (InputFile.is_open())
	{
		buffer << InputFile.rdbuf();
		InputFile.close();
	}
	else
	{
		LOG_ERR("Unable to open file!");
	}
	return buffer.str();
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
		std::cerr << "Failed to compile fragment shader :: " << buffer << "\n";

	}

	return fragmentshader;
}

GLuint FUSIONCORE::CompileShaderProgram(GLuint vertexshader, GLuint fragmentshader)
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
	ShaderSources[FF_COMPUTE_SHADER_SOURCE] = ComputeSource;

	GLuint ComputeShader = CompileComputeShader(ComputeSource.c_str());
	shaderID = CompileShaderProgram(ComputeShader);
}

FUSIONCORE::Shader::Shader(const char* vertsourcepath, const char* fragsourcepath)
{
	std::string vertsource = ReadTextFile(vertsourcepath);
	std::string fragsource = ReadTextFile(fragsourcepath);
	ShaderSources[FF_VERTEX_SHADER_SOURCE] = vertsource;
	ShaderSources[FF_FRAGMENT_SHADER_SOURCE] = fragsource;

	GLuint vertexshader = CompileVertShader(vertsource.c_str());
	GLuint fragmentshader = CompileFragShader(fragsource.c_str());
	shaderID = CompileShaderProgram(vertexshader, fragmentshader);
}

FUSIONCORE::Shader::Shader(const char* vertsourcepath, const char* geosourcepath, const char* fragsourcepath)
{
	std::string vertsource = ReadTextFile(vertsourcepath);
	std::string geosource = ReadTextFile(geosourcepath);
	std::string fragsource = ReadTextFile(fragsourcepath);
	ShaderSources[FF_VERTEX_SHADER_SOURCE] = vertsource;
	ShaderSources[FF_FRAGMENT_SHADER_SOURCE] = fragsource;
	ShaderSources[FF_GEOMETRY_SHADER_SOURCE] = geosource;

	GLuint vertexshader = CompileVertShader(vertsource.c_str());
	GLuint fragmentshader = CompileFragShader(fragsource.c_str());
	GLuint geoshader = CompileGeoShader(geosource.c_str());
	shaderID = CompileShaderProgram(vertexshader, geoshader, fragmentshader);
}

void FUSIONCORE::Shader::Compile(const char* ComputeShaderSourcePath)
{
	std::string ComputeSource = ReadTextFile(ComputeShaderSourcePath);

	GLuint ComputeShader = CompileComputeShader(ComputeSource.c_str());
	shaderID = CompileShaderProgram(ComputeShader);
}

void FUSIONCORE::Shader::Compile(const char* vertsourcepath, const char* fragsourcepath)
{
	std::string vertsource = ReadTextFile(vertsourcepath);
	std::string fragsource = ReadTextFile(fragsourcepath);

	GLuint vertexshader = CompileVertShader(vertsource.c_str());
	GLuint fragmentshader = CompileFragShader(fragsource.c_str());
	shaderID = CompileShaderProgram(vertexshader, fragmentshader);
}

void FUSIONCORE::Shader::Compile(const char* vertsourcepath, const char* geosourcepath, const char* fragsourcepath)
{
	std::string vertsource = ReadTextFile(vertsourcepath);
	std::string geosource = ReadTextFile(geosourcepath);
	std::string fragsource = ReadTextFile(fragsourcepath);

	GLuint vertexshader = CompileVertShader(vertsource.c_str());
	GLuint fragmentshader = CompileFragShader(fragsource.c_str());
	GLuint geoshader = CompileGeoShader(geosource.c_str());
	shaderID = CompileShaderProgram(vertexshader, geoshader, fragmentshader);
}

void FUSIONCORE::Shader::Compile(std::string ComputeShaderSource)
{
	GLuint ComputeShader = CompileComputeShader(ComputeShaderSource.c_str());
	shaderID = CompileShaderProgram(ComputeShader);
}

void FUSIONCORE::Shader::Compile(std::string VertexShaderSource, std::string FragmentShaderSource)
{
	GLuint vertexshader = CompileVertShader(VertexShaderSource.c_str());
	GLuint fragmentshader = CompileFragShader(FragmentShaderSource.c_str());
	shaderID = CompileShaderProgram(vertexshader, fragmentshader);
}

void FUSIONCORE::Shader::Compile(std::string VertexShaderSource, std::string GeometryShaderSource, std::string FragmentShaderSource)
{
	GLuint vertexshader = CompileVertShader(VertexShaderSource.c_str());
	GLuint fragmentshader = CompileFragShader(FragmentShaderSource.c_str());
	GLuint geoshader = CompileGeoShader(GeometryShaderSource.c_str());
	shaderID = CompileShaderProgram(vertexshader, geoshader, fragmentshader);
}

void FUSIONCORE::Shader::PushShaderSource(FF_SHADER_SOURCE Usage, const char* ShaderSourcePath)
{
	this->ShaderSources[Usage] = ReadTextFile(ShaderSourcePath);
}

std::string FUSIONCORE::Shader::GetShaderSource(FF_SHADER_SOURCE Usage)
{
	if (ShaderSources.find(Usage) != ShaderSources.end())
	{
		return this->ShaderSources[Usage];
	}
	LOG_ERR("Invalid shader usage! Make sure its one of the four core usages : \n"
		"FF_VERTEX_SHADER_SOURCE,\n"
		"FF_FRAGMENT_SHADER_SOURCE,\n"
		"FF_COMPUTE_SHADER_SOURCE,\n"
		"FF_GEOMETRY_SHADER_SOURCE");
}

size_t FindSubString(std::string Source, std::string SubString)
{
	size_t SubstringSize = SubString.size();
	for (size_t i = 0; i < Source.size() - SubstringSize - 1; i++)
	{
		if (Source.substr(i, i + SubstringSize) == SubString)
		{
			return i;
		}
	}
	return std::string::npos;
}

void FUSIONCORE::Shader::AlterShaderUniformArrayValue(FF_SHADER_SOURCE ShaderUsage, std::string uniformName, int ArrayCount)
{
	if (ShaderSources.find(ShaderUsage) != ShaderSources.end())
	{
		std::string& Source = ShaderSources[ShaderUsage];
		size_t UniformPos = Source.find(uniformName);
		std::string ArrayCountStr = std::to_string(ArrayCount);
		if (UniformPos != std::string::npos)
		{
			auto LeftBracetPos = Source.find("[", UniformPos);
			auto RightBracetPos = Source.find("]", UniformPos);

			if (LeftBracetPos != std::string::npos && RightBracetPos != std::string::npos)
			{
				if (ArrayCountStr.size() == (RightBracetPos - LeftBracetPos))
				{
					for (size_t i = 0; i < ArrayCountStr.size(); i++)
					{
						Source[LeftBracetPos + 1 + i] = ArrayCountStr[i];
					}
				}
				else
				{
					for (size_t i = LeftBracetPos + 1; i < RightBracetPos; i++)
					{
						Source.erase(Source.begin() + LeftBracetPos + 1);
					}
					Source.insert(LeftBracetPos + 1, ArrayCountStr);
				}
			}
			else
			{
				LOG_ERR("Specified uniform is not an array type!");
			}
		}
		else
		{
			LOG_ERR("Specified uniform name is not found in the shader source!");
		}
	}
	else
	{
		LOG_ERR("Invalid shader usage! Make sure its one of the four core usages : \n"
			    "FF_VERTEX_SHADER_SOURCE,\n"
			    "FF_FRAGMENT_SHADER_SOURCE,\n"
			    "FF_COMPUTE_SHADER_SOURCE,\n"
		        "FF_GEOMETRY_SHADER_SOURCE");
	}
}

void FUSIONCORE::Shader::AlterShaderMacroDefinitionValue(FF_SHADER_SOURCE ShaderUsage, std::string MacroName, std::string Value)
{
	if(ShaderSources.find(ShaderUsage) != ShaderSources.end())
	{
		std::string& Source = ShaderSources[ShaderUsage];
		size_t MacroPos = Source.find(MacroName);
		size_t ValueStrLength = Value.size();
		LOG("SourceSIZE: " << Source.size() << " MacroPos " << MacroName << ": " << MacroPos);
		if (MacroPos != std::string::npos)
		{
			size_t EndOfLinePos = Source.find("\n" , MacroPos);

			LOG("EndOfLinePos: " << EndOfLinePos);
			if (EndOfLinePos != std::string::npos)
			{
				size_t ValueBeginningPos = MacroPos + MacroName.size() + 1;
				LOG("ValueBeginningPos: " << ValueBeginningPos);
				if (ValueStrLength == (EndOfLinePos - ValueBeginningPos))
				{
					for (size_t i = 0; i < ValueStrLength; i++)
					{
						Source[ValueBeginningPos + i] = Value[i];
					}
				}
				else
				{
					for (size_t i = ValueBeginningPos; i < EndOfLinePos; i++)
					{
						Source.erase(Source.begin() + ValueBeginningPos);
					}
					Source.insert(ValueBeginningPos, Value);
				}
			}
			else
			{
				LOG_ERR("Specified uniform is not an array type!");
			}
		}
		else
		{
			LOG_ERR("Specified macro name is not found in the shader source!");
		}
	}
	else
	{
		LOG_ERR("Invalid shader usage! Make sure its one of the four core usages : \n"
			"FF_VERTEX_SHADER_SOURCE,\n"
			"FF_FRAGMENT_SHADER_SOURCE,\n"
			"FF_COMPUTE_SHADER_SOURCE,\n"
			"FF_GEOMETRY_SHADER_SOURCE");
	}
}

void FUSIONCORE::Shader::AlterShaderLayoutQualifierValue(FF_SHADER_SOURCE ShaderUsage, std::string MacroName, std::string Value)
{
}

void FUSIONCORE::Shader::Compile()
{
	auto VertexShaderFound = ShaderSources.find(FF_VERTEX_SHADER_SOURCE) != ShaderSources.end();
	auto FragmentShaderFound = ShaderSources.find(FF_FRAGMENT_SHADER_SOURCE) != ShaderSources.end();

	if (VertexShaderFound || FragmentShaderFound)
	{
		if (!VertexShaderFound)
		{
			auto ErrorMessage = "Shader compilation error :: Vertex shader source is missing!";
			LOG_ERR(ErrorMessage);
			throw FFexception(ErrorMessage);
		}
		if (!FragmentShaderFound)
		{
			auto ErrorMessage = "Shader compilation error :: Fragment shader source is missing!";
			LOG_ERR(ErrorMessage);
			throw FFexception(ErrorMessage);
		}

		if (ShaderSources.find(FF_GEOMETRY_SHADER_SOURCE) != ShaderSources.end())
		{
			Compile(ShaderSources[FF_VERTEX_SHADER_SOURCE], ShaderSources[FF_FRAGMENT_SHADER_SOURCE], ShaderSources[FF_GEOMETRY_SHADER_SOURCE]);
		}
		else
		{
			Compile(ShaderSources[FF_VERTEX_SHADER_SOURCE], ShaderSources[FF_FRAGMENT_SHADER_SOURCE]);
		}
	}
	else if (ShaderSources.find(FF_COMPUTE_SHADER_SOURCE) != ShaderSources.end())
	{
		Compile(ShaderSources[FF_COMPUTE_SHADER_SOURCE]);
	}
	else
	{
		auto ErrorMessage = "Shader compilation error :: No appropriate shader source found!";
		LOG_ERR(ErrorMessage);
		throw FFexception(ErrorMessage);
	}
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
