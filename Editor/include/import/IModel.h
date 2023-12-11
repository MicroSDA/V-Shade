#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <shade/core/layer/Layer.h>

enum IImportFlags : std::uint32_t
{
	ImportModel						= (1u << 0),
	ImportMeshes					= (1u << 1),
	ImportMaterials					= (1u << 2),
	ImportAnimation					= (1u << 3),
	TryToImportSkeleton				= (1u << 4),
	BakeBoneIdsWeightsIntoMesh		= (1u << 5),
	TryValidateAnimationChannels	= (1u << 6),
	Triangulate						= (1u << 7),
	FlipUVs							= (1u << 8),
	JoinIdenticalVertices			= (1u << 9),
	CalcTangentSpace				= (1u << 10),
	CalcBitangents					= (1u << 11),
	CalcNormals						= (1u << 12),
	GenSmoothNormals				= (1u << 13)
};

using IImportFlag = std::uint32_t;

class ISkeleton
{
public:
	static shade::SharedPointer<shade::Skeleton> ExtractSkeleton(const aiScene* scene);
private:
	static void ProcessBone(const aiScene* pScene, const aiNode* pNode, shade::SharedPointer<shade::Skeleton>& skeleton, shade::SharedPointer<shade::Skeleton::BoneNode> bone);
};

class IAnimation
{
public:
	static shade::AnimationControllerComponent ImportAnimations(const aiScene* scene, const shade::SharedPointer<shade::Skeleton>& skeleton = nullptr);
};

class IModel
{
public:
	IModel() = default;
	virtual ~IModel() = default;
public:
	static std::pair<shade::SharedPointer<shade::Model>, shade::AnimationControllerComponent> Import(const std::string& filePath, IImportFlag flags);
private:
	static void ProcessModelNode(shade::SharedPointer<shade::Model>& model, const char* filePath, const aiNode* node, const aiScene* scene, IImportFlag flags);
	static void ProcessMeshNode(shade::SharedPointer<shade::Model>& model, shade::SharedPointer<shade::Mesh>& mesh, const char* filePath, aiMesh* amesh, const aiNode* node, IImportFlag flags);
	static void ProcessBones(shade::SharedPointer<shade::Mesh>& mesh, const char* filePath, aiMesh* amesh, const aiNode* node);
};