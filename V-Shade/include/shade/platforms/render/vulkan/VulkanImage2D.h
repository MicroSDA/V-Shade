#pragma once
#include <shade/core/image/Image.h>
#include <shade/platforms/render/vulkan/VKUtils.h>
#include <shade/platforms/render/vulkan/VulkanContext.h>
#include <shade/platforms/render/vulkan/buffers/VulkanCommandBuffer.h>

namespace shade
{
	class VulkanImage2D : public render::Image2D
	{
	public:
		VulkanImage2D(const VkDevice device, const VulkanContext::VulkanInstance& instnace, const render::Image::Specification& specification);
		VulkanImage2D(const VkDevice device, const VulkanContext::VulkanInstance& instnace, render::Image& source);
		VulkanImage2D(const VkDevice device, const VulkanContext::VulkanInstance& instnace, const render::Image::Specification& specification, const void* source);
		
		virtual ~VulkanImage2D(); 
		virtual void Resize(std::uint32_t width, std::uint32_t height, std::uint32_t mipCount) override;

		void LayoutTransition(SharedPointer<RenderCommandBuffer>& commandBuffer, VkImageLayout newLayout, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, std::uint32_t mipBaselevel = 0, std::uint32_t mipLevels = 1, std::uint32_t layerBseLevel = 0, std::uint32_t layerCount = 1, std::uint32_t frameIndex = 0);
	
		// TODO: Create non const variation if needed !
		VkImageLayout GetImageLayout() const;
		VkImageView GetImageViewPerMipLevel(std::uint32_t mip = 0) const; 
		VkImageView GetImageViewPerLayer(std::uint32_t layer = 0) const; 
		VkImageView GetImageView() const; 
		VkImage GetImage() const;
		VkImageAspectFlags GetAspectMask() const;
	private:
		VkImage m_VkImage = VK_NULL_HANDLE;
		std::vector<VkImageView> m_VkImageViewsMips;
		std::vector<VkImageView> m_VkImageViewsLayers;
		VkImageView m_VkImageView = VK_NULL_HANDLE;
		VkDeviceMemory m_VkImageMemory = VK_NULL_HANDLE;
		VkDevice m_VkDevice = VK_NULL_HANDLE;
		VulkanContext::VulkanInstance m_VkInstance;
		VkImageLayout m_ImageLayout;
		VkFormat m_ImageFormat = VK_FORMAT_UNDEFINED;
		VkImageAspectFlags m_AspectFlags;
		bool m_IsDepth = false;
		bool m_IsDepthStencil = false;


		VkDeviceSize m_GPUMemorySize = 0;
		bool m_RemouteDestorying = false;
	private:
		std::tuple<VkBuffer, VkDeviceMemory> CreateBuffer(VkDevice device, const VkAllocationCallbacks* pAllocator, const VkBufferCreateInfo* pCreateInfo, const VkMemoryPropertyFlags& properties);
		//void Invalidate(const render::Image::Specification& spec, render::Image* image = nullptr, const void* source = nullptr);

		void Invalidate(const render::Image::Specification& spec);
		void Invalidate(render::Image& image);
		void Invalidate(const render::Image::Specification& specification, const void* source);
	};
}
