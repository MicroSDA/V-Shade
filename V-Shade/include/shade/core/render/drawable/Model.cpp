#include "shade_pch.h"
#include "Model.h"
#include <shade/core/asset/AssetManager.h>

shade::Model::~Model()
{
	m_Meshes.clear();
}

shade::AssetMeta::Type shade::Model::GetAssetStaticType()
{
    return AssetMeta::Type::Model;
}

shade::AssetMeta::Type shade::Model::GetAssetType() const
{
    return GetAssetStaticType();
}

void shade::Model::AddMesh(const Asset<Mesh>& mesh)
{
	m_Meshes.emplace_back(mesh);
}

const std::vector<shade::Asset<shade::Mesh>>& shade::Model::GetMeshes() const
{
	return m_Meshes;
}

std::vector<shade::Asset<shade::Mesh>>& shade::Model::GetMeshes()
{
	return m_Meshes;
}

void shade::Model::SetSkeleton(const Asset<Skeleton>& skeleton)
{
	m_Skeleton = skeleton;
}

const shade::Asset<shade::Skeleton>& shade::Model::GetSkeleton() const
{
	return m_Skeleton;
}

shade::Model::Model(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour) : BaseAsset(assetData, lifeTime, behaviour)
{
	for (auto& dependency : assetData->GetDependencies())
	{
		if (dependency->GetType() == AssetMeta::Type::Mesh)
		{
			auto callback = [&](auto& mesh) mutable
				{
					AddMesh(mesh);
				};

			if(behaviour == InstantiationBehaviour::Synchronous)
				AssetManager::GetAsset<Mesh, InstantiationBehaviour::Synchronous>(dependency->GetId(), AssetMeta::Category::Primary, BaseAsset::LifeTime::KeepAlive, callback);
			else
				AssetManager::GetAsset<Mesh, InstantiationBehaviour::Aynchronous>(dependency->GetId(), AssetMeta::Category::Primary, BaseAsset::LifeTime::KeepAlive, callback);	
		}
		if (dependency->GetType() == AssetMeta::Type::Skeleton)
		{
			auto callback = [&](auto& skeleton) mutable
				{
					SetSkeleton(skeleton);
				};

			if (behaviour == InstantiationBehaviour::Synchronous)
				AssetManager::GetAsset<Skeleton, InstantiationBehaviour::Synchronous>(dependency->GetId(), AssetMeta::Category::Secondary, BaseAsset::LifeTime::KeepAlive, callback);
			else
				AssetManager::GetAsset<Skeleton, InstantiationBehaviour::Aynchronous>(dependency->GetId(), AssetMeta::Category::Secondary, BaseAsset::LifeTime::KeepAlive, callback);
		}
	}
}

std::size_t shade::Model::SerializeAsComponent(std::ostream& stream) const
{
	return shade::Serializer::Serialize(stream, GetAssetData()->GetId());
}

std::size_t shade::Model::DeserializeAsComponent(std::istream& stream)
{
	return std::size_t();
}

shade::Model* shade::Model::Create(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour)
{
    return new Model(assetData, lifeTime, behaviour);
}
shade::SharedPointer<shade::Model> shade::Model::CreateEXP()
{
	return SharedPointer<shade::Model>::Create();
}

