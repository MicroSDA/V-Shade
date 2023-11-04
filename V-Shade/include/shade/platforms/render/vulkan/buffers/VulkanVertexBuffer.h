#pragma once
#include <shade/core/render/buffers/VertexBuffer.h>
#include <shade/platforms/render/vulkan/VulkanContext.h>
#include <shade/platforms/render/vulkan/buffers/VulkanCommandBuffer.h>

namespace shade
{
	class VulkanVertexBuffer : public VertexBuffer
	{
	public:
		VulkanVertexBuffer(const VkDevice& device, VulkanContext::VulkanInstance& instance, Usage usage, std::uint32_t size, std::size_t resizeThreshold, const void* data);
		virtual ~VulkanVertexBuffer();
	public:
		virtual void SetData(std::uint32_t size, const void* data, std::uint32_t offset = 0) override;
		virtual void Resize(std::uint32_t size) override;
		virtual void Bind(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex, std::uint32_t binding, std::uint32_t offset) const override;
	public:
		const VkBuffer* GetBuffer() const;
	private:
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_DeviceMemory = VK_NULL_HANDLE;

		VkBuffer m_StagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_StagingDeviceMemory = VK_NULL_HANDLE;

		VkDevice m_VkDevice = VK_NULL_HANDLE;
		VulkanContext::VulkanInstance m_VkInstance;
	private:
		std::tuple<VkBuffer, VkDeviceMemory> CreateBuffer(VkDevice device, const VkAllocationCallbacks* pAllocator, const VkBufferCreateInfo* pCreateInfo, const VkMemoryPropertyFlags& properties);
		void Invalidate(Usage usage, std::uint32_t size, std::size_t resizeThreshold);
	private:
	};

}