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
#include "Transformation.hpp"

#define MAX_BONE_INFLUENCE 4
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
			for (size_t i = 0; i < size; i++)
			{
				this->Vertices[i] = Vertices[i];
			}
		}

		Face()
		{
			Normal = glm::vec3(0.0f);
			for (size_t i = 0; i < size; i++)
			{
				this->Vertices[i] = Vertex();
			}
		}

		void SetVertices(std::vector<Vertex> Vertices)
		{
			for (size_t i = 0; i < size; i++)
			{
				this->Vertices[i] = Vertices[i];
			}
		}

		void SetNormal(glm::vec3 Normal)
		{
			this->Normal = Normal;
		}

		void FindNormal()
		{
			Normal = glm::cross((Vertices[1].Position - Vertices[0].Position), (Vertices[2].Position - Vertices[0].Position));
			if (glm::length(Normal) < glm::epsilon<float>()) 
			{
				LOG_ERR("Error: Attempting to normalize a zero-length vector.");
			}
			else {
				Normal = glm::normalize(glm::abs(Normal));
			}
		}

		void FindNormal(glm::mat4 ModelMatrix)
		{
			Normal = glm::cross((TranslateVertex(ModelMatrix, Vertices[1].Position) - TranslateVertex(ModelMatrix, Vertices[0].Position)), (TranslateVertex(ModelMatrix, Vertices[2].Position) - TranslateVertex(ModelMatrix, Vertices[0].Position)));
			if (glm::length(Normal) < glm::epsilon<float>()) 
			{
				LOG_ERR("Error: Attempting to normalize a zero-length vector.");
			}
			else {
				Normal = glm::normalize(glm::abs(Normal));
			}
		}

		std::array<Vertex, size> GetVertices() { return Vertices; };
		glm::vec3 GetNormal() { return Normal; };
	private:
		glm::vec3 Normal;
		std::array<Vertex, size> Vertices;
	};

	

	class Mesh3D
	{
	public:
		Mesh3D(std::vector<FUSIONOPENGL::Vertex>& vertices_i, std::vector<unsigned int>& indices_i , std::vector<Texture2D>& textures_i);
		void Draw(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations);
		void Draw(Camera3D& camera, Shader& shader , Material material, std::function<void()>& ShaderPreperations);
		void Draw(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations , CubeMap& cubeMap,Material material, float EnvironmentAmbientAmount = 0.2f);
		void DrawImportedMaterial(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, CubeMap& cubeMap,float EnvironmentAmbientAmount = 0.2f);
		void ConstructMesh();
		std::vector<Vertex>& GetVertexArray() { return vertices; };

		unsigned int VAO;
		Material ImportedMaterial;
		std::string MeshName;

		void Clean()
		{
			ObjectBuffer.clean();

			for (auto it = ImportedMaterial.GetTextureMaps().begin(); it != ImportedMaterial.GetTextureMaps().end(); ++it)
			{
				it->second->Clear();
			}			
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


