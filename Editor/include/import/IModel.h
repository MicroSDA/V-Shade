
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma once
#include <shade/core/layer/Layer.h>

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
};