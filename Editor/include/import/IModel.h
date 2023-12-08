
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma once
#include <shade/core/layer/Layer.h>


class ISkeleton
{
public:
	static shade::SharedPointer<shade::Skeleton> ExtractSkeleton(const aiScene* scene);
private:
	static void ProcessNode(const aiScene* pScene, const aiNode* pNode, shade::SharedPointer<shade::Skeleton>& skeleton);
	static void ProcessBone(const aiScene* pScene, const aiNode* pNode, shade::SharedPointer<shade::Skeleton>& skeleton, shade::SharedPointer<shade::Skeleton::BoneNode> bone);
	static void ProcessSkeleton(const aiScene* pScene, shade::SharedPointer<shade::Skeleton>& skeleton, shade::SharedPointer<shade::Skeleton::BoneNode> bone);
private:
	static std::set<std::string> m_sBones;
};

class IAnimation
{
public:
	static std::unordered_map<std::string, shade::SharedPointer<shade::Animation>> ImportAnimation(const aiScene* scene, const shade::SharedPointer<shade::Skeleton>& skeleton);
};

class IModel
{
public:
	IModel() = default;
	virtual ~IModel() = default;
public:
	static std::tuple<shade::SharedPointer<shade::Model>, std::unordered_map<std::string, shade::SharedPointer<shade::Animation>>> Import(const std::string& filePath);
private:
	static void ProcessModelNode(shade::SharedPointer<shade::Model>& model, const char* filePath, const aiNode* node, const aiScene* scene);
	static void ProcessMeshNode(shade::SharedPointer<shade::Model>& model, shade::SharedPointer<shade::Mesh>& mesh, const char* filePath, aiMesh* amesh, const aiNode* node);
	static void ProcessBones(shade::SharedPointer<shade::Mesh>& mesh, const char* filePath, aiMesh* amesh, const aiNode* node);
};