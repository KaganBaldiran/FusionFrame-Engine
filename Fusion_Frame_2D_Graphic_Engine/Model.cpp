#include "Model.hpp"

FUSIONOPENGL::Model::Model(std::string filePath)
{
	LoadModel(filePath);
	static unsigned int counter = 0;
	this->ModelID = counter;
	counter++;
}

void FUSIONOPENGL::Model::Draw(Camera3D& camera, GLuint shader, std::function<void()> ShaderPreperations)
{
	for (size_t i = 0; i < Meshes.size(); i++)
	{
		Meshes[i].Draw(camera, shader, ShaderPreperations);
	}
}


void FUSIONOPENGL::Model::LoadModel(std::string path)
{
    this->path = path;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }


    std::vector<glm::vec3> originPoints;
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[i];


        float minX = mesh->mVertices[0].x, minY = mesh->mVertices[0].y, minZ = mesh->mVertices[0].z;
        float maxX = mesh->mVertices[0].x, maxY = mesh->mVertices[0].y, maxZ = mesh->mVertices[0].z;
        for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
            if (mesh->mVertices[j].x < minX) minX = mesh->mVertices[j].x;
            if (mesh->mVertices[j].y < minY) minY = mesh->mVertices[j].y;
            if (mesh->mVertices[j].z < minZ) minZ = mesh->mVertices[j].z;
            if (mesh->mVertices[j].x > maxX) maxX = mesh->mVertices[j].x;
            if (mesh->mVertices[j].y > maxY) maxY = mesh->mVertices[j].y;
            if (mesh->mVertices[j].z > maxZ) maxZ = mesh->mVertices[j].z;
        }
        float centerX = (minX + maxX) / 2.0f;
        float centerY = (minY + maxY) / 2.0f;
        float centerZ = (minZ + maxZ) / 2.0f;

        originPoints.push_back(glm::vec3(centerX, centerY, centerZ));
    }

    // Compute overall origin point
    float overallCenterX = 0.0f, overallCenterY = 0.0f, overallCenterZ = 0.0f;
    for (unsigned int i = 0; i < originPoints.size(); i++) {
        overallCenterX += originPoints[i].x;
        overallCenterY += originPoints[i].y;
        overallCenterZ += originPoints[i].z;
    }
    overallCenterX /= originPoints.size();
    overallCenterY /= originPoints.size();
    overallCenterZ /= originPoints.size();

    //originpoint = { overallCenterX,overallCenterY,overallCenterZ };

    //dynamic_origin = glm::vec3(overallCenterX, overallCenterY, overallCenterZ);

    std::cout << "Overall origin point: (" << overallCenterX << ", " << overallCenterY << ", " << overallCenterZ << ")" << std::endl;


    // retrieve the directory path of the filepath



    ProcessNode(scene->mRootNode, scene);
}


void FUSIONOPENGL::Model::ProcessNode(aiNode* node, const aiScene* scene)
{

    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		LOG("MESH" << std::to_string(i) << " : " << mesh->mName.C_Str());
        this->Meshes.push_back(ProcessMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
		ProcessNode(node->mChildren[i], scene);
	}

}

FUSIONOPENGL::Mesh3D FUSIONOPENGL::Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<FUSIONOPENGL::Vertex> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        FUSIONOPENGL::Vertex vertex;
        glm::vec3 vector;

        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        // normals
        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }

        if (mesh->mTextureCoords[0])
        {
            glm::vec2 vec;

            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;
            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;

        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    Mesh3D newmesh(vertices, indices);
    return newmesh;
}