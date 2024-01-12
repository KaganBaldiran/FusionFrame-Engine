#pragma once
#include <glew.h>
#include <glfw3.h>
#include "../FusionUtility/Log.h"
#include "../FusionUtility/VectorMath.h"
#include "Buffer.h"
#include "Camera.h"
#include "Texture.h"
#include <functional>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Shader.h"
#include "Material.hpp"
#include <queue>
#include <memory>
#include <array>
#include "Cubemap.h"
#define MAX_BONE_INFLUENCE 4
#define FF_ORIGIN glm::vec3(0.0f,0.0f,0.0f)
namespace FUSIONOPENGL
{

	inline glm::vec3 TranslateVertex(glm::mat4 Matrix, glm::vec3 VertexPos)
	{
		glm::vec4 transformed = Matrix * glm::vec4(VertexPos, 1.0f);
		return glm::vec3(transformed.x, transformed.y, transformed.z);
	}

	struct Vertex {

		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
		glm::vec3 Tangent;
		glm::vec3 Bitangent;

		int m_BoneIDs[MAX_BONE_INFLUENCE];
		float m_Weights[MAX_BONE_INFLUENCE];
	};

	struct BoneInfo
	{
		int id;
		glm::mat4 OffsetMat;
	};

	struct TransformAction
	{
		glm::vec3 Transformation;
	};

	struct RotateAction
	{
		float Degree;
		glm::vec3 Vector;
	};

	struct ScaleAction
	{
		glm::vec3 Scale;
	};

	template<std::size_t size>
	class Face
	{
	public:

		Face(Vertex Vertices[size], glm::vec3 Normal = { 0.0f ,0.0f,0.0f})
		{
			this->Normal = Normal;
			this->Vertices.assign(Vertices);
		}

		Face(std::vector<Vertex> Vertices, glm::vec3 Normal = { 0.0f ,0.0f,0.0f })
		{
			this->Normal = Normal;
			std::copy_n(Vertices.begin(), size, this->Vertices.begin());
		}

		Face()
		{
			Normal = glm::vec3(0.0f);
			std::fill_n(Vertices, Vertices.size(), Vertex());
		}

		void SetVertices(Vertex Vertices[size])
		{
			this->Vertices.assign(Vertices, Vertices + size);
		}

		void SetNormal(glm::vec3 Normal)
		{
			this->Normal = Normal;
		}

		void FindNormal()
		{
			Normal = glm::cross((Vertices[1].Position - Vertices[0].Position), (Vertices[2].Position - Vertices[0].Position));
			Normal = glm::normalize(glm::abs(Normal));
		}

		void FindNormal(glm::mat4 ModelMatrix)
		{
			Normal = glm::cross((TranslateVertex(ModelMatrix, Vertices[1].Position) - TranslateVertex(ModelMatrix, Vertices[0].Position)), (TranslateVertex(ModelMatrix, Vertices[2].Position) - TranslateVertex(ModelMatrix, Vertices[0].Position)));
			Normal = glm::normalize(glm::abs(Normal));
		}

		std::array<Vertex, size> GetVertices() { return Vertices; };
		glm::vec3 GetNormal() { return Normal; };
	private:
		glm::vec3 Normal;
		std::array<Vertex, size> Vertices;
	};

	class WorldTransform
	{
	public:

		WorldTransform()
		{
			Position = glm::vec3(0.0f, 0.0f, 0.0f);
			ScaleFactor = glm::vec3(1.0f, 1.0f, 1.0f);
		}

		glm::mat4 TranslationMatrix = glm::mat4(1.0f);
		glm::mat4 RotationMatrix = glm::mat4(1.0f);
		glm::mat4 ScalingMatrix = glm::mat4(1.0f);

		glm::vec3 ObjectScales;
		glm::vec3 InitialObjectScales;

		glm::vec3 Position;
		glm::vec3 *OriginPoint;

		float scale_avg;
		float dynamic_scale_avg;
		std::vector<TransformAction> LastTransforms;
		std::vector<RotateAction> LastRotations;
		std::vector<ScaleAction> LastScales;

		glm::vec3 ScaleFactor;

		void SetModelMatrixUniformLocation(GLuint shader, const char* uniform);
		void Translate(glm::vec3 v);
		void Scale(glm::vec3 v);
		void Rotate(glm::vec3 v, float angle);

		//No transform history for children
		void TranslateNoTraceBack(glm::vec3 v);
		void ScaleNoTraceBack(glm::vec3 v);
		void RotateNoTraceBack(glm::vec3 v, float angle);

		glm::mat4 GetModelMat4() 
		{ 
			return TranslationMatrix * RotationMatrix * ScalingMatrix;
		};
	};

	class Mesh3D
	{
	public:
		Mesh3D(std::vector<FUSIONOPENGL::Vertex>& vertices_i, std::vector<unsigned int>& indices_i , std::vector<Texture2D>& textures_i);
		void Draw(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations);
		void Draw(Camera3D& camera, Shader& shader , Material material, std::function<void()>& ShaderPreperations);
		void Draw(Camera3D& camera, Shader& shader, Material material, std::function<void()>& ShaderPreperations , CubeMap& cubeMap, float EnvironmentAmbientAmount = 0.2f);
		void ConstructMesh();
		std::vector<Vertex>& GetVertexArray() { return vertices; };

		unsigned int VAO;

		void Clean()
		{
			ObjectBuffer.clean();
		}

	private:
		unsigned int VBO, EBO;

		Buffer3D ObjectBuffer;

		std::vector<Texture2D> textures;
		std::vector<unsigned int> indices;
		std::vector<Vertex> vertices;
	};

	class TextureObj
	{
	public:

		TextureObj();
		~TextureObj();

		void Draw(Camera2D& camera, GLuint shader, Texture2D& texture);
		void Draw(Camera3D& camera, GLuint shader, Texture2D& texture, std::function<void()> ShaderPreperations);

		WorldTransform* GetTransformation() { return &this->transformation; };

	protected:
		Buffer ObjectBuffer;
		WorldTransform transformation;

	};
}


