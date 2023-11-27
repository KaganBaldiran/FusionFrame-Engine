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
#include "Material.hpp"
#include <queue>
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

	class WorldTransform
	{
	public:

		WorldTransform()
		{
			Position = glm::vec3(0.0f, 0.0f, 0.0f);
		}

		glm::mat4 ModelMatrix = glm::mat4(1.0f);
		glm::vec3 ObjectScales;
		glm::vec3 Position;
		float scale_avg;
		float dynamic_scale_avg;
		std::vector<TransformAction> LastTransforms;
		std::vector<RotateAction> LastRotations;
		std::vector<ScaleAction> LastScales;

		void SetModelMatrixUniformLocation(GLuint shader, const char* uniform);
		void Translate(glm::vec3 v);
		void Scale(glm::vec3 v);
		void Rotate(glm::vec3 v, float angle);

		//No transform history for children
		void TranslateNoTraceBack(glm::vec3 v);
		void ScaleNoTraceBack(glm::vec3 v);
		void RotateNoTraceBack(glm::vec3 v, float angle);

		glm::mat4* GetModelMat4() { return &ModelMatrix; };
	};

	class Mesh3D
	{
	public:

		Mesh3D(std::vector<FUSIONOPENGL::Vertex>& vertices_i, std::vector<unsigned int>& indices_i , std::vector<Texture2D>& textures_i);
		void Draw(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations);
		void Draw(Camera3D& camera, Shader& shader , Material& material, std::function<void()>& ShaderPreperations);
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


