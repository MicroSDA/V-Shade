#include "shade_pch.h"
#include "IModel.h"

std::map<std::string, BoneInfo> IModel::m_sBoneInfoMap;

namespace utils
{
	template<typename G, typename A>
	static G FromAssimToToGLM(const A& value);


	template<>
	inline glm::mat4 FromAssimToToGLM(const aiMatrix4x4& aMatrix)
	{
		glm::mat4 matrix;
		////the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		//matrix[0][0] = aMatrix.a1; matrix[1][0] = aMatrix.a2; matrix[2][0] = aMatrix.a3; matrix[3][0] = aMatrix.a4;
		//matrix[0][1] = aMatrix.b1; matrix[1][1] = aMatrix.b2; matrix[2][1] = aMatrix.b3; matrix[3][1] = aMatrix.b4;
		//matrix[0][2] = aMatrix.c1; matrix[1][2] = aMatrix.c2; matrix[2][2] = aMatrix.c3; matrix[3][2] = aMatrix.c4;
		//matrix[0][3] = aMatrix.d1; matrix[1][3] = aMatrix.d2; matrix[2][3] = aMatrix.d3; matrix[3][3] = aMatrix.d4;
		
		for (auto i = 0; i < 4; ++i) {
			for (auto j = 0; j < 4; ++j) {
				matrix[j][i] = aMatrix[i][j];
			}
		}
		return matrix;
	}
}
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

		if (aMesh->HasNormals())
		{
			vertex.Normal = { aMesh->mNormals[v].x ,aMesh->mNormals[v].y, aMesh->mNormals[v].z };

			if (aMesh->HasTangentsAndBitangents())
			{
				vertex.Tangent   = { aMesh->mTangents[v].x, aMesh->mTangents[v].y, aMesh->mTangents[v].z };
				vertex.Bitangent = { aMesh->mBitangents[v].x, aMesh->mBitangents[v].y, aMesh->mBitangents[v].z };
			}
		}
		if (aMesh->HasTextureCoords(0))
			vertex.UV_Coordinates = { aMesh->mTextureCoords[0][v].x, aMesh->mTextureCoords[0][v].y };

		mesh->AddVertex(vertex);
	}

	for (std::size_t i = 0; i < aMesh->mNumFaces; i++)
	{
		const aiFace& face = aMesh->mFaces[i];

		for (std::size_t j = 0; j < face.mNumIndices; j++)
			mesh->AddIndex(face.mIndices[j]);
	}

	if (aMesh->HasBones())
		ProcessBones(mesh, filePath, aMesh, scene);

	if (aMesh->mMaterialIndex >= 0)
	{
		// TODO: 
	}

	mesh->RecalculateAllLods(shade::Drawable::MAX_LEVEL_OF_DETAIL, mesh->GetLod(0).Indices.size() / 3, 200, 0.1);
	mesh->GenerateHalfExt();
}

void IModel::ProcessBones(shade::SharedPointer<shade::Mesh>& mesh, const char* filePath, aiMesh* aMesh, const aiScene* scene)
{
	m_sBoneInfoMap.clear();
	// We have bones only for lod at level 0 !!!!
	mesh->GetLod(0).Bones.resize(mesh->GetLod(0).Vertices.size());

	for (std::size_t boneIndex = 0; boneIndex < aMesh->mNumBones; ++boneIndex)
	{
		std::uint32_t boneId = std::numeric_limits<std::uint32_t>::max();
		std::string boneName = aMesh->mBones[boneIndex]->mName.C_Str();

		if (m_sBoneInfoMap.find(boneName) == m_sBoneInfoMap.end())
		{
			BoneInfo newBoneInfo;
			newBoneInfo.id = m_sBoneInfoMap.size();
			newBoneInfo.offset = utils::FromAssimToToGLM<glm::mat4>(aMesh->mBones[boneIndex]->mOffsetMatrix);
			
			boneId = m_sBoneInfoMap.size();
			m_sBoneInfoMap[boneName] = newBoneInfo;
		}
		else
		{
			boneId = m_sBoneInfoMap[boneName].id;
		}

		assert(boneId != std::numeric_limits<std::uint32_t>::max());

		for (std::uint32_t weightIndex = 0; weightIndex < aMesh->mBones[boneIndex]->mNumWeights; ++weightIndex)
		{
			std::uint32_t vertexId = aMesh->mBones[boneIndex]->mWeights[weightIndex].mVertexId;
			float weight = aMesh->mBones[boneIndex]->mWeights[weightIndex].mWeight;
			assert(vertexId <= mesh->GetVertices().size());

			for (auto &bone : mesh->GetBones()[vertexId])
			{
				if (bone.Id == std::numeric_limits<std::uint32_t>::max())
				{
					bone.Id = boneId;
					bone.Weight = weight;
					break;
				}
			}
		}
	}
}
