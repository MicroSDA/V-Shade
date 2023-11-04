#include "shade_pch.h"
#include "Material.h"
#include <shade/core/asset/AssetManager.h>

shade::Material::Material(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour) : BaseAsset(assetData, lifeTime, behaviour)
{
	for (auto& dependency : assetData->GetDependencies())
	{
		if (dependency->GetType() == AssetMeta::Type::Texture)
		{
			auto callback = [=](auto& texture) mutable
				{
					std::string type = texture->GetAssetData()->GetAttribute<std::string>("Type");

					if (!type.empty())
					{
						if (type == "Diffuse")
						{
							TextureDiffuse = texture;
						}
						if (type == "Specular")
						{
							TextureSpecular = texture;
						}
						if (type == "Normal")
						{
							TextureNormals = texture;
						}
					}
				};

			if (behaviour == InstantiationBehaviour::Synchronous)
				AssetManager::GetAsset<Texture2D, InstantiationBehaviour::Synchronous>(dependency->GetId(), AssetMeta::Category::Secondary, BaseAsset::LifeTime::KeepAlive, callback);
			else
				AssetManager::GetAsset<Texture2D, InstantiationBehaviour::Aynchronous>(dependency->GetId(), AssetMeta::Category::Secondary, BaseAsset::LifeTime::KeepAlive, callback);
		}
	}

	auto material = assetData->GetReference();
	if (!material)
		throw std::exception("Material doens't have secondary reference!");

	auto filePath = material->GetAttribute<std::string>("Path");

	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open())
		SHADE_CORE_WARNING("Failed to read file, wrong path = {0}", filePath)
	else
	{
		Deserialize(file);
	}
	file.close();
}

shade::AssetMeta::Type shade::Material::GetAssetStaticType()
{
	return AssetMeta::Type::Material;
}

shade::AssetMeta::Type shade::Material::GetAssetType() const
{
	return GetAssetStaticType();
}

shade::Material* shade::Material::Create(SharedPointer<AssetData> assetData, BaseAsset::LifeTime lifeTime, InstantiationBehaviour behaviour)
{
	return new Material(assetData, lifeTime, behaviour);
}

std::size_t shade::Material::Serialize(std::ostream& stream) const
{
	std::string header = "@s_mat";
	Serializer::Serialize(stream, header);
	Serializer::Serialize(stream, ColorAmbient);
	Serializer::Serialize(stream, ColorDiffuse);
	Serializer::Serialize(stream, ColorSpecular);
	Serializer::Serialize(stream, ColorTransparent);
	Serializer::Serialize(stream, Emmisive);
	Serializer::Serialize(stream, Opacity);
	Serializer::Serialize(stream, Shininess);
	Serializer::Serialize(stream, ShininessStrength);
	Serializer::Serialize(stream, Refracti);
	Serializer::Serialize(stream, NormalMapEnabled);
	Serializer::Serialize(stream, BumpMapEnabled);
	return 0;
}

std::size_t shade::Material::Deserialize(std::istream& stream)
{
	std::string header;
	Serializer::Deserialize(stream, header);
	if (header == "@s_mat")
	{
		Serializer::Deserialize(stream, ColorAmbient);
		Serializer::Deserialize(stream, ColorDiffuse);
		Serializer::Deserialize(stream, ColorSpecular);
		Serializer::Deserialize(stream, ColorTransparent);
		Serializer::Deserialize(stream, Emmisive);
		Serializer::Deserialize(stream, Opacity);
		Serializer::Deserialize(stream, Shininess);
		Serializer::Deserialize(stream, ShininessStrength);
		Serializer::Deserialize(stream, Refracti);
		Serializer::Deserialize(stream, Shading);
		/*Serializer::Deserialize(stream, NormalMapEnabled);
		Serializer::Deserialize(stream, BumpMapEnabled);*/
	}

	return 0;
}
