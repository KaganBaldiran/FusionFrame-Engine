#include "Model.hpp"

FUSIONOPENGL::Model::Model(const char* filePath)
{
	LoadModel(filePath);
	static unsigned int counter = 0;
	this->ModelID = counter;
	counter++;
}

void FUSIONOPENGL::Model::LoadModel(const char* filePath)
{
	this->path = filePath;

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		LOG_ERR("ASSIMP:: " << importer.GetErrorString());
		return;
	}

	ProcessNode(scene->mRootNode, scene);
}

void FUSIONOPENGL::Model::ProcessNode(aiNode* node, const aiScene* scene)
{
	for (size_t i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		Meshes.push_back(ProcessMesh(mesh, scene));
	}

	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}

}

Mesh3D FUSIONOPENGL::Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> Vertices;
	std::vector<unsigned int> Indices;
	std::vector<Texture2D> Textures;


	for (size_t i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		glm::vec3 vector;

		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;

		if (mesh->HasNormals())
		{
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
		    vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;
		}

		if (mesh->mTextureCoords[0])
		{
			glm::vec2 Vec;

			Vec.x = mesh->mTextureCoords[0][i].x;
			Vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = Vec;

			vector.x = mesh->mTangents[i].x;
			vector.y = mesh->mTangents[i].y;
			vector.z = mesh->mTangents[i].z;
			vertex.Tangent = vector;
		    
			vector.x = mesh->mBitangents[i].x;
			vector.y = mesh->mBitangents[i].y;
			vector.z = mesh->mBitangents[i].z;
			vertex.Bitangent = vector;

		}
		else
		{
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		}

		Vertices.push_back(vertex);
	}

	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		for (size_t j = 0; j < face.mNumIndices; j++)
		{
			Indices.push_back(face.mIndices[j]);
		}
	}

	Mesh3D newMesh(Vertices, Indices);

	return newMesh;
}
