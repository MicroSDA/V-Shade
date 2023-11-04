#pragma once
#include <shade/core/render/buffers/IndexBuffer.h>
#include <shade/platforms/render/vulkan/VulkanContext.h>
#include <shade/platforms/render/vulkan/buffers/VulkanCommandBuffer.h>

namespace shade
{
	class VulkanIndexBuffer : public IndexBuffer
	{
	public:
		VulkanIndexBuffer(const VkDevice& device, const VulkanContext::VulkanInstance& instance, Usage usage, std::uint32_t size, std::uint32_t resizeThreshold, const void* data);
		virtual ~VulkanIndexBuffer();

		virtual std::uint32_t GetCount() const override;
		virtual void Bind(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex, std::uint32_t offset) const override;
		virtual void Resize(std::uint32_t size) override;
		virtual void SetData(std::uint32_t size, const void* data, std::uint32_t offset) override;

		const VkBuffer GetBuffer() const;
	private:
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_DeviceMemory = VK_NULL_HANDLE;
		VkBuffer m_StagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_StagingDeviceMemory = VK_NULL_HANDLE;
		VkDevice m_VkDevice = VK_NULL_HANDLE;
		VulkanContext::VulkanInstance m_VkInstance;
		std::uint32_t m_Count = 0;
	private:
		void Invalidate(Usage usage, std::uint32_t size, std::uint32_t resizeThreshold);
		std::tuple<VkBuffer, VkDeviceMemory> CreateBuffer(VkDevice device, const VkAllocationCallbacks* pAllocator, const VkBufferCreateInfo* pCreateInfo, const VkMemoryPropertyFlags& properties);
	};
}

