
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma once
#include <shade/core/layer/Layer.h>

// TODO: Need to refactor !
struct BoneInfo
{
	/*id is index in finalBoneMatrices*/
	int id;

	/*offset matrix transforms vertex from model space to bone space*/
	glm::mat4 offset;
};

class IModel
{
public:
	IModel() = default;
	virtual ~IModel() = default;
public:
	static shade::SharedPointer<shade::Model> Import(const std::string& filePath);
private:
	static void ProcessModelNode(shade::SharedPointer<shade::Model>& model, const char* filePath, const aiNode* node, const aiScene* scene);
	static void ProcessMeshNode(shade::SharedPointer<shade::Mesh>& mesh, const char* filePath, aiMesh* amesh, const aiScene* scene);
	static void ProcessBones(shade::SharedPointer<shade::Mesh>& mesh, const char* filePath, aiMesh* amesh, const aiScene* scene);
private:
	static std::map<std::string, BoneInfo> m_sBoneInfoMap;
};