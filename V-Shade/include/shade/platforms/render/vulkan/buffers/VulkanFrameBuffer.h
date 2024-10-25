#pragma once
#include <shade/core/render/buffers/FrameBuffer.h>
#include <vulkan/vulkan.h>
#include <shade/platforms/render/vulkan/VulkanImage2D.h>
#include <shade/platforms/render/vulkan/VulkanTexture2D.h>
#include <shade/platforms/render/vulkan/buffers/VulkanCommandBuffer.h>

namespace shade
{
	class VulkanFrameBuffer : public FrameBuffer
	{
	public:
		VulkanFrameBuffer(const VkDevice& device, const VulkanContext::VulkanInstance& instance, const FrameBuffer::Specification& specification);
		VulkanFrameBuffer(const VkDevice& device, const VulkanContext::VulkanInstance& instance, const FrameBuffer::Specification& specification, const std::vector<SharedPointer<render::Image2D>>& images);
		virtual ~VulkanFrameBuffer();

		const std::vector<VkImageView>& GetAttachmentViews() const;
		const VkRenderingInfo& GetRenderingInfo() const;

		virtual SharedPointer<Texture2D>& GetTextureAttachment(std::uint32_t index) override;
		virtual SharedPointer<Texture2D>& GetDepthAttachment(std::uint32_t index) override;

		virtual std::vector<SharedPointer<Texture2D>>& GetTextureAttachments() override;
		virtual std::vector<SharedPointer<Texture2D>>& GetDepthAttachments() override;

		virtual std::uint32_t GetWidth()  const override;
		virtual std::uint32_t GetHeight() const override;

		virtual void Resize(std::uint32_t width, uint32_t height) override;
		virtual void Resize(std::uint32_t width, uint32_t height, const std::vector<SharedPointer<render::Image2D>>& images) override;

		// In case we want to clear attahcment manually we need to specify clear attahcment data for it.
		const std::vector<VkClearAttachment>& GetClearAttachments() const;

	private:
		SharedPointer<RenderCommandBuffer> m_CommandBuffer;

		std::vector<SharedPointer<Texture2D>> m_TextureAttachments;
		std::vector<VkImageView> m_ColorAttachmentsViews;

		std::vector<SharedPointer<Texture2D>> m_DepthAttachments;
		std::vector<VkImageView> m_DepthAttachmentsViews;

		std::vector<VkAttachmentDescription> m_AttachmentDescriptions;

		std::vector<VkRenderingAttachmentInfo> m_RenderingColorAttachmentInfos;
		std::vector<VkRenderingAttachmentInfo> m_RenderingDepthAttachmentInfos;

		std::vector<VkClearAttachment> m_ClearAttachments;
		VkRenderingInfo m_RenderingInfo;
		VkAttachmentSampleCountInfoNV m_SamplesCountInfoNV;
	
		VkDevice m_VkDevice = VK_NULL_HANDLE;
		VulkanContext::VulkanInstance m_VkInstance;
	private:
		void AllocateImageMemory(VkDevice device, VkImage image);

		void Invalidate(const FrameBuffer::Specification& specification, const std::vector<SharedPointer<render::Image2D>>* images);
		friend class VulkanSwapChain;
	};
}