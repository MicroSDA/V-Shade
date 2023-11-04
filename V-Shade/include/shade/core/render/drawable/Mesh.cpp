#include "shade_pch.h"
#include "Mesh.h"
#include <shade/core/asset/AssetManager.h>
// TEMPORARY
#include <shade/core/physics/algo/ConvexHullGenerator.h>

shade::AssetMeta::Type shade::Mesh::GetAssetStaticType()
{
	return AssetMeta::Type::Mesh;
}

shade::AssetMeta::Type shade::Mesh::GetAssetType() const
{
	return GetAssetStaticType();
}

shade::Mesh::Mesh(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour) : BaseAsset(assetData, lifeTime, behaviour)
{
	for (auto& dependency : assetData->GetDependencies())
	{
		if (dependency->GetType() == AssetMeta::Type::Material)
		{
			auto callback = [&](auto& material) mutable
				{
					Drawable::SetMaterial(material);
				};

			if (behaviour == InstantiationBehaviour::Synchronous)
				AssetManager::GetAsset<Material, InstantiationBehaviour::Synchronous>(dependency->GetId(), AssetMeta::Category::Primary, BaseAsset::LifeTime::KeepAlive, callback);
			else
				AssetManager::GetAsset<Material, InstantiationBehaviour::Aynchronous>(dependency->GetId(), AssetMeta::Category::Primary, BaseAsset::LifeTime::KeepAlive, callback);
		}
	}

	auto mesh = assetData->GetReference();
	if (!mesh)
		throw std::exception("Mesh doens't have secondary reference!");

	auto filePath = mesh->GetAttribute<std::string>("Path");

	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open())
		SHADE_CORE_WARNING("Failed to read file, wrong path = {0}", filePath)
	else
	{
		Deserialize(file);
		for (std::size_t lod = 1; lod < GetLods().size(); lod++)
		{
			if (!GetLod(lod).Vertices.size())
			{
				GetLod(lod) = GetLod(0);
			}
		}
	}
	file.close();
}

shade::Mesh* shade::Mesh::Create(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour)
{
	return new Mesh(assetData, lifeTime, behaviour);
}

std::size_t shade::Mesh::Serialize(std::ostream& stream) const
{
	if (stream.good())
	{
		std::string header("@s_mesh");
		Serializer::Serialize(stream, header);
		Serializer::Serialize(stream, Drawable::MAX_LEVEL_OF_DETAIL);

		for (auto& lod : GetLods())
		{
			Serializer::Serialize(stream, std::uint32_t(lod.Vertices.size()));
			Serializer::Serialize(stream, std::uint32_t(lod.Indices.size()));

			for (auto& vertex : lod.Vertices)
			{
				Serializer::Serialize(stream, vertex.Position.x);
				Serializer::Serialize(stream, vertex.Position.y);
				Serializer::Serialize(stream, vertex.Position.z);

				Serializer::Serialize(stream, vertex.UV_Coordinates.x);
				Serializer::Serialize(stream, vertex.UV_Coordinates.y);

				Serializer::Serialize(stream, vertex.Normal.x);
				Serializer::Serialize(stream, vertex.Normal.y);
				Serializer::Serialize(stream, vertex.Normal.z);

				Serializer::Serialize(stream, vertex.Tangent.x);
				Serializer::Serialize(stream, vertex.Tangent.y);
				Serializer::Serialize(stream, vertex.Tangent.z);
			}

			for (auto& index : lod.Indices)
				Serializer::Serialize(stream, index);
		}
		
		/* AABB */
		Serializer::Serialize(stream, GetMinHalfExt().x);	Serializer::Serialize(stream, GetMinHalfExt().y); Serializer::Serialize(stream, GetMinHalfExt().z);
		Serializer::Serialize(stream, GetMaxHalfExt().x);	Serializer::Serialize(stream, GetMaxHalfExt().y); Serializer::Serialize(stream, GetMaxHalfExt().z);

		/* Convex parts */
		/*Serializer::Serialize(stream, static_cast<std::uint32_t>(m_ConvexParts.second.size()));
		for(const )*/
		
	}
	return std::size_t();
}

std::size_t shade::Mesh::Deserialize(std::istream& stream)
{
	// TODO: Need to make it safe, if file is empty for example !
	if (stream.good() || !stream.eof())
	{
		std::string header;
		Serializer::Deserialize(stream, header);
		if (header == "@s_mesh")
		{
			std::uint32_t lodCount = 0;
			Serializer::Deserialize(stream, lodCount);

			if(lodCount <= 0 || lodCount > Drawable::MAX_LEVEL_OF_DETAIL)
				throw std::exception("Invalide lods count!");

			for (std::size_t i = 0; i < lodCount; i++)
			{
				std::uint32_t verticesCount = 0;
				Serializer::Deserialize(stream, verticesCount);
				if (verticesCount == UINT32_MAX)
					throw std::exception("Invalide vertices count!");

				std::uint32_t indicesCount = 0;
				Serializer::Deserialize(stream, indicesCount);
				if (indicesCount == UINT32_MAX)
					throw std::exception("Invalide indices count!");

				Vertices vertices(verticesCount);
				Indices indices(indicesCount);

				for (auto& vertex : vertices)
				{
					Serializer::Deserialize(stream, vertex.Position.x);
					Serializer::Deserialize(stream, vertex.Position.y);
					Serializer::Deserialize(stream, vertex.Position.z);

					Serializer::Deserialize(stream, vertex.UV_Coordinates.x);
					Serializer::Deserialize(stream, vertex.UV_Coordinates.y);

					Serializer::Deserialize(stream, vertex.Normal.x);
					Serializer::Deserialize(stream, vertex.Normal.y);
					Serializer::Deserialize(stream, vertex.Normal.z);

					Serializer::Deserialize(stream, vertex.Tangent.x);
					Serializer::Deserialize(stream, vertex.Tangent.y);
					Serializer::Deserialize(stream, vertex.Tangent.z);
				}

				for (auto& index : indices)
					Serializer::Deserialize(stream, index);

				SetVertices(vertices, i); SetIndices(indices, i);
				//SetVertices(vertices); SetIndices(indices);
			}

			/* AABB */
			glm::vec3 minHalf, maxHalf;
			Serializer::Deserialize(stream, minHalf.x);	Serializer::Deserialize(stream, minHalf.y); Serializer::Deserialize(stream, minHalf.z);
			Serializer::Deserialize(stream, maxHalf.x);	Serializer::Deserialize(stream, maxHalf.y); Serializer::Deserialize(stream, maxHalf.z);
			SetMinHalfExt(minHalf); SetMaxHalfExt(maxHalf);	

		}
		else
		{
			SHADE_CORE_WARNING("Wrong file format!");
		}
	}
	else
	{
		SHADE_CORE_WARNING("Couldn't read mesh - corrupted file !");
	}
	

	return stream.tellg();
}
