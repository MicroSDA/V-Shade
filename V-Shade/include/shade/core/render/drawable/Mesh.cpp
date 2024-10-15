#include "shade_pch.h"
#include "Mesh.h"
#include <shade/core/asset/AssetManager.h>
// TEMPORARY
#include <shade/core/physics/algo/ConvexHullGenerator.h>

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


	if (file::File file = file::FileManager::LoadFile(filePath, "@s_mesh"))
	{
		file.Read(*this);

		for (std::size_t lod = 1; lod < GetLods().size(); lod++)
		{
			if (!GetLod(lod).Vertices.size())
			{
				GetLod(lod) = GetLod(0);
			}
		}
	}
	else
	{
		SHADE_CORE_WARNING("Failed to read file, wrong path = {0}", filePath)
	}
}

void shade::Mesh::Serialize(std::ostream& stream) const
{
	serialize::Serializer::Serialize(stream, Drawable::MAX_LEVEL_OF_DETAIL);

	for (auto& lod : GetLods())
	{
		serialize::Serializer::Serialize(stream, std::uint32_t(lod.Vertices.size()));
		serialize::Serializer::Serialize(stream, std::uint32_t(lod.Indices.size()));
		serialize::Serializer::Serialize(stream, std::uint32_t(lod.Bones.size()));

		for (auto& vertex : lod.Vertices)
		{
			serialize::Serializer::Serialize(stream, vertex.Position.x);
			serialize::Serializer::Serialize(stream, vertex.Position.y);
			serialize::Serializer::Serialize(stream, vertex.Position.z);

			serialize::Serializer::Serialize(stream, vertex.UV_Coordinates.x);
			serialize::Serializer::Serialize(stream, vertex.UV_Coordinates.y);

			serialize::Serializer::Serialize(stream, vertex.Normal.x);
			serialize::Serializer::Serialize(stream, vertex.Normal.y);
			serialize::Serializer::Serialize(stream, vertex.Normal.z);

			serialize::Serializer::Serialize(stream, vertex.Tangent.x);
			serialize::Serializer::Serialize(stream, vertex.Tangent.y);
			serialize::Serializer::Serialize(stream, vertex.Tangent.z);
		}

		for (auto& index : lod.Indices)
			serialize::Serializer::Serialize(stream, index);

		for (auto& bone : lod.Bones)
		{
			serialize::Serializer::Serialize(stream, *bone.IDs.data(), MAX_BONES_PER_VERTEX);
			serialize::Serializer::Serialize(stream, *bone.Weights.data(), MAX_BONES_PER_VERTEX);
		}
	}

	/* AABB */
	serialize::Serializer::Serialize(stream, GetMinHalfExt().x);	serialize::Serializer::Serialize(stream, GetMinHalfExt().y); serialize::Serializer::Serialize(stream, GetMinHalfExt().z);
	serialize::Serializer::Serialize(stream, GetMaxHalfExt().x);	serialize::Serializer::Serialize(stream, GetMaxHalfExt().y); serialize::Serializer::Serialize(stream, GetMaxHalfExt().z);

	/* Convex parts */
	/*Serializer::Serialize(stream, static_cast<std::uint32_t>(m_ConvexParts.second.size()));
	for(const )*/
}

void shade::Mesh::Deserialize(std::istream& stream)
{
	// TODO: Need to make it safe, if file is empty for example !
	if (stream.good() || !stream.eof())
	{
		std::uint32_t lodCount = 0;
		serialize::Serializer::Deserialize(stream, lodCount);

		if (lodCount <= 0 || lodCount > Drawable::MAX_LEVEL_OF_DETAIL)
			throw std::exception("Invalide lods count!");

		for (std::size_t i = 0; i < lodCount; i++)
		{
			std::uint32_t verticesCount = 0;
			serialize::Serializer::Deserialize(stream, verticesCount);
			if (verticesCount == UINT32_MAX)
				throw std::exception("Invalide vertices count!");

			std::uint32_t indicesCount = 0;
			serialize::Serializer::Deserialize(stream, indicesCount);
			if (indicesCount == UINT32_MAX)
				throw std::exception("Invalide indices count!");

			std::uint32_t bonesCount = 0;
			serialize::Serializer::Deserialize(stream, bonesCount);
			if (bonesCount == UINT32_MAX)
				throw std::exception("Invalide indices count!");

			Vertices vertices(verticesCount);
			Indices indices(indicesCount);
			Bones bones(bonesCount);
			/*if (bonesCount > 100)
			{
				stream.seekg(std::size_t(stream.tellg()) - sizeof(std::uint32_t));
			}
			else
				bones.resize(bonesCount);*/

			for (auto& vertex : vertices)
			{
				serialize::Serializer::Deserialize(stream, vertex.Position.x);
				serialize::Serializer::Deserialize(stream, vertex.Position.y);
				serialize::Serializer::Deserialize(stream, vertex.Position.z);

				serialize::Serializer::Deserialize(stream, vertex.UV_Coordinates.x);
				serialize::Serializer::Deserialize(stream, vertex.UV_Coordinates.y);

				serialize::Serializer::Deserialize(stream, vertex.Normal.x);
				serialize::Serializer::Deserialize(stream, vertex.Normal.y);
				serialize::Serializer::Deserialize(stream, vertex.Normal.z);

				serialize::Serializer::Deserialize(stream, vertex.Tangent.x);
				serialize::Serializer::Deserialize(stream, vertex.Tangent.y);
				serialize::Serializer::Deserialize(stream, vertex.Tangent.z);
			}

			for (auto& index : indices)
				serialize::Serializer::Deserialize(stream, index);

			for (auto& bone : bones)
			{
				serialize::Serializer::Deserialize(stream, *bone.IDs.data(), MAX_BONES_PER_VERTEX);
				serialize::Serializer::Deserialize(stream, *bone.Weights.data(), MAX_BONES_PER_VERTEX);
			}

			SetVertices(vertices, i); SetIndices(indices, i); SetBones(bones, i);

			
		}

		/* AABB */
		glm::vec3 minHalf, maxHalf;
		serialize::Serializer::Deserialize(stream, minHalf.x);	serialize::Serializer::Deserialize(stream, minHalf.y); serialize::Serializer::Deserialize(stream, minHalf.z);
		serialize::Serializer::Deserialize(stream, maxHalf.x);	serialize::Serializer::Deserialize(stream, maxHalf.y); serialize::Serializer::Deserialize(stream, maxHalf.z);
		SetMinHalfExt(minHalf); SetMaxHalfExt(maxHalf);

	}
	else
	{
		SHADE_CORE_WARNING("Couldn't read mesh - corrupted file !");
	}
	
}