#include "Model.hpp"
#include "Light.hpp"
#include "ShadowMaps.hpp"

unsigned int counter = 0;

FUSIONCORE::Model::Model()
{
    LOG("USED THIS ONE");
    this->AnimationEnabled = false;
    FinalAnimationMatrices = nullptr;
    this->ModelID = counter;
    counter++;
}
FUSIONCORE::Model::Model(std::string const& filePath, bool Async, bool AnimationModel)
{
    this->AnimationEnabled = AnimationModel;
    FinalAnimationMatrices = nullptr;
    this->loadModel(filePath, Async, AnimationModel);
    FindGlobalMeshScales();
    this->ModelID = counter;
    counter++;
}

void FUSIONCORE::Model::Draw(Camera3D& camera, Shader& shader, std::function<void()> &ShaderPreperations)
{
    std::function<void()> shaderPrep = [&]() {
        this->GetTransformation().SetModelMatrixUniformLocation(shader.GetID(), "model");
        FUSIONCORE::SendLightsShader(shader);
        shader.setFloat("ModelID", this->GetModelID());
        shader.setFloat("ObjectScale", this->GetTransformation().scale_avg);
        shader.setInt("OmniShadowMapCount", 0);
        //shader.setBool("EnableAnimation", false);
        ShaderPreperations();
    };

	for (size_t i = 0; i < Meshes.size(); i++)
	{
		Meshes[i].Draw(camera, shader, shaderPrep);
	}
}

void FUSIONCORE::Model::Draw(Camera3D& camera, Shader& shader, Material material, std::function<void()>& ShaderPreperations)
{
    std::function<void()> shaderPrep = [&]() {
        this->GetTransformation().SetModelMatrixUniformLocation(shader.GetID(), "model");
        FUSIONCORE::SendLightsShader(shader);
        shader.setFloat("ModelID", this->GetModelID());
        shader.setFloat("ObjectScale", this->GetTransformation().scale_avg);
        shader.setInt("OmniShadowMapCount", 0);
        shader.setBool("EnableAnimation", false);
        ShaderPreperations();
    };

	for (size_t i = 0; i < Meshes.size(); i++)
	{
		Meshes[i].Draw(camera, shader,material, shaderPrep);
	}
}


void FUSIONCORE::Model::Draw(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, CubeMap& cubemap,Material material, float EnvironmentAmbientAmount)
{
    std::function<void()> shaderPrep = [&]() {
        this->GetTransformation().SetModelMatrixUniformLocation(shader.GetID(), "model");
        FUSIONCORE::SendLightsShader(shader);
        shader.setFloat("ModelID", this->GetModelID());
        shader.setFloat("ObjectScale", this->GetTransformation().scale_avg);
        shader.setInt("OmniShadowMapCount", 0);
        shader.setBool("EnableAnimation", false);
        ShaderPreperations();
    };

    for (size_t i = 0; i < Meshes.size(); i++)
    {
        Meshes[i].Draw(camera, shader, shaderPrep, cubemap ,material, EnvironmentAmbientAmount);
    }
}

void FUSIONCORE::Model::Draw(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, CubeMap& cubemap, Material material, std::vector<OmniShadowMap*> ShadowMaps, float EnvironmentAmbientAmount)
{
    std::function<void()> shaderPrep = [&]() {
        this->GetTransformation().SetModelMatrixUniformLocation(shader.GetID(), "model");
        FUSIONCORE::SendLightsShader(shader);
        shader.setFloat("ModelID", this->GetModelID());
        shader.setFloat("ObjectScale", this->GetTransformation().scale_avg);
        shader.setInt("OmniShadowMapCount", ShadowMaps.size());
        for (size_t i = 0; i < ShadowMaps.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + 8 + i);
            glBindTexture(GL_TEXTURE_CUBE_MAP, ShadowMaps[i]->GetShadowMap());
            glUniform1i(glGetUniformLocation(shader.GetID(), ("OmniShadowMaps[" + std::to_string(i) + "]").c_str()), 8 + i);
            glUniform1f(glGetUniformLocation(shader.GetID(), ("ShadowMapFarPlane[" + std::to_string(i) + "]").c_str()), ShadowMaps[i]->GetFarPlane());
        }
        shader.setBool("EnableAnimation", false);
        ShaderPreperations();
    };

    for (size_t i = 0; i < Meshes.size(); i++)
    {
        Meshes[i].Draw(camera, shader, shaderPrep, cubemap, material, EnvironmentAmbientAmount);
    }
}

void FUSIONCORE::Model::Draw(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, CubeMap& cubemap, Material material, std::vector<OmniShadowMap*> ShadowMaps, std::vector<glm::mat4>& AnimationBoneMatrices, float EnvironmentAmbientAmount)
{
    std::function<void()> shaderPrep = [&]() {
        this->GetTransformation().SetModelMatrixUniformLocation(shader.GetID(), "model");
        FUSIONCORE::SendLightsShader(shader);
        shader.setFloat("ModelID", this->GetModelID());
        shader.setFloat("ObjectScale", this->GetTransformation().scale_avg);
        shader.setInt("OmniShadowMapCount", ShadowMaps.size());
        for (size_t i = 0; i < ShadowMaps.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + 8 + i);
            glBindTexture(GL_TEXTURE_CUBE_MAP, ShadowMaps[i]->GetShadowMap());
            glUniform1i(glGetUniformLocation(shader.GetID(), ("OmniShadowMaps[" + std::to_string(i) + "]").c_str()), 8 + i);
            glUniform1f(glGetUniformLocation(shader.GetID(), ("ShadowMapFarPlane[" + std::to_string(i) + "]").c_str()), ShadowMaps[i]->GetFarPlane());
        }
        for (int i = 0; i < AnimationBoneMatrices.size(); ++i)
        {
            shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", AnimationBoneMatrices[i]);
        }
        shader.setBool("EnableAnimation", true);
        ShaderPreperations();
    };

    for (size_t i = 0; i < Meshes.size(); i++)
    {
        Meshes[i].Draw(camera, shader, shaderPrep, cubemap, material, EnvironmentAmbientAmount);
    }
}

void FUSIONCORE::Model::DrawInstanced(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, CubeMap& cubemap, Material material,VBO &InstanceDataVBO,size_t InstanceCount,std::vector<OmniShadowMap*> ShadowMaps, float EnvironmentAmbientAmount)
{
    std::function<void()> shaderPrep = [&]() {
        this->GetTransformation().SetModelMatrixUniformLocation(shader.GetID(), "model");
        FUSIONCORE::SendLightsShader(shader);
        shader.setFloat("ModelID", this->GetModelID());
        shader.setFloat("ObjectScale", this->GetTransformation().scale_avg);
        shader.setInt("OmniShadowMapCount", ShadowMaps.size());
        for (size_t i = 0; i < ShadowMaps.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + 8 + i);
            glBindTexture(GL_TEXTURE_CUBE_MAP, ShadowMaps[i]->GetShadowMap());
            glUniform1i(glGetUniformLocation(shader.GetID(), ("OmniShadowMaps[" + std::to_string(i) + "]").c_str()), 8 + i);
            glUniform1f(glGetUniformLocation(shader.GetID(), ("ShadowMapFarPlane[" + std::to_string(i) + "]").c_str()), ShadowMaps[i]->GetFarPlane());
        }
        shader.setBool("EnableAnimation", false);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.GetConvDiffCubeMap());
        shader.setInt("ConvDiffCubeMap", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.GetPreFilteredEnvMap());
        shader.setInt("prefilteredMap", 2);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, FUSIONCORE::brdfLUT);
        shader.setInt("LUT", 3);

        shader.setBool("EnableIBL", true);
        shader.setFloat("ao", EnvironmentAmbientAmount);

        material.SetMaterialShader(shader);

        InstanceDataVBO.Bind();
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribDivisor(7, 1); // Tell OpenGL this is an instanced vertex attribute.

        ShaderPreperations();
    };

    for (size_t i = 0; i < Meshes.size(); i++)
    {
        Meshes[i].DrawInstanced(camera, shader, shaderPrep,InstanceCount, EnvironmentAmbientAmount);
    }
}

void FUSIONCORE::Model::DrawDeferredInstanced(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, Material material, VBO& InstanceDataVBO, size_t InstanceCount, float EnvironmentAmbientAmount)
{
    std::function<void()> shaderPrep = [&]() {
        this->GetTransformation().SetModelMatrixUniformLocation(shader.GetID(), "model");
        FUSIONCORE::SendLightsShader(shader);
        shader.setFloat("ModelID", this->GetModelID());
        shader.setFloat("ObjectScale", this->GetTransformation().scale_avg);
        material.SetMaterialShader(shader);

        if (InstanceDataVBO.IsVBOchanged())
        {
            InstanceDataVBO.Bind();
            glEnableVertexAttribArray(7);
            glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glVertexAttribDivisor(7, 1);
            InstanceDataVBO.SetVBOstate(false);
        }
        ShaderPreperations();
    };

    for (size_t i = 0; i < Meshes.size(); i++)
    {
        Meshes[i].DrawInstanced(camera, shader, shaderPrep, InstanceCount, EnvironmentAmbientAmount);
    }
}

void FUSIONCORE::Model::DrawDeferred(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, CubeMap& cubemap, Material material, std::vector<OmniShadowMap*> ShadowMaps, std::vector<glm::mat4>& AnimationBoneMatrices, float EnvironmentAmbientAmount)
{
    std::function<void()> shaderPrep = [&]() {
        this->GetTransformation().SetModelMatrixUniformLocation(shader.GetID(), "model");
        FUSIONCORE::SendLightsShader(shader);
        shader.setFloat("ModelID", this->GetModelID());
        shader.setFloat("ObjectScale", this->GetTransformation().scale_avg);
        for (int i = 0; i < AnimationBoneMatrices.size(); ++i)
        {
            shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", AnimationBoneMatrices[i]);
        }
        shader.setBool("EnableAnimation", true);
        ShaderPreperations();
        };

    for (size_t i = 0; i < Meshes.size(); i++)
    {
        Meshes[i].DrawDeferred(camera, shader, shaderPrep, cubemap, material, EnvironmentAmbientAmount);
    }
}

void FUSIONCORE::Model::Draw(Camera3D& camera, Shader& shader, std::vector<Material> materials, std::function<void()>& ShaderPreperations, CubeMap& cubemap, float EnvironmentAmbientAmount)
{
    std::function<void()> shaderPrep = [&]() 
    {
        this->GetTransformation().SetModelMatrixUniformLocation(shader.GetID(), "model");
        FUSIONCORE::SendLightsShader(shader);
        shader.setFloat("ModelID", this->GetModelID());
        shader.setInt("OmniShadowMapCount", 0);
        shader.setBool("EnableAnimation", false);
        ShaderPreperations();
    };

    for (size_t i = 0; i < Meshes.size(); i++)
    {
        Meshes[i].Draw(camera, shader, shaderPrep, cubemap, materials[i], EnvironmentAmbientAmount);
    }
}

void FUSIONCORE::Model::Draw(Camera3D& camera, Shader& shader, std::vector<Material> materials, std::function<void()>& ShaderPreperations, CubeMap& cubemap, std::vector<OmniShadowMap*> ShadowMaps, float EnvironmentAmbientAmount)
{
    std::function<void()> shaderPrep = [&]() {
        this->GetTransformation().SetModelMatrixUniformLocation(shader.GetID(), "model");
        FUSIONCORE::SendLightsShader(shader);
        shader.setFloat("ModelID", this->GetModelID());
        shader.setInt("OmniShadowMapCount", ShadowMaps.size());
        for (size_t i = 0; i < ShadowMaps.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + 7 + i);
            glBindTexture(GL_TEXTURE_CUBE_MAP, ShadowMaps[i]->GetShadowMap());
            glUniform1i(glGetUniformLocation(shader.GetID(), ("OmniShadowMaps[" + std::to_string(i) + "]").c_str()), 7 + i);
        }
        shader.setBool("EnableAnimation", false);
        ShaderPreperations();
    };

    for (size_t i = 0; i < Meshes.size(); i++)
    {
        Meshes[i].Draw(camera, shader, shaderPrep, cubemap, materials[i], EnvironmentAmbientAmount);
    }
}

void FUSIONCORE::Model::Draw(Camera3D& camera, Shader& shader, std::vector<Material> materials, std::function<void()>& ShaderPreperations, CubeMap& cubemap, std::vector<OmniShadowMap*> ShadowMaps, std::vector<glm::mat4>& AnimationBoneMatrices, float EnvironmentAmbientAmount)
{
    std::function<void()> shaderPrep = [&]() {
        this->GetTransformation().SetModelMatrixUniformLocation(shader.GetID(), "model");
        FUSIONCORE::SendLightsShader(shader);
        shader.setFloat("ModelID", this->GetModelID());
        shader.setInt("OmniShadowMapCount", ShadowMaps.size());
        for (size_t i = 0; i < ShadowMaps.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + 7 + i);
            glBindTexture(GL_TEXTURE_CUBE_MAP, ShadowMaps[i]->GetShadowMap());
            glUniform1i(glGetUniformLocation(shader.GetID(), ("OmniShadowMaps[" + std::to_string(i) + "]").c_str()), 7 + i);
        }
        for (int i = 0; i < AnimationBoneMatrices.size(); ++i)
        {
            shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", AnimationBoneMatrices[i]);
        }
        shader.setBool("EnableAnimation", true);
        ShaderPreperations();
    };

    for (size_t i = 0; i < Meshes.size(); i++)
    {
        Meshes[i].Draw(camera, shader, shaderPrep, cubemap, materials[i], EnvironmentAmbientAmount);
    }
}

void FUSIONCORE::Model::DrawImportedMaterial(Camera3D& camera, Shader& shader, std::function<void()>& ShaderPreperations, CubeMap& cubemap, float EnvironmentAmbientAmount)
{
    std::function<void()> shaderPrep = [&]() {
        this->GetTransformation().SetModelMatrixUniformLocation(shader.GetID(), "model");
        FUSIONCORE::SendLightsShader(shader);
        shader.setFloat("ModelID", this->GetModelID());
        shader.setFloat("ObjectScale", this->GetTransformation().scale_avg);
        shader.setBool("EnableAnimation", false);
        ShaderPreperations();
    };

    for (size_t i = 0; i < Meshes.size(); i++)
    {
        Meshes[i].DrawImportedMaterial(camera, shader, shaderPrep, cubemap, EnvironmentAmbientAmount);
    }
}

void FUSIONCORE::Model::FindGlobalMeshScales()
{

	float maxX = -std::numeric_limits<float>::infinity();
	float maxY = -std::numeric_limits<float>::infinity();
	float maxZ = -std::numeric_limits<float>::infinity();
	float minX = std::numeric_limits<float>::infinity();
	float minY = std::numeric_limits<float>::infinity();
	float minZ = std::numeric_limits<float>::infinity();

	for (size_t j = 0; j < Meshes.size(); j++)
	{
		auto& VertexArray = Meshes[j].GetVertices();
		Vertex origin = *VertexArray[0]; 

		for (unsigned int k = 0; k < VertexArray.size(); k++) {

			Vertex vertex;
			vertex.Position.x = VertexArray[k]->Position.x - origin.Position.x;
			vertex.Position.y = VertexArray[k]->Position.y - origin.Position.y;
			vertex.Position.z = VertexArray[k]->Position.z - origin.Position.z;

			maxX = std::max(maxX, vertex.Position.x);
			maxY = std::max(maxY, vertex.Position.y);
			maxZ = std::max(maxZ, vertex.Position.z);
			minX = std::min(minX, vertex.Position.x);
			minY = std::min(minY, vertex.Position.y);
			minZ = std::min(minZ, vertex.Position.z);
		}
	}


	float meshWidth = maxX - minX;
	float meshHeight = maxY - minY;
	float meshDepth = maxZ - minZ;

	transformation.ObjectScales.x = meshWidth;
	transformation.ObjectScales.y = meshHeight;
	transformation.ObjectScales.z = meshDepth;

	maxX = -std::numeric_limits<float>::infinity();
	maxY = -std::numeric_limits<float>::infinity();
	maxZ = -std::numeric_limits<float>::infinity();
	minX = std::numeric_limits<float>::infinity();
	minY = std::numeric_limits<float>::infinity();
	minZ = std::numeric_limits<float>::infinity();

	transformation.scale_avg = (transformation.ObjectScales.x + transformation.ObjectScales.y + transformation.ObjectScales.z) / 3;
	transformation.dynamic_scale_avg = transformation.scale_avg;

	transformation.ObjectScales.x = transformation.ObjectScales.x;
	transformation.ObjectScales.y = transformation.ObjectScales.y;
	transformation.ObjectScales.z = transformation.ObjectScales.z;

	transformation.InitialObjectScales = transformation.ObjectScales;

	std::cout << "Model width: " << transformation.ObjectScales.x << " Model height: " << transformation.ObjectScales.y << " Model Depth: " << transformation.ObjectScales.z << "\n";
	std::cout << "Scale avg: " << transformation.scale_avg << "\n";

}

void FUSIONCORE::Model::loadModel(std::string const& path, bool Async , bool AnimationModel)
{
    Assimp::Importer importer;
    int flags;
    if (AnimationModel)
    {
        flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace;
    }
    else
    {
        flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices;
    }
    const aiScene* scene = importer.ReadFile(path, flags);
    this->scene = (aiScene*)scene;
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));

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


    float overallCenterX = 0.0f, overallCenterY = 0.0f, overallCenterZ = 0.0f;
    for (unsigned int i = 0; i < originPoints.size(); i++) {
        overallCenterX += originPoints[i].x;
        overallCenterY += originPoints[i].y;
        overallCenterZ += originPoints[i].z;
    }
    overallCenterX /= originPoints.size();
    overallCenterY /= originPoints.size();
    overallCenterZ /= originPoints.size();

    originpoint = { overallCenterX,overallCenterY,overallCenterZ };
    transformation.OriginPoint = &this->originpoint;
    transformation.Position = glm::vec3(originpoint.x, originpoint.y, originpoint.z);
    dynamic_origin = glm::vec3(overallCenterX, overallCenterY, overallCenterZ);

    std::cout << "Overall origin point: (" << overallCenterX << ", " << overallCenterY << ", " << overallCenterZ << ")" << std::endl;

    if (Async)
    {
        processNodeAsync(scene->mRootNode, scene);
    }
    else
    {
        processNode(scene->mRootNode, scene);
    }
}

void FUSIONCORE::Model::processNode(aiNode* node, const aiScene* scene)
{
    this->ModelName = scene->mName.data;
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {

        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        Meshes.push_back(processMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

void FUSIONCORE::Model::processNodeAsync(aiNode* node, const aiScene* scene)
{
    this->ModelName = scene->mName.data;
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        PreMeshDatas.push_back(processMeshAsync(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNodeAsync(node->mChildren[i], scene);
    }
}

void FUSIONCORE::Model::ConstructMeshes(std::vector<PreMeshData> PreMeshDatas)
{
    for (size_t i = 0; i < PreMeshDatas.size(); i++)
    {
        PreMeshData& data = PreMeshDatas[i];
        Meshes.push_back(Mesh(data.vertices,data.indices,data.textures));
    }
}

FUSIONCORE::PreMeshData FUSIONCORE::Model::processMeshAsync(aiMesh* mesh, const aiScene* scene)
{
    std::vector<std::shared_ptr<Vertex>> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture2D> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector;

        SetVertexBoneDataDefault(vertex);

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
            glm::vec2 vec;

            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;

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

        vertices.push_back(std::make_shared<Vertex>(vertex));
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }


    ExtractBones(vertices, mesh, scene);

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];


    /*std::vector<Texture2D> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    std::vector<Texture2D> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    std::vector<Texture2D> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

    std::vector<Texture2D> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());*/

    loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", textures);
    loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", textures);
    loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal", textures);
    loadMaterialTextures(material, aiTextureType_METALNESS, "texture_metalic", textures);

    return PreMeshData(textures , indices , vertices);
}

FUSIONCORE::Mesh FUSIONCORE::Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<std::shared_ptr<Vertex>> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture2D> textures;
    std::vector<std::shared_ptr<Face>> Faces;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector;

        SetVertexBoneDataDefault(vertex);

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
            glm::vec2 vec;

            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;

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

        vertices.push_back(std::make_shared<Vertex>(vertex));
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        Face NewFace;
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
            NewFace.Indices.push_back(face.mIndices[j]);
        }
        Faces.push_back(std::make_shared<Face>(NewFace));
    }

    ExtractBones(vertices, mesh, scene);

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse" , textures);
    loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular" , textures);
    loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal" , textures);
    loadMaterialTextures(material, aiTextureType_METALNESS, "texture_metalic" , textures);
   
    Mesh newMesh(vertices, indices,Faces, textures);
    newMesh.MeshName = mesh->mName.data;
    return newMesh;
}

void FUSIONCORE::Model::ExtractBones(std::vector<std::shared_ptr<Vertex>>& vertices, aiMesh* mesh, const aiScene* scene)
{
    //LOG("mesh->mNumBones: " << mesh->mNumBones);

    for (size_t i = 0; i < mesh->mNumBones; i++)
    {
        int boneID = -1;
        std::string boneName = mesh->mBones[i]->mName.C_Str();
        if (Bones.find(boneName) == Bones.end())
        {
            BoneInfo newBone;
            newBone.id = boneCounter;
            newBone.OffsetMat = this->ConvertMatrixToGLMFormat(mesh->mBones[i]->mOffsetMatrix);
            Bones[boneName] = newBone;
            boneID = boneCounter;
            boneCounter++;
        }
        else
        {
            boneID = Bones[boneName].id;
        }

        assert(boneID != -1);
        auto weights = mesh->mBones[i]->mWeights;
        int NumberWeights = mesh->mBones[i]->mNumWeights;
        //LOG("NumberWeights[" << i << "] : " << NumberWeights);

        for (size_t x = 0; x < NumberWeights; x++)
        {
            int vertexID = weights[x].mVertexId;
            float weight = weights[x].mWeight;
            //LOG("vertexID: " << vertexID << "weight: " << weight);
            assert(vertexID <= vertices.size());

            auto& vertextemp = vertices[vertexID];
            for (size_t e = 0; e < MAX_BONE_INFLUENCE; e++)
            {
                if (vertextemp->m_BoneIDs[e] < 0)
                {
                    vertextemp->m_Weights[e] = weight;
                    vertextemp->m_BoneIDs[e] = boneID;
                    break;
                }
            }
        }

    }
}

void FUSIONCORE::Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName , std::vector<FUSIONCORE::Texture2D> &Destination)
{
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if (std::strcmp(textures_loaded[j].GetFilePath().c_str(), str.C_Str()) == 0)
            {
                Destination.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if (!skip)
        {
            std::string FilePath;
            if (directory.find('\\') != std::string::npos)
            {
                FilePath = directory.substr(0, directory.find_last_of('\\'));
                FilePath += "\\" + std::string(str.C_Str());
            }
            else if (directory.find('/') != std::string::npos)
            {
                FilePath = directory.substr(0, directory.find_last_of('/'));
                FilePath += "/" + std::string(str.C_Str());
            }
            
            Texture2D texture(FilePath.c_str(), GL_TEXTURE_2D, GL_UNSIGNED_BYTE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT, false);
            texture.PbrMapType = typeName;

            Destination.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }
}

