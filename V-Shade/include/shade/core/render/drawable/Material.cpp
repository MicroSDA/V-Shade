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
	if (material)
	{
		auto filePath = material->GetAttribute<std::string>("Path");

		if (file::File file = file::FileManager::LoadFile(filePath, "@s_mat"))
		{
			file.Read(*this);
		}
		else
		{
			SHADE_CORE_WARNING("Failed to read file, wrong path = {0}", filePath)
		}
	}
	else
	{
		SHADE_CORE_WARNING("Material '{0}' dosen't have secondary reference!", assetData->GetId());
	}
}

void shade::Material::Serialize(std::ostream& stream) const
{
	serialize::Serializer::Serialize(stream, ColorAmbient);
	serialize::Serializer::Serialize(stream, ColorDiffuse);
	serialize::Serializer::Serialize(stream, ColorSpecular);
	serialize::Serializer::Serialize(stream, ColorTransparent);
	serialize::Serializer::Serialize(stream, Emmisive);
	serialize::Serializer::Serialize(stream, Opacity);
	serialize::Serializer::Serialize(stream, Shininess);
	serialize::Serializer::Serialize(stream, ShininessStrength);
	serialize::Serializer::Serialize(stream, Refracti);
	serialize::Serializer::Serialize(stream, NormalMapEnabled);
	serialize::Serializer::Serialize(stream, BumpMapEnabled);
}

void shade::Material::Deserialize(std::istream& stream)
{
	serialize::Serializer::Deserialize(stream, ColorAmbient);
	serialize::Serializer::Deserialize(stream, ColorDiffuse);
	serialize::Serializer::Deserialize(stream, ColorSpecular);
	serialize::Serializer::Deserialize(stream, ColorTransparent);
	serialize::Serializer::Deserialize(stream, Emmisive);
	serialize::Serializer::Deserialize(stream, Opacity);
	serialize::Serializer::Deserialize(stream, Shininess);
	serialize::Serializer::Deserialize(stream, ShininessStrength);
	serialize::Serializer::Deserialize(stream, Refracti);
	serialize::Serializer::Deserialize(stream, Shading);
	serialize::Serializer::Deserialize(stream, NormalMapEnabled);
	serialize::Serializer::Deserialize(stream, BumpMapEnabled);
}
