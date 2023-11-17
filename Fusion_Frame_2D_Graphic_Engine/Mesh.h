#pragma once
#include <glew.h>
#include <glfw3.h>
#include "Log.h"
#include "VectorMath.h"
#include "Buffer.h"
#include "Camera.h"
#include "Texture.h"
#include <functional>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Shader.h"
#define MAX_BONE_INFLUENCE 4
namespace FUSIONOPENGL
{
	struct Vertex {

		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
		glm::vec3 Tangent;
		glm::vec3 Bitangent;

		int m_BoneIDs[MAX_BONE_INFLUENCE];
		float m_Weights[MAX_BONE_INFLUENCE];
	};


	class WorldTransform
	{
	public:
		WorldTransform();
		glm::mat4 ModelMatrix;
		glm::vec3 ObjectScales;
		glm::vec3 Position;
		float scale_avg;
		float dynamic_scale_avg;

		void SetModelMatrixUniformLocation(GLuint shader, const char* uniform);
		void Translate(glm::vec3 v);
		void Scale(glm::vec3 v);
		void Rotate(glm::vec3 v, float angle);

		glm::mat4* GetModelMat4() { return &ModelMatrix; };
	};

	class Mesh3D
	{
	public:
		Mesh3D(std::vector<Vertex> vertices_i, std::vector<unsigned int> indices_i);
		void Draw(Camera3D& camera, GLuint shader, std::function<void()> ShaderPreperations);
		/*~Mesh3D()
		{
			glDeleteVertexArrays(1, &VAO);
			glDeleteBuffers(1, &VBO);
			glDeleteBuffers(1, &EBO);
		}*/
		Buffer3D ObjectBuffer;
		std::vector<unsigned int> indices;

		WorldTransform GetTransformation() { return this->transformation; };


	private:

		std::vector<Vertex> vertices;
		std::vector<Texture2D> textures;

		WorldTransform transformation;


		GLuint VBO, EBO, VAO;


	};

	class TextureObj
	{
	public:

		TextureObj();
		~TextureObj();

		void Draw(Camera2D& camera, GLuint shader, Texture2D& texture);
		void Draw(Camera2D& camera, GLuint shader, Texture2D& texture, std::function<void()> ShaderPreperations);
		WorldTransform* GetTransformation() { return &this->transformation; };

	protected:
		Buffer ObjectBuffer;
		WorldTransform transformation;

	};
}


