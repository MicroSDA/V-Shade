#include "shade_pch.h"
#include "BaseAsset.h"
#include <shade/core/asset/AssetManager.h>

std::string shade::AssetMeta::GetTypeAsString(AssetMeta::Type type)
{
	switch (type)
	{
		case AssetMeta::Type::Asset: return "Asset";
		case AssetMeta::Type::Model: return "Model";
		case AssetMeta::Type::Mesh: return "Mesh";
		case AssetMeta::Type::Material: return "Material";
		case AssetMeta::Type::Texture: return "Texture";
		case AssetMeta::Type::Animation: return "AnimationRig";
		case AssetMeta::Type::Skeleton: return "Skeleton";
		case AssetMeta::Type::CollisionShapes: return "CollisionShapes";
		case AssetMeta::Type::Sound: return "Sound";
		case AssetMeta::Type::Other: return "Other";
		default: return "Undefined";
	}
}

shade::AssetMeta::Type shade::AssetMeta::GetTypeFromString(const std::string& type)
{
	if (type == "Model")
		return AssetMeta::Type::Model;
	if (type == "Mesh")
		return AssetMeta::Type::Mesh;
	if (type == "Material")
		return AssetMeta::Type::Material;
	if (type == "Texture")
		return AssetMeta::Type::Texture;
	if (type == "AnimationRig")
		return AssetMeta::Type::Animation;
	if (type == "AnimationRig")
		return AssetMeta::Type::Skeleton;
	if (type == "CollisionShapes")
		return AssetMeta::Type::CollisionShapes;
	if (type == "Sound")
		return AssetMeta::Type::Sound;
	if (type == "Other")
		return AssetMeta::Type::Other;

	return AssetMeta::Type::Asset;
}

std::string shade::AssetMeta::GetCategoryAsString(AssetMeta::Category category)
{
	switch (category)
	{
	case AssetMeta::Category::Primary: return "Primary";
	case AssetMeta::Category::Secondary: return "Secondary";
	case AssetMeta::Category::Blueprint: return "Blueprint";
	case AssetMeta::Category::PrimaryReference: return "PrimaryReference";
	case AssetMeta::Category::SecondaryReference: return "SecondaryReference";
	default: return "None";
	}
}

shade::AssetMeta::Category shade::AssetMeta::GetCategoryFromString(const std::string& category)
{
	if (category == "Primary")
		return AssetMeta::Category::Primary;
	if (category == "Secondary")
		return AssetMeta::Category::Secondary;
	if (category == "Blueprint")
		return AssetMeta::Category::Blueprint;
	if (category == "PrimaryReference")
		return AssetMeta::Category::PrimaryReference;
	if (category == "SecondaryReference")
		return AssetMeta::Category::SecondaryReference;

	return AssetMeta::Category::None;
}

shade::AssetData::AssetData() :
	m_Category(AssetMeta::Category::None), m_Type(AssetMeta::Type::Asset)
{
}

void shade::AssetData::SetCategory(AssetMeta::Category category)
{
	m_Category = category;
}

shade::AssetMeta::Category shade::AssetData::GetCategory() const
{
	return m_Category;
}

void shade::AssetData::SetType(AssetMeta::Type type)
{
	m_Type = type;
}

shade::AssetMeta::Type shade::AssetData::GetType() const
{
	return m_Type;
}

void shade::AssetData::SetId(const std::string& id)
{
	m_Id = id;
}

const std::string& shade::AssetData::GetId() const
{
	return m_Id;
}

void shade::AssetData::SetReference(SharedPointer<AssetData>& data)
{
	m_SecondaryReference = data;
}

shade::SharedPointer<shade::AssetData>& shade::AssetData::GetReference()
{
	return m_SecondaryReference;
}

void shade::AssetData::AddDependency(SharedPointer<AssetData>& dependency)
{
	auto find = std::find(m_Dependencies.begin(), m_Dependencies.end(), dependency);
	if (find == m_Dependencies.end())
	{
		if (this != dependency.Raw())
			m_Dependencies.emplace_back(dependency);
	}
	else
		throw std::exception(std::invalid_argument(std::format("Dependency already present in AssetData !")));
}

const std::vector<shade::SharedPointer<shade::AssetData>>& shade::AssetData::GetDependencies() const
{
	return m_Dependencies;
}

std::vector<shade::SharedPointer<shade::AssetData>>& shade::AssetData::GetDependencies()
{
	return m_Dependencies;
}

const std::unordered_map<std::string, std::string>& shade::AssetData::GetAttributes() const
{
	return m_Attributes;
}

std::unordered_map<std::string, std::string>& shade::AssetData::GetAttributes()
{
	return m_Attributes;
}

std::size_t shade::AssetData::Serialize(std::ostream& stream) const
{
	std::size_t result = 0;
	result += Serializer::Serialize(stream, m_Id);
	result += Serializer::Serialize(stream, std::uint32_t(m_Category));
	result += Serializer::Serialize(stream, std::uint32_t(m_Type));
	if (m_SecondaryReference)
		result += Serializer::Serialize(stream, m_SecondaryReference->GetId());
	else
		result += Serializer::Serialize(stream, m_SecondaryReferenceId);
	result += Serializer::Serialize(stream, m_Attributes);
	result += Serializer::Serialize(stream, m_Dependencies);
	return result;
}

std::size_t shade::AssetData::Deserialize(std::istream& stream)
{
	std::size_t result = 0;
	result += Serializer::Deserialize(stream, m_Id);
	result += Serializer::Deserialize(stream, (std::uint32_t&)m_Category);
	result += Serializer::Deserialize(stream, (std::uint32_t&)m_Type);
	result += Serializer::Deserialize(stream, m_SecondaryReferenceId);
	result += Serializer::Deserialize(stream, m_Attributes);
	result += Serializer::Deserialize(stream, m_Dependencies);
	return result;
}

shade::BaseAsset::BaseAsset(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour) :
	m_AssetData(assetData),m_LifeTime(lifeTime), m_Behaviour(behaviour)
{
}

shade::BaseAsset::~BaseAsset()
{
	if (m_AssetData)
	{
		SHADE_CORE_DEBUG("Asset: '{0}' has been removed successfully.", m_AssetData->GetId());
	}
}

shade::BaseAsset::LifeTime shade::BaseAsset::GetLifeTime()
{
	return m_LifeTime;
}

void shade::BaseAsset::SetAssetData(SharedPointer<AssetData>& data)
{
	m_AssetData = data;
}

const shade::SharedPointer<shade::AssetData>& shade::BaseAsset::GetAssetData() const
{
	return m_AssetData;
}

shade::SharedPointer<shade::AssetData>& shade::BaseAsset::GetAssetData()
{
	return m_AssetData;
}

shade::AssetMeta::Type shade::BaseAsset::GetAssetStaticType()
{
	return AssetMeta::Type::Asset;
}

shade::AssetMeta::Type shade::BaseAsset::GetAssetType() const
{
	return GetAssetStaticType();
}

void shade::BaseAsset::Initialize()
{
	//assert(!m_HasBeenInitialized && "BaseAsset has been alraedy initialized!");
	m_HasBeenInitialized = true;
}

void shade::BaseAsset::LifeTimeManagment()
{
	if (m_HasBeenInitialized)
		AssetManager::ReleaseMe(m_AssetData->GetId(), m_AssetData->GetCategory());
}
