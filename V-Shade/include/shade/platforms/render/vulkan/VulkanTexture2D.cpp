#include "shade_pch.h"
#include "VulkanTexture2D.h"

shade::VulkanTexture2D::VulkanTexture2D(const VkDevice device, const VulkanContext::VulkanInstance& instnace, SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour) :
	Texture2D(assetData, lifeTime, behaviour), m_VkDevice(device), m_VkInstance(instnace)
{
	Invalidate();
}

shade::VulkanTexture2D::VulkanTexture2D(const VkDevice device, const VulkanContext::VulkanInstance& instnace, const SharedPointer<render::Image2D>& image) : Texture2D(image),
	m_VkDevice(device), m_VkInstance(instnace)
{
	Invalidate();
}

shade::VulkanTexture2D::VulkanTexture2D(const VkDevice device, const VulkanContext::VulkanInstance& instnace, const render::Image::Specification& specification) : Texture2D(specification),
	m_VkDevice(device), m_VkInstance(instnace)
{
	Invalidate();
}

shade::VulkanTexture2D::~VulkanTexture2D()
{
	if (m_Sampler != VK_NULL_HANDLE)
		vkDestroySampler(m_VkDevice, m_Sampler, m_VkInstance.AllocationCallbaks);
}

std::uint32_t shade::VulkanTexture2D::GetWidth()  const
{
    return m_Image->GetSpecification().Width;
}

std::uint32_t shade::VulkanTexture2D::GetHeight() const 
{
	return m_Image->GetSpecification().Height;
}

void shade::VulkanTexture2D::Resize(std::uint32_t width, std::uint32_t height, std::uint32_t mipCount)
{
	if (width > 1 && height > 1)
	{
		m_Image->Resize(width, height, mipCount);
		Invalidate();
	}
}

VkSampler shade::VulkanTexture2D::GetSampler() const
{
	return m_Sampler;
}

VkDescriptorImageInfo shade::VulkanTexture2D::GetDescriptorImageInfo()
{
	return { .sampler = m_Sampler, .imageView = m_Image->As<VulkanImage2D>().GetImageView(), .imageLayout = m_Image->As<VulkanImage2D>().GetImageLayout() };
}

VkDescriptorImageInfo shade::VulkanTexture2D::GetDescriptorImageInfoMip(std::uint32_t mipLevel)
{
	return { .sampler = m_Sampler, .imageView = m_Image->As<VulkanImage2D>().GetImageViewPerMipLevel(mipLevel), .imageLayout = m_Image->As<VulkanImage2D>().GetImageLayout()};
}
VkDescriptorImageInfo shade::VulkanTexture2D::GetDescriptorImageInfoLayer(std::uint32_t layer)
{
	return { .sampler = m_Sampler, .imageView = m_Image->As<VulkanImage2D>().GetImageViewPerLayer(layer), .imageLayout = m_Image->As<VulkanImage2D>().GetImageLayout() };
}

shade::VulkanDescriptorSet& shade::VulkanTexture2D::GetDescriptorSet()
{
	return *m_DescriptorSet;
}

void shade::VulkanTexture2D::Invalidate()
{
	if(m_Sampler != VK_NULL_HANDLE)
		vkDestroySampler(m_VkDevice, m_Sampler, m_VkInstance.AllocationCallbaks);

	VkSamplerCreateInfo samplerCreateInfo
	{
		VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO, // sType
		VK_NULL_HANDLE, // pNext
		0,

		VK_FILTER_MAX_ENUM,
		VK_FILTER_MAX_ENUM,
		VK_SAMPLER_MIPMAP_MODE_MAX_ENUM,
		(m_Image->GetSpecification().IsCubeMap) ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : VK_SAMPLER_ADDRESS_MODE_REPEAT,
		samplerCreateInfo.addressModeU,
		samplerCreateInfo.addressModeU,
		0.0f, //mipLodBias
		VK_TRUE, // anisotropyEnable // TODO: TAKE A LOOK
		16.0f, // maxAnisotropy
		VK_FALSE, // compareEnable // TODO: TAKE A LOOK
		VK_COMPARE_OP_MAX_ENUM, // compareOp
		0.0f, // minLod
		1.f, // maxLod
		VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE, // borderColor // TODO: TAKE A LOOK
		VK_FALSE, // unnormalizedCoordinates // TODO: TAKE A LOOK
	};

	if (m_Image->GetSpecification().Format <= render::Image::Format(5))
	{
		samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	}
	else
	{
		samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}

	VK_CHECK_RESULT(vkCreateSampler(m_VkDevice, &samplerCreateInfo, m_VkInstance.AllocationCallbaks, &m_Sampler), "Failed to create image sampler!");
	VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Iamge sampler", (std::uint32_t)&m_Sampler), m_VkDevice, VK_OBJECT_TYPE_SAMPLER, m_Sampler);

	/*if (m_Image->GetSpecification().Format == render::Image::Format::DEPTH24STENCIL8 || m_Image->GetSpecification().Format == render::Image::Format::DEPTH32F || m_Image->GetSpecification().Format == render::Image::Format::DEPTH32FSTENCIL8UINT)
		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	else if (m_Image->GetSpecification().Usage == render::Image::Usage::Texture || m_Image->GetSpecification().Usage == render::Image::Usage::Attachment)
		m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;*/

	m_DescriptorImageInfo.imageView		= m_Image->As<VulkanImage2D>().GetImageView();
	m_DescriptorImageInfo.sampler		= m_Sampler;
	m_DescriptorImageInfo.imageLayout	= m_Image->As<VulkanImage2D>().GetImageLayout();
}
