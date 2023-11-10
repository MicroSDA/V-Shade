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

	File file(filePath, File::In, "@s_mat", File::VERSION(0, 0, 1));

	if (!file.IsOpen())
		SHADE_CORE_WARNING("Failed to read file, wrong path = {0}", filePath)
	else
		file.Read(*this);

	file.CloseFile();
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
	std::size_t size = Serializer::Serialize(stream, ColorAmbient);
	
	size += Serializer::Serialize(stream, ColorDiffuse);
	size += Serializer::Serialize(stream, ColorSpecular);
	size += Serializer::Serialize(stream, ColorTransparent);
	size += Serializer::Serialize(stream, Emmisive);
	size += Serializer::Serialize(stream, Opacity);
	size += Serializer::Serialize(stream, Shininess);
	size += Serializer::Serialize(stream, ShininessStrength);
	size += Serializer::Serialize(stream, Refracti);
	size += Serializer::Serialize(stream, NormalMapEnabled);
	size += Serializer::Serialize(stream, BumpMapEnabled);

	return size;
}

std::size_t shade::Material::Deserialize(std::istream& stream)
{
	std::size_t size = Serializer::Deserialize(stream, ColorAmbient);
	size += Serializer::Deserialize(stream, ColorDiffuse);
	size += Serializer::Deserialize(stream, ColorSpecular);
	size += Serializer::Deserialize(stream, ColorTransparent);
	size += Serializer::Deserialize(stream, Emmisive);
	size += Serializer::Deserialize(stream, Opacity);
	size += Serializer::Deserialize(stream, Shininess);
	size += Serializer::Deserialize(stream, ShininessStrength);
	size += Serializer::Deserialize(stream, Refracti);
	size += Serializer::Deserialize(stream, Shading);
	/*size +=Serializer::Deserialize(stream, NormalMapEnabled);
	size +=Serializer::Deserialize(stream, BumpMapEnabled);*/

	return size;
}
