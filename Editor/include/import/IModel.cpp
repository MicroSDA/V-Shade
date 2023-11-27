#include "shade_pch.h"
#include "IModel.h"

shade::SharedPointer<shade::Model> IModel::Import(const std::string& filePath)
{
	Assimp::Importer importer;
	const aiScene* pScene = importer.ReadFile(filePath,
		aiProcess_FlipUVs |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_CalcTangentSpace |
		aiProcess_FixInfacingNormals |
		aiProcess_GenSmoothNormals);

	if (!pScene)
	{
		SHADE_WARNING(importer.GetErrorString());

		SHADE_WARNING("Failed to import model: {0}", filePath);
		return shade::SharedPointer<shade::Model>();
	}
	else
	{
		auto model = shade::Model::CreateEXP();
		ProcessModelNode(model, filePath.c_str(), pScene->mRootNode, pScene);
		return model;

	}

	//return shade::SharedPointer<shade::Model>();
}

void IModel::ProcessModelNode(shade::SharedPointer<shade::Model>& model, const char* filePath, const aiNode* node, const aiScene* scene)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* aMesh = scene->mMeshes[node->mMeshes[i]];

		shade::SharedPointer<shade::Mesh> mesh = shade::Mesh::CreateEXP();
		ProcessMeshNode(mesh, filePath, aMesh, scene);
		model->AddMesh(mesh);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessModelNode(model, filePath, node->mChildren[i], scene); // TODO THERE ! how to return if static 
	}
}

void IModel::ProcessMeshNode(shade::SharedPointer<shade::Mesh>& mesh, const char* filePath, aiMesh* aMesh, const aiScene* scene)
{
	shade::SharedPointer<shade::AssetData> assetData = shade::SharedPointer<shade::AssetData>::Create();
	assetData->SetId(aMesh->mName.C_Str());
	mesh->SetAssetData(assetData);

	for (std::size_t v = 0; v < aMesh->mNumVertices; v++)
	{
		shade::Vertex vertex { .Position = { aMesh->mVertices[v].x, aMesh->mVertices[v].y, aMesh->mVertices[v].z} };
		if (amesh->HasNormals())
		{
			vertex.Normal = { amesh->mNormals[v].x ,amesh->mNormals[v].y,amesh->mNormals[v].z };
			if (amesh->HasTangentsAndBitangents())
			{
				vertex.Tangent   = { amesh->mTangents[v].x ,amesh->mTangents[v].y ,amesh->mTangents[v].z };
				vertex.Bitangent = { amesh->mBitangents[v].x ,amesh->mBitangents[v].y ,amesh->mBitangents[v].z };
			}
		}
		if (amesh->HasTextureCoords(0))
		{
			vertex.UV_Coordinates = { amesh->mTextureCoords[0][v].x, amesh->mTextureCoords[0][v].y };
		}

		mesh->AddVertex(vertex);
	}

	for (std::size_t i = 0; i < amesh->mNumFaces; i++)
	{
		const aiFace& face = amesh->mFaces[i];

		for (std::size_t j = 0; j < face.mNumIndices; j++)
			mesh->AddIndex(face.mIndices[j]);
	}

	if (amesh->mMaterialIndex >= 0)
	{
		// TODO: 
	}

	mesh->RecalculateAllLods(shade::Drawable::MAX_LEVEL_OF_DETAIL, mesh->GetLod(0).Indices.size() / 3, 200, 0.1);
	mesh->GenerateHalfExt();
}
