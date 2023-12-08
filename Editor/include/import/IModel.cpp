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
	importer.SetPropertyBool(AI_CONFIG_FBX_USE_SKELETON_BONE_CONTAINER, true);
	importer.SetPropertyBool(AI_CONFIG_FBX_CONVERT_TO_M, true);

	
	const aiScene* pScene = importer.ReadFile(filePath,
		aiProcess_FlipUVs |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_CalcTangentSpace |
		aiProcess_FixInfacingNormals |
		aiProcess_GenSmoothNormals |
		aiProcess_ValidateDataStructure |
		//aiProcess_GlobalScale|
		aiProcess_FindInvalidData |
		aiProcess_PopulateArmatureData |
		aiProcess_OptimizeGraph
	);
	

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
		shade::SharedPointer<shade::Skeleton> skeleton = ISkeleton::ExtractSkeleton(pScene);
		auto animations = IAnimation::ImportAnimation(pScene, skeleton);
		
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

shade::SharedPointer<shade::Skeleton> ISkeleton::ExtractSkeleton(const aiScene* pScene)
{
	m_sBones.clear();

	for (uint32_t meshIndex = 0; meshIndex < pScene->mNumMeshes; ++meshIndex)
	{
		const aiMesh* mesh = pScene->mMeshes[meshIndex];
		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
		{
			m_sBones.emplace(mesh->mBones[boneIndex]->mName.C_Str());
		}
	}

	if (!m_sBones.empty())
	{
		shade::SharedPointer<shade::Skeleton> skeleton = shade::SharedPointer<shade::Skeleton>::Create();


		//ProcessSkeleton(pScene, skeleton, nullptr);
		skeleton->m_GlobalInverseTransform = glm::transpose(utils::FromAssimToToGLM<glm::mat4>(pScene->mRootNode->mTransformation));

		ProcessBone(pScene, pScene->mRootNode, skeleton, nullptr);

		return skeleton;
	}

	return nullptr;
}

void ISkeleton::ProcessNode(const aiScene* pScene, const aiNode* pNode, shade::SharedPointer<shade::Skeleton>& skeleton)
{
	
}

void ISkeleton::ProcessBone(const aiScene* pScene, const aiNode* pNode, shade::SharedPointer<shade::Skeleton>& skeleton, shade::SharedPointer<shade::Skeleton::BoneNode> parent)
{
	bool BoneHasBeenFound = false;

	for (uint32_t meshIndex = 0; meshIndex < pScene->mNumMeshes; ++meshIndex)
	{
		const aiMesh* mesh = pScene->mMeshes[meshIndex];
		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
		{
			if (skeleton->GetArmature() == nullptr)
				skeleton->AddArmature(utils::FromAssimToToGLM<glm::mat4>(mesh->mBones[boneIndex]->mArmature->mTransformation));

			if (pNode == mesh->mBones[boneIndex]->mNode)
			{
				BoneHasBeenFound = true; break;
			}
		}

		if (BoneHasBeenFound) break;
	}

	if (BoneHasBeenFound)
	{
		auto bone = skeleton->AddBone(pNode->mName.C_Str(), utils::FromAssimToToGLM<glm::mat4>(pNode->mTransformation));

		if (parent != nullptr)
			parent->Children.push_back(bone);

		SHADE_CORE_DEBUG(pNode->mName.C_Str());

		for (std::uint32_t nodeIndex = 0; nodeIndex < pNode->mNumChildren; ++nodeIndex)
			ProcessBone(pScene, pNode->mChildren[nodeIndex], skeleton, bone);
	}
	else
	{
		for (std::uint32_t nodeIndex = 0; nodeIndex < pNode->mNumChildren; ++nodeIndex)
			ProcessBone(pScene, pNode->mChildren[nodeIndex], skeleton, nullptr);
	}
}

void ISkeleton::ProcessSkeleton(const aiScene* pScene, shade::SharedPointer<shade::Skeleton>& skeleton, shade::SharedPointer<shade::Skeleton::BoneNode> bone)
{
	//for (std::uint32_t meshIndex = 0; meshIndex < pScene->mNumMeshes; ++meshIndex)
	//{
	//	const aiMesh* pMesh = pScene->mMeshes[meshIndex];

	//	for (std::uint32_t boneIndex = 0; boneIndex < pMesh->mNumBones; ++boneIndex)
	//	{
	//		const aiBone* pBone = pMesh->mBones[boneIndex];

	//		if (skeleton->m_RootBone != nullptr)
	//		{
	//			skeleton->AddBone(pBone->mName.C_Str(), utils::FromAssimToToGLM<glm::mat4>(pBone->mArmature->mTransformation));
	//		}
	//		
	//		aiMatrix4x4 assimpMatrix = pBone->mNode->mTransformation;

	//	/*	skeleton->AddBone(pBone->mName.C_Str(), utils::FromAssimToToGLM<glm::mat4>(pBone->mArmature->mTransformation));
	//			skeleton->AddBone(pBone->mName.C_Str(), utils::FromAssimToToGLM<glm::mat4>(pBone->mArmature->mTransformation));*/
	//		std::cout << "Assimp Matrix:" << std::endl;
	//		std::cout << assimpMatrix.a1 << "\t" << assimpMatrix.a2 << "\t" << assimpMatrix.a3 << "\t" << assimpMatrix.a4 << std::endl;
	//		std::cout << assimpMatrix.b1 << "\t" << assimpMatrix.b2 << "\t" << assimpMatrix.b3 << "\t" << assimpMatrix.b4 << std::endl;
	//		std::cout << assimpMatrix.c1 << "\t" << assimpMatrix.c2 << "\t" << assimpMatrix.c3 << "\t" << assimpMatrix.c4 << std::endl;
	//		std::cout << assimpMatrix.d1 << "\t" << assimpMatrix.d2 << "\t" << assimpMatrix.d3 << "\t" << assimpMatrix.d4 << std::endl;
	//	}
	//}
	//

	////for(std::uint32_t )

	//auto b = pScene->mMeshes[0]->mBones[0];
	//for (std::uint32_t skeletonIndex = 0; skeletonIndex < pScene->mNumSkeletons; ++skeletonIndex)
	//{
	//	auto* pSkeleton = pScene->mSkeletons[skeletonIndex];
	//	for (std::uint32_t boneIndex = 0; boneIndex < pSkeleton->mNumBones; ++boneIndex)
	//	{
	//		const auto* pSkeletonBone = pSkeleton->mBones[boneIndex];
	//		const auto* armature = pSkeletonBone->mArmature;

	//	}
	//}
}

std::unordered_map<std::string, shade::SharedPointer<shade::Animation>> IAnimation::ImportAnimation(const aiScene* pScene, const shade::SharedPointer<shade::Skeleton>& skeleton)
{
	std::unordered_map<std::string, shade::SharedPointer<shade::Animation>> Animations;

	for (std::uint32_t animationIndex = 0; animationIndex < pScene->mNumAnimations; ++animationIndex)
	{
		const aiAnimation* pAnimation = pScene->mAnimations[animationIndex];
		shade::SharedPointer<shade::Animation>& animation = Animations.insert({ pAnimation->mName.C_Str(), shade::SharedPointer<shade::Animation>::Create() }).first->second;

		animation->m_Duration = pAnimation->mDuration;
		animation->m_TicksPerSecond = pAnimation->mTicksPerSecond;

		for (std::uint32_t channelIndex = 0; channelIndex < pAnimation->mNumChannels; ++channelIndex)
		{
			const aiNodeAnim* channel = pAnimation->mChannels[channelIndex];
			// We found bone related to this animation channel
			if (skeleton->GetBone(channel->mNodeName.C_Str()) != nullptr)
			{
				SHADE_CORE_INFO("Add new animation channel = {0}", channel->mNodeName.C_Str());

				for (std::uint32_t positionIndex = 0; positionIndex < channel->mNumPositionKeys; ++positionIndex)
				{
					animation->m_AnimationChanels[channel->mNodeName.C_Str()].PositionKeys.emplace_back
					(
						utils::FromAssimToToGLM<glm::vec3>(channel->mPositionKeys[positionIndex].mValue),
						channel->mPositionKeys[positionIndex].mTime
					);
				}

				for (std::uint32_t rotationIndex = 0; rotationIndex < channel->mNumRotationKeys; ++rotationIndex)
				{
					animation->m_AnimationChanels[channel->mNodeName.C_Str()].RotationKeys.emplace_back
					(
						utils::FromAssimToToGLM<glm::quat>(channel->mRotationKeys[rotationIndex].mValue),
						channel->mRotationKeys[rotationIndex].mTime
					);
				}

				for (std::uint32_t scaleIndex = 0; scaleIndex < channel->mNumScalingKeys; ++scaleIndex)
				{
					animation->m_AnimationChanels[channel->mNodeName.C_Str()].ScaleKeys.emplace_back
					(
						utils::FromAssimToToGLM<glm::vec3>(channel->mScalingKeys[scaleIndex].mValue),
						channel->mScalingKeys[scaleIndex].mTime
					);
				}
			}
			else
			{
				SHADE_CORE_WARNING("Couldn't find any bones related to this animation channel = {0}", channel->mNodeName.C_Str());
			}
		}
	}

	return Animations;
}
