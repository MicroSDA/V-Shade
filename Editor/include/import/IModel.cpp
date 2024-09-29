#include "shade_pch.h"
#include "IModel.h"
#include <assimp/config.h>

namespace utils
{
	template<typename G, typename A>
	static G FromAssimpToGLM(const A& value);

	template<>
	inline glm::mat4 FromAssimpToGLM(const aiMatrix4x4& aMatrix)
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
	inline glm::vec3 FromAssimpToGLM(const aiVector3D& aVector)
	{
		return { aVector.x, aVector.y, aVector.z };
	}
	template<>
	inline glm::quat FromAssimpToGLM(const aiQuaternion& aQuat)
	{
		return { aQuat.w, aQuat.x, aQuat.y, aQuat.z };
	}
}
std::pair<shade::SharedPointer<shade::Model>, std::unordered_map<std::string, shade::SharedPointer<shade::Animation>>> IModel::Import(const std::string& filePath, IImportFlag flags, float scale)
{
	Assimp::Importer importer;
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false); // false
	importer.SetPropertyBool(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, false); // Add as settings
	importer.SetPropertyBool(AI_CONFIG_FBX_CONVERT_TO_M, false);
	//importer.SetPropertyBool(AI_GLTF, false);

	
	unsigned int ImportFagls = 
		aiProcess_OptimizeGraph |
		aiProcess_ValidateDataStructure |
		aiProcess_PopulateArmatureData |
		aiProcess_FindInvalidData |
		aiProcess_OptimizeMeshes |
		aiProcess_FixInfacingNormals;

	ImportFagls |=
		((flags & Triangulate) ? aiProcess_Triangulate : 0) |
		((flags & FlipUVs) ? aiProcess_FlipUVs : 0) |
		((flags & JoinIdenticalVertices) ? aiProcess_JoinIdenticalVertices : 0) |
		((flags & CalcTangentSpace) ? aiProcess_CalcTangentSpace : 0) |
		((flags & CalcNormals) ? aiProcess_ForceGenNormals : 0) |
		((flags & GenSmoothNormals) ? aiProcess_GenSmoothNormals : 0) |
		((flags & UseScale) ? aiProcess_GlobalScale : 0);

	const aiScene* pScene = importer.ReadFile(filePath, ImportFagls);

	if (!pScene)
	{
		SHADE_WARNING(importer.GetErrorString());

		SHADE_WARNING("Failed to import model: {0}", filePath);
		return { nullptr, {} };
	}
	else
	{
		auto model = shade::Model::CreateEXP();
		model->SetAssetData(shade::SharedPointer<shade::AssetData>::Create(pScene->mName.C_Str(), shade::AssetMeta::Category::Primary, shade::AssetMeta::Type::Model));


		if (flags & ImportModel)
		{
			SHADE_INFO("******************************************************************");
			SHADE_INFO("Start to extractin model");
			SHADE_INFO("******************************************************************");


			ProcessModelNode(model, filePath.c_str(), pScene->mRootNode, pScene, flags);
		}

		shade::SharedPointer<shade::Skeleton> skeleton;

		if (flags & TryToImportSkeleton)
		{
			SHADE_INFO("******************************************************************");
			SHADE_INFO("Start to extracting skeleton");
			SHADE_INFO("******************************************************************");

			skeleton = ISkeleton::ExtractSkeleton(pScene);
		}

		std::unordered_map<std::string, shade::SharedPointer<shade::Animation>> animations;

		if (flags & ImportAnimation)
		{
			SHADE_INFO("******************************************************************");
			SHADE_INFO("Start to extracting animations");
			SHADE_INFO("******************************************************************");

			animations = IAnimation::ImportAnimations(pScene, skeleton);
		}

		model->SetSkeleton(skeleton);

		return { model, animations };
	}
}

void IModel::ProcessModelNode(shade::SharedPointer<shade::Model>& model, const char* filePath, const aiNode* node, const aiScene* scene, IImportFlag flags)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* aMesh = scene->mMeshes[node->mMeshes[i]];

		shade::SharedPointer<shade::Mesh> mesh = shade::Mesh::CreateEXP();
		ProcessMeshNode(model, mesh, filePath, aMesh, node, flags);
		model->AddMesh(mesh);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessModelNode(model, filePath, node->mChildren[i], scene, flags);
	}
}

void IModel::ProcessMeshNode(shade::SharedPointer<shade::Model>& model, shade::SharedPointer<shade::Mesh>& mesh, const char* filePath, aiMesh* aMesh, const aiNode* node, IImportFlag flags)
{
	//node->mTransformation;

	SHADE_INFO("-- Process new mesh {0} in {1} node --", aMesh->mName.C_Str(), node->mName.C_Str());

	mesh->SetAssetData(shade::SharedPointer<shade::AssetData>::Create(aMesh->mName.C_Str(), shade::AssetMeta::Category::Primary, shade::AssetMeta::Type::Mesh));

	SHADE_INFO("Vertex count : {}", aMesh->mNumVertices);

	for (std::size_t v = 0; v < aMesh->mNumVertices; v++)
	{
		
		shade::Vertex vertex{ .Position = { aMesh->mVertices[v].x, aMesh->mVertices[v].y, aMesh->mVertices[v].z} };
		//shade::Vertex vertex{ .Position = utils::FromAssimpToGLM<glm::mat4>(node->mTransformation) * glm::vec4{ aMesh->mVertices[v].x, aMesh->mVertices[v].y, aMesh->mVertices[v].z, 1.f } };

		if (aMesh->HasNormals())
		{
			vertex.Normal = { aMesh->mNormals[v].x ,aMesh->mNormals[v].y, aMesh->mNormals[v].z };
			//vertex.Normal = { utils::FromAssimpToGLM<glm::mat4>(node->mTransformation) * glm::vec4{ aMesh->mNormals[v].x, aMesh->mNormals[v].y, aMesh->mNormals[v].z, 1.f } };
			
			if (aMesh->HasTangentsAndBitangents())
			{
				vertex.Tangent = { aMesh->mTangents[v].x, aMesh->mTangents[v].y, aMesh->mTangents[v].z };
				vertex.Bitangent = { aMesh->mBitangents[v].x, aMesh->mBitangents[v].y, aMesh->mBitangents[v].z };
			}
		}
		if (aMesh->HasTextureCoords(0))
			vertex.UV_Coordinates = { aMesh->mTextureCoords[0][v].x, aMesh->mTextureCoords[0][v].y };

		mesh->AddVertex(vertex);
	}

	SHADE_INFO("Faces count : {}", aMesh->mNumFaces);

	for (std::size_t i = 0; i < aMesh->mNumFaces; i++)
	{
		const aiFace& face = aMesh->mFaces[i];

		for (std::size_t j = 0; j < face.mNumIndices; j++)
			mesh->AddIndex(face.mIndices[j]);
	}
	
	if (aMesh->HasBones() && (flags & BakeBoneIdsWeightsIntoMesh))
		ProcessBones(mesh, filePath, aMesh, node);

	if (aMesh->mMaterialIndex >= 0)
	{
		// TODO: 
	}

	//mesh->RecalculateAllLods(shade::Drawable::MAX_LEVEL_OF_DETAIL, mesh->GetLod(0).Indices.size() / 3, 200, 0.1);
	mesh->GenerateHalfExt();
}

void IModel::ProcessBones(shade::SharedPointer<shade::Mesh>& mesh, const char* filePath, aiMesh* aMesh, const aiNode* pNode)
{
	SHADE_INFO("Bones count : {}", aMesh->mNumBones);

	// Where std::string name -> std::uint32_t id
	std::unordered_map<std::string, std::uint32_t> boneList;
	// We have bones only for lod at level 0 !!!!
	mesh->GetLod(0).Bones.resize(mesh->GetLod(0).Vertices.size());

	for (std::size_t boneIndex = 0; boneIndex < aMesh->mNumBones; ++boneIndex)
	{
		std::uint32_t boneId = shade::Skeleton::BONE_NULL_ID;

		std::string boneName = aMesh->mBones[boneIndex]->mName.C_Str();

		if (boneList.find(boneName) == boneList.end())
		{
			SHADE_INFO("-- Attach '{0}' bone to : '{1}' mesh --", boneName, aMesh->mName.C_Str());
			boneId = boneList.size();
			boneList[boneName] = boneId;
		}
		else
		{
			boneId = boneList[boneName];
		}

		assert(boneId != shade::Skeleton::BONE_NULL_ID);

		for (std::uint32_t weightIndex = 0; weightIndex < aMesh->mBones[boneIndex]->mNumWeights; ++weightIndex)
		{
			std::uint32_t vertexId = aMesh->mBones[boneIndex]->mWeights[weightIndex].mVertexId;
			float weight = aMesh->mBones[boneIndex]->mWeights[weightIndex].mWeight;

			assert(vertexId <= mesh->GetVertices().size());

			for (std::uint32_t i = 0; i < shade::MAX_BONES_PER_VERTEX; ++i)
			{
				auto& bone = mesh->GetLod(0).Bones[vertexId];

				if (bone.IDs[i] == shade::Skeleton::BONE_NULL_ID)
				{
					bone.IDs[i] = boneId;
					bone.Weights[i] = weight;
					break;
				}
			}
		}
	}
}

shade::SharedPointer<shade::Skeleton> ISkeleton::ExtractSkeleton(const aiScene* pScene)
{
	shade::SharedPointer<shade::Skeleton> skeleton = shade::Skeleton::CreateEXP();

	skeleton->SetAssetData(shade::SharedPointer<shade::AssetData>::Create(pScene->mName.C_Str(), shade::AssetMeta::Category::Secondary, shade::AssetMeta::Type::Skeleton));
	ProcessBone(pScene, pScene->mRootNode, skeleton, nullptr);
	
	return (skeleton->GetBones().size()) ? skeleton : nullptr;
}

void PrintMatrix(const glm::mat4& matrix) {
	for (int i = 0; i < 4; ++i) { // По строкам
		for (int j = 0; j < 4; ++j) { // По столбцам
			std::cout << matrix[i][j] << "\t"; // Вывод элемента матрицы
		}
		std::cout << std::endl; // Перевод строки для новой строки матрицы
	}
}
void ISkeleton::ProcessBone(const aiScene* pScene, const aiNode* pNode, shade::SharedPointer<shade::Skeleton>& skeleton, shade::Skeleton::BoneNode* parent)
{
	
	std::cout << pNode->mName.C_Str() << std::endl;
	PrintMatrix(utils::FromAssimpToGLM<glm::mat4>(pNode->mTransformation));

	aiBone* pBone = nullptr;

	for (uint32_t meshIndex = 0; meshIndex < pScene->mNumMeshes; ++meshIndex)
	{
		const aiMesh* mesh = pScene->mMeshes[meshIndex];
		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
		{
			//SHADE_INFO("-- Add Armature --");
			skeleton->AddArmature(mesh->mBones[boneIndex]->mArmature->mName.C_Str(), utils::FromAssimpToGLM<glm::mat4>(mesh->mBones[boneIndex]->mArmature->mTransformation));
			if (pNode == mesh->mBones[boneIndex]->mNode)
			{
				pBone = mesh->mBones[boneIndex]; break;
			}
		}

		if (pBone) break;
	}

	if (pBone)
	{
		SHADE_INFO("-- Add new bone : {} --", pNode->mName.C_Str());

		shade::Skeleton::BoneNode* bone = skeleton->AddBone(pNode->mName.C_Str(), utils::FromAssimpToGLM<glm::mat4>(pNode->mTransformation), utils::FromAssimpToGLM<glm::mat4>(pBone->mOffsetMatrix));

		if (parent != nullptr)
			parent->Children.push_back(bone);

		for (std::uint32_t nodeIndex = 0; nodeIndex < pNode->mNumChildren; ++nodeIndex)
			ProcessBone(pScene, pNode->mChildren[nodeIndex], skeleton, bone);
	}
	else
	{
		for (std::uint32_t nodeIndex = 0; nodeIndex < pNode->mNumChildren; ++nodeIndex)
			ProcessBone(pScene, pNode->mChildren[nodeIndex], skeleton, nullptr);
	}
}

std::unordered_map<std::string, shade::SharedPointer<shade::Animation>> IAnimation::ImportAnimations(const aiScene* pScene, const shade::SharedPointer<shade::Skeleton>& skeleton)
{
	std::unordered_map<std::string, shade::SharedPointer<shade::Animation>> Animations;

	SHADE_INFO("Animation count : {}", pScene->mNumAnimations);

	for (std::uint32_t animationIndex = 0; animationIndex < pScene->mNumAnimations; ++animationIndex)
	{
		const aiAnimation* pAnimation = pScene->mAnimations[animationIndex];

		SHADE_INFO("-- Process '{0}' animation --", pAnimation->mName.C_Str());

		shade::SharedPointer<shade::Animation> animation = shade::Animation::CreateEXP();

		shade::SharedPointer<shade::AssetData> assetData = shade::SharedPointer<shade::AssetData>::Create();

		assetData->SetId(pAnimation->mName.C_Str());
		animation->SetAssetData(assetData);

		SHADE_INFO("Duration : {0}, Ticks per second : {1}", pAnimation->mDuration, pAnimation->mTicksPerSecond);

		animation->SetDuration(pAnimation->mDuration); animation->SetTicksPerSecond(pAnimation->mTicksPerSecond);

		SHADE_INFO("Channels : {}", pAnimation->mNumChannels);

		for (std::uint32_t channelIndex = 0; channelIndex < pAnimation->mNumChannels; ++channelIndex)
		{
			const aiNodeAnim* pChannel = pAnimation->mChannels[channelIndex];
			shade::Animation::Channel channel;

			if (skeleton)
			{
				if (!skeleton->GetBone(pChannel->mNodeName.C_Str()) && skeleton->GetArmature()->Name != pChannel->mNodeName.C_Str())
				{
					SHADE_WARNING("Rudenant animation channel '{0}' for '{1}' animation, skipping...", pChannel->mNodeName.C_Str(), pAnimation->mName.C_Str());
					continue;
				}
			}

			SHADE_INFO("-- Add new '{0}' channel into '{1}' animation -- ", pChannel->mNodeName.C_Str(), pAnimation->mName.C_Str());
			SHADE_INFO("Position keys : {0}', Rotation keys : {1}, Scaling keys : {2}", pChannel->mNumPositionKeys, pChannel->mNumRotationKeys, pChannel->mNumScalingKeys);

			for (std::uint32_t positionIndex = 0; positionIndex < pChannel->mNumPositionKeys; ++positionIndex)
			{
				channel.PositionKeys.emplace_back
				(
					utils::FromAssimpToGLM<glm::vec3>(pChannel->mPositionKeys[positionIndex].mValue),
					pChannel->mPositionKeys[positionIndex].mTime
				);
			}
			for (std::uint32_t rotationIndex = 0; rotationIndex < pChannel->mNumRotationKeys; ++rotationIndex)
			{
				channel.RotationKeys.emplace_back
				(
					utils::FromAssimpToGLM<glm::quat>(pChannel->mRotationKeys[rotationIndex].mValue),
					pChannel->mRotationKeys[rotationIndex].mTime
				);
			}
			for (std::uint32_t scaleIndex = 0; scaleIndex < pChannel->mNumScalingKeys; ++scaleIndex)
			{
				channel.ScaleKeys.emplace_back
				(
					utils::FromAssimpToGLM<glm::vec3>(pChannel->mScalingKeys[scaleIndex].mValue),
					pChannel->mScalingKeys[scaleIndex].mTime
				);
			}

			animation->AddChannel(pChannel->mNodeName.C_Str(), channel);
		}

		Animations[pAnimation->mName.C_Str()] = animation;
	}

	return Animations;
}
