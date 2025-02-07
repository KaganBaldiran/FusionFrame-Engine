#pragma once
#include "../FusionUtility/FusionDLLExport.h"
#include "../FusionUtility/VectorMath.h"
#include "../FusionUtility/Definitions.hpp"

namespace FUSIONCORE
{
	class FUSIONFRAME_EXPORT Buffer
	{
	public:
		Buffer();
		void clean();

		void Bind();
		void BindVAO();
		void UnbindVAO();
		void Unbind();
		void BufferDataFill(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
		void AttribPointer(GLuint index, GLuint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
		void AttribIPointer(GLuint index, GLuint size, GLenum type, GLsizei stride, const void* pointer);
		GLuint vao, vbo;
	};

	class FUSIONFRAME_EXPORT Buffer3D : public Buffer
	{
	public:
		Buffer3D();
		void clean();
		void BindEBO();
		void Unbind();
	private:

		GLuint ebo;
	};

	class FUSIONFRAME_EXPORT VBO
	{
	public:
		VBO();
		GLuint Bind();
		GLuint GetBufferID();
		inline void SetVBOstate(bool IsChanged) { this->IsChanged = IsChanged; };
		inline const bool IsVBOchanged() { return this->IsChanged; };
		~VBO();
	private:
		GLuint vbo;
		bool IsChanged;
	};

	class FUSIONFRAME_EXPORT VAO
	{
	public:

		VAO();
		GLuint Bind();
		GLuint GetVertexArrID();
		~VAO();
	private:
		GLuint vao;
	};

	class FUSIONFRAME_EXPORT UBO
	{
	public:
		UBO();
		GLuint Bind();
		void BindUBO(unsigned int BindingPoint);
		GLuint GetUBOID();
		void BufferDataFill(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
		~UBO();

	private:
		GLuint ubo;
	};

	class FUSIONFRAME_EXPORT EBO
	{
	public:

		EBO();
		GLuint Bind();
		GLuint GetEBOID();
		~EBO();

	private:

		GLuint ebo;

	};

	class FUSIONFRAME_EXPORT SSBO
	{
	public:
		void BindSSBO(unsigned int BindingPoint);
		SSBO();
		~SSBO();
		void clean();

		void Bind();
		void Unbind();
		void BufferDataFill(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
		void BufferSubDataFill(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
	private:
		GLuint ssbo;
	};

	class FUSIONFRAME_EXPORT TBO
	{
	public:
		void TexBuffer(GLenum InternalFormat);
		TBO();
		~TBO();
		void clean();

		void Bind();
		void Unbind();
		void BufferDataFill(GLsizeiptr size, const void* data, GLenum usage);
	private:
		GLuint tbo;
	};

	struct FUSIONFRAME_EXPORT DrawArraysIndirectCommand {
		GLuint Count;
		GLuint InstanceCount;
		GLuint First;
		GLuint BaseInstance;
	};

	struct FUSIONFRAME_EXPORT DrawElementsIndirectCommand {
		GLuint Count;
		GLuint InstanceCount;
		GLuint FirstIndex;
		GLuint BaseVertex;
		GLuint BaseInstance;
	};

	class FUSIONFRAME_EXPORT IndirectCommandBuffer
	{
	public:
		IndirectCommandBuffer();
		GLuint Bind();
		GLuint GetBufferID();
		~IndirectCommandBuffer();
	private:
		GLuint icb;
	};

	FUSIONFRAME_EXPORT_FUNCTION void BindVBONull();
	FUSIONFRAME_EXPORT_FUNCTION void BindVAONull();
	FUSIONFRAME_EXPORT_FUNCTION void BindEBONull();
	FUSIONFRAME_EXPORT_FUNCTION void BindUBONull();
	FUSIONFRAME_EXPORT_FUNCTION void BindSSBONull();
	FUSIONFRAME_EXPORT_FUNCTION void BindTBONull();
	FUSIONFRAME_EXPORT_FUNCTION void BindIndirectCommandBufferNull();

	//3D cube buffer
	FUSIONFRAME_EXPORT_FUNCTION VAO* GetCubeBuffer();
	//2D rectangle buffer
	FUSIONFRAME_EXPORT_FUNCTION VAO* GetRectangleBuffer();

	//Internally called, no need to call 
	FUSIONFRAME_EXPORT_FUNCTION void InitializeBuffers();
}
