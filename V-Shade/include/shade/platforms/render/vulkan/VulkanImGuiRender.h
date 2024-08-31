#pragma once
#include <shade/core/layer/imgui/ImGuiRender.h>
#include <shade/platforms/render/vulkan/VulkanContext.h>
#include <shade/platforms/render/vulkan/buffers/VulkanCommandBuffer.h>
#include <shade/platforms/render/vulkan/VulkanSwapChain.h>
#include <shade/platforms/render/vulkan/VulkanImage2D.h>
#include <vulkan/vulkan.h>

namespace shade
{
	class VulkanImGuiRender : public ImGuiRender
	{
	public:
		VulkanImGuiRender();
		virtual ~VulkanImGuiRender();
		virtual void BeginRender() override;
		virtual void EndRender() override;

		virtual void DrawImage(SharedPointer<Texture2D>& texture, const ImVec2& size, const ImVec4& borderColor) override;
		virtual void DrawImage(SharedPointer<Texture2D>& texture, const ImVec2& size, const ImVec4& borderColor, std::uint32_t mip) override;
		virtual void DrawImage(Asset<Texture2D>& texture, const ImVec2& size, const ImVec4& borderColor) override;
	private:
		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
		SharedPointer<RenderCommandBuffer> m_CommandBuffer;
		VkDescriptorSet	m_Image = VK_NULL_HANDLE;
		VkPipeline m_ImGuiPipeline = VK_NULL_HANDLE;
		std::unordered_map<Texture2D*, std::pair<VkDescriptorSet, VkImageView>> m_Images;
	};
}