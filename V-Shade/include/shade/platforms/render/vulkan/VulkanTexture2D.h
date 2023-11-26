#pragma once
#include <shade/core/image/Texture.h>
#include <shade/platforms/render/vulkan/VulkanContext.h>
#include <shade/platforms/render/vulkan/VulkanImage2D.h>
#include <shade/platforms/render/vulkan/descriptors/VulkanDescriptor.h>

namespace shade
{
	class VulkanTexture2D : public Texture2D
	{
	public:
		// To create through AssetManager
		VulkanTexture2D(const VkDevice device, const VulkanContext::VulkanInstance& instnace, SharedPointer<AssetData> assetData, LifeTime lifeTime, InstantiationBehaviour behaviour);
		// To create directly.
		VulkanTexture2D(const VkDevice device, const VulkanContext::VulkanInstance& instnace, const SharedPointer<render::Image2D>& image);
		VulkanTexture2D(const VkDevice device, const VulkanContext::VulkanInstance& instnace, const render::Image::Specification& specification);

		virtual ~VulkanTexture2D();
	public:
		virtual std::uint32_t GetWidth() const override;
		virtual std::uint32_t GetHeight() const override;

		virtual void Resize(std::uint32_t width, std::uint32_t height, std::uint32_t mipCount = 1) override;

		VkSampler GetSampler() const;
		VkDescriptorImageInfo GetDescriptorImageInfo();
		VkDescriptorImageInfo GetDescriptorImageInfoMip(std::uint32_t mipLevel = 0);
		VkDescriptorImageInfo GetDescriptorImageInfoLayer(std::uint32_t layer  = 0);
	private:
		VkSampler m_Sampler = VK_NULL_HANDLE;
		VkDevice m_VkDevice = VK_NULL_HANDLE;
		VulkanContext::VulkanInstance m_VkInstance;
		VkDescriptorImageInfo m_DescriptorImageInfo;
	private:
		void Invalidate();
	};
}
