#pragma once
#include <shade/core/render/buffers/RenderCommandBuffer.h>
#include <shade/platforms/render/vulkan/VulkanShader.h>
#include <shade/platforms/render/vulkan/VulkanPipeline.h>

namespace shade
{
	class VulkanCommandBuffer : public RenderCommandBuffer
	{
	public:
		VulkanCommandBuffer(const Type& type, const Family& family, const std::uint32_t& count , const std::string& name);

		virtual ~VulkanCommandBuffer();
		virtual void Begin(std::uint32_t index = 0) override;
		virtual void End(std::uint32_t index = 0) override;
		virtual void Submit(std::uint32_t index = 0, std::uint32_t timeout = UINT32_MAX) override;

		VkCommandBuffer GetCommandBuffer(std::uint32_t index = 0);
		VkFence GetFence(std::uint32_t index = 0);

	private:
		struct CommandBuffer
		{
			VkCommandPool Pool = VK_NULL_HANDLE;
			VkCommandBuffer Buffer = VK_NULL_HANDLE;
		};
		std::vector<CommandBuffer> m_CommandBuffers;
		std::vector<VkFence> m_WaitFences;

		SharedPointer<VulkanDevice> m_VkDevice;
		SharedPointer<VulkanPhysicalDevice> m_VkPhysicalDevice;
		VulkanContext::VulkanInstance m_VkInstance;
	private:
		
	};
}