#include "shade_pch.h"
#include "Texture.h"

#include <shade/core/render/RenderAPI.h>
#include <shade/platforms/render/vulkan/VulkanTexture2D.h>

shade::Texture2D* shade::Texture2D::Create(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour)
{
	switch (RenderAPI::GetCurrentAPI())
	{
		case RenderAPI::API::None:  SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
		case RenderAPI::API::Vulkan: return new VulkanTexture2D(VulkanContext::GetDevice()->GetLogicalDevice(),VulkanContext::GetInstance(), assetData, lifeTime, behaviour);
		default: SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	}
}

shade::SharedPointer<shade::render::Image2D>& shade::Texture2D::GetImage()
{
	return m_Image;
}

shade::SharedPointer<shade::Texture2D> shade::Texture2D::CreateEXP(const SharedPointer<render::Image2D>& image)
{
	switch (RenderAPI::GetCurrentAPI())
	{
		case RenderAPI::API::None:  SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
		case RenderAPI::API::Vulkan: return SharedPointer<VulkanTexture2D>::Create(VulkanContext::GetDevice()->GetLogicalDevice(), VulkanContext::GetInstance(), image);
		default: SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	}
}

shade::SharedPointer<shade::Texture2D> shade::Texture2D::CreateEXP(const render::Image::Specification& specification)
{
	switch (RenderAPI::GetCurrentAPI())
	{
		case RenderAPI::API::None:  SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
		case RenderAPI::API::Vulkan: return SharedPointer<VulkanTexture2D>::Create(VulkanContext::GetDevice()->GetLogicalDevice(), VulkanContext::GetInstance(), specification);
		default: SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	}
}

shade::Texture2D::Texture2D(SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour) : BaseAsset(assetData, lifeTime, behaviour)
{
	
	auto filePath = assetData->GetAttribute<std::string>("Path");

	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open())
		SHADE_CORE_WARNING("Failed to read file, wrong path = {0}", filePath)
	else
	{
		render::Image image;
		Serializer::Deserialize(file, image);
		m_Image = render::Image2D::Create(image);
	}
	file.close();
}

shade::Texture2D::Texture2D(const SharedPointer<render::Image2D>& image)
{
	m_Image = image;
}

shade::Texture2D::Texture2D(const render::Image::Specification& specification)
{
	m_Image = render::Image2D::Create(specification);
}
shade::AssetMeta::Type shade::Texture2D::GetAssetStaticType()
{
	return AssetMeta::Texture;
}

shade::AssetMeta::Type shade::Texture2D::GetAssetType() const
{
	return GetAssetStaticType();
}
