#include "shade_pch.h"
#include "IModel.h"
#include <assimp/config.h>

namespace utils
{
	template<typename G, typename A>
	static G FromAssimToToGLM(const A& value);


	template<>
	inline glm::mat4 FromAssimToToGLM(const aiMatrix4x4& aMatrix)
	{
		glm::mat4 matrix;
		//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		matrix[0][0] = aMatrix.a1; matrix[1][0] = aMatrix.a2; matrix[2][0] = aMatrix.a3; matrix[3][0] = aMatrix.a4;
		matrix[0][1] = aMatrix.b1; matrix[1][1] = aMatrix.b2; matrix[2][1] = aMatrix.b3; matrix[3][1] = aMatrix.b4;
		matrix[0][2] = aMatrix.c1; matrix[1][2] = aMatrix.c2; matrix[2][2] = aMatrix.c3; matrix[3][2] = aMatrix.c4;
		matrix[0][3] = aMatrix.d1; matrix[1][3] = aMatrix.d2; matrix[2][3] = aMatrix.d3; matrix[3][3] = aMatrix.d4;
		
		/*for (auto i = 0; i < 4; ++i) {
			for (auto j = 0; j < 4; ++j) {
				matrix[j][i] = aMatrix[i][j];
			}
		}*/
		return matrix;
	}
	template<>
	inline glm::vec3 FromAssimToToGLM(const aiVector3D& aVector)
	{
		return { aVector.x, aVector.y, aVector.z };
	}
	template<>
	inline glm::quat FromAssimToToGLM(const aiQuaternion& aQuat)
	{
		return { aQuat.w, aQuat.x, aQuat.y, aQuat.z };
	}
}
std::tuple<shade::SharedPointer<shade::Model>, std::unordered_map<std::string, shade::SharedPointer<shade::Animation>>> IModel::Import(const std::string& filePath)
{
	Assimp::Importer importer;
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	const aiScene* pScene = importer.ReadFile(filePath,
		aiProcess_FlipUVs |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_CalcTangentSpace |
		aiProcess_FixInfacingNormals |
		aiProcess_GenSmoothNormals |
		aiProcess_ValidateDataStructure |
		aiProcess_OptimizeGraph |
		aiProcess_GlobalScale);
	

	if (!pScene)
	{
		SHADE_WARNING(importer.GetErrorString());

		SHADE_WARNING("Failed to import model: {0}", filePath);
		return { nullptr, {} };
	}
	else
	{
		auto model = shade::Model::CreateEXP();
		ProcessModelNode(model, filePath.c_str(), pScene->mRootNode, pScene);
		auto animations = IAnimation::ImportAnimation(pScene);
		shade::SharedPointer<shade::Skeleton> skeleton = ISkeleton::ExtractSkeleton(pScene);
		model->m_Skeleton = skeleton;

		
		return { model, animations };
	}
}

void IModel::ProcessModelNode(shade::SharedPointer<shade::Model>& model, const char* filePath, const aiNode* node, const aiScene* scene)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* aMesh = scene->mMeshes[node->mMeshes[i]];

		shade::SharedPointer<shade::Mesh> mesh = shade::Mesh::CreateEXP();
		ProcessMeshNode(model, mesh, filePath, aMesh, node);
		model->AddMesh(mesh);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessModelNode(model, filePath, node->mChildren[i], scene); // TODO THERE ! how to return if static 
	}
}

void IModel::ProcessMeshNode(shade::SharedPointer<shade::Model>& model, shade::SharedPointer<shade::Mesh>& mesh, const char* filePath, aiMesh* aMesh, const aiNode* node)
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
		ProcessBones(mesh, filePath, aMesh, node);
		

	if (aMesh->mMaterialIndex >= 0)
	{
		// TODO: 
	}

	mesh->RecalculateAllLods(shade::Drawable::MAX_LEVEL_OF_DETAIL, mesh->GetLod(0).Indices.size() / 3, 200, 0.1);
	mesh->GenerateHalfExt();
}

void IModel::ProcessBones(shade::SharedPointer<shade::Mesh>& mesh, const char* filePath, aiMesh* aMesh, const aiNode* node)
{
	// We have bones only for lod at level 0 !!!!
	mesh->GetLod(0).Bones.resize(mesh->GetLod(0).Vertices.size());

	for (std::size_t boneIndex = 0; boneIndex < aMesh->mNumBones; ++boneIndex)
	{
		std::uint32_t boneId = ~0;
		std::string boneName = aMesh->mBones[boneIndex]->mName.C_Str();

		if (mesh->m_BoneInfoMap.find(boneName) == mesh->m_BoneInfoMap.end())
		{
			shade::BoneInfo newBoneInfo;
			newBoneInfo.ID = mesh->m_BoneInfoMap.size();
			newBoneInfo.Offset = utils::FromAssimToToGLM<glm::mat4>(aMesh->mBones[boneIndex]->mOffsetMatrix);
			
			boneId = mesh->m_BoneInfoMap.size();
			mesh->m_BoneInfoMap[boneName] = newBoneInfo; 
		}
		else
		{
			boneId = mesh->m_BoneInfoMap[boneName].ID;
		}

		assert(boneId != ~0);

		for (std::uint32_t weightIndex = 0; weightIndex < aMesh->mBones[boneIndex]->mNumWeights; ++weightIndex)
		{
			std::uint32_t vertexId = aMesh->mBones[boneIndex]->mWeights[weightIndex].mVertexId;
			float weight = aMesh->mBones[boneIndex]->mWeights[weightIndex].mWeight;
			assert(vertexId <= mesh->GetVertices().size());

			for (std::uint32_t i = 0; i < shade::MAX_BONES_PER_VERTEX; ++i)
			{
				auto& bone = mesh->GetBones()[vertexId];

				if (bone.IDs[i] == ~0)
				{
					bone.IDs[i] = boneId;
					bone.Weights[i] = weight;
					break;
				}
			}
		}
	}
}

std::set<std::string> ISkeleton::m_sBones;

shade::SharedPointer<shade::Skeleton> ISkeleton::ExtractSkeleton(const aiScene* scene)
{
	m_sBones.clear();

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
	{
		const aiMesh* mesh = scene->mMeshes[meshIndex];
		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
		{
			m_sBones.emplace(mesh->mBones[boneIndex]->mName.C_Str());
		}
	}

	if (!m_sBones.empty())
	{
		shade::SharedPointer<shade::Skeleton> skeleton = shade::SharedPointer<shade::Skeleton>::Create();

		ProcessBone(scene, scene->mRootNode, skeleton, nullptr);

		return skeleton;
	}

	return nullptr;
}

void ISkeleton::ProcessNode(const aiScene* scene, aiNode* node, shade::SharedPointer<shade::Skeleton>& skeleton)
{
	ProcessBone(scene, node, skeleton, nullptr);
}

void ISkeleton::ProcessBone(const aiScene* pScene, const aiNode* pNode, shade::SharedPointer<shade::Skeleton>& skeleton, shade::SharedPointer<shade::Skeleton::BoneNode> bone)
{
	bool channelFound = true;

	for (std::uint32_t animationIndex = 0; animationIndex < pScene->mNumAnimations; ++animationIndex)
	{
		for (std::uint32_t chanelIndex = 0; chanelIndex < pScene->mAnimations[animationIndex]->mNumChannels; ++chanelIndex)
		{
			if (strcmp(pNode->mName.C_Str(), pScene->mAnimations[animationIndex]->mChannels[chanelIndex]->mNodeName.C_Str()) == 0)
			{
				channelFound = true; break;
			}
		}
		if (channelFound) break;
	}

	if (channelFound)
	{
		auto node = skeleton->AddBone(pNode->mName.C_Str(), utils::FromAssimToToGLM<glm::mat4>(pNode->mTransformation));

		if (bone != nullptr)
			bone->Children.push_back(node);

		SHADE_CORE_DEBUG(pNode->mName.C_Str());

		for (std::uint32_t nodeIndex = 0; nodeIndex < pNode->mNumChildren; ++nodeIndex)
			ProcessBone(pScene, pNode->mChildren[nodeIndex], skeleton, node);
	}
	else
	{
		for (std::uint32_t nodeIndex = 0; nodeIndex < pNode->mNumChildren; ++nodeIndex)
			ProcessBone(pScene, pNode->mChildren[nodeIndex], skeleton, nullptr);
	}
	
}

std::unordered_map<std::string, shade::SharedPointer<shade::Animation>> IAnimation::ImportAnimation(const aiScene* pScene)
{
	std::unordered_map<std::string, shade::SharedPointer<shade::Animation>> Animations;

	for (std::uint32_t animationIndex = 0; animationIndex < pScene->mNumAnimations; ++animationIndex)
	{
		shade::SharedPointer<shade::Animation>& animation = Animations.insert({ pScene->mAnimations[animationIndex]->mName.C_Str(), shade::SharedPointer<shade::Animation>::Create() }).first->second;
		
		animation->m_Duration = pScene->mAnimations[animationIndex]->mDuration;
		animation->m_TicksPerSecond = pScene->mAnimations[animationIndex]->mTicksPerSecond;

		for (std::uint32_t chanelIndex = 0; chanelIndex < pScene->mAnimations[animationIndex]->mNumChannels; ++chanelIndex)
		{
			SHADE_CORE_DEBUG(pScene->mAnimations[animationIndex]->mChannels[chanelIndex]->mNodeName.C_Str());
			auto* chanel = pScene->mAnimations[animationIndex]->mChannels[chanelIndex];

			for (std::uint32_t positionIndex = 0; positionIndex < pScene->mAnimations[animationIndex]->mChannels[chanelIndex]->mNumPositionKeys; ++positionIndex)
			{
				animation->m_AnimationChanels[pScene->mAnimations[animationIndex]->mChannels[chanelIndex]->mNodeName.C_Str()].PositionKeys.emplace_back
				(
					utils::FromAssimToToGLM<glm::vec3>(pScene->mAnimations[animationIndex]->mChannels[chanelIndex]->mPositionKeys[positionIndex].mValue),
					pScene->mAnimations[animationIndex]->mChannels[chanelIndex]->mPositionKeys[positionIndex].mTime
				);
			}

			for (std::uint32_t rotationIndex = 0; rotationIndex < pScene->mAnimations[animationIndex]->mChannels[chanelIndex]->mNumRotationKeys; ++rotationIndex)
			{
				animation->m_AnimationChanels[pScene->mAnimations[animationIndex]->mChannels[chanelIndex]->mNodeName.C_Str()].RotationKeys.emplace_back
				(
					utils::FromAssimToToGLM<glm::quat>(pScene->mAnimations[animationIndex]->mChannels[chanelIndex]->mRotationKeys[rotationIndex].mValue),
					pScene->mAnimations[animationIndex]->mChannels[chanelIndex]->mRotationKeys[rotationIndex].mTime
				);
			}

			for (std::uint32_t scaleIndex = 0; scaleIndex < pScene->mAnimations[animationIndex]->mChannels[chanelIndex]->mNumScalingKeys; ++scaleIndex)
			{
				animation->m_AnimationChanels[pScene->mAnimations[animationIndex]->mChannels[chanelIndex]->mNodeName.C_Str()].ScaleKeys.emplace_back
				(
					utils::FromAssimToToGLM<glm::vec3>(pScene->mAnimations[animationIndex]->mChannels[chanelIndex]->mScalingKeys[scaleIndex].mValue),
					pScene->mAnimations[animationIndex]->mChannels[chanelIndex]->mScalingKeys[scaleIndex].mTime
				);
			}
		}

	}

	return Animations;
}
