#pragma once
#include <shade/core/render/buffers/StorageBuffer.h>
#include <shade/platforms/render/vulkan/VKUtils.h>
#include <shade/platforms/render/vulkan/VulkanContext.h>
#include <shade/platforms/render/vulkan/buffers/VulkanCommandBuffer.h>

namespace shade
{
	class VulkanStorageBuffer : public StorageBuffer
	{
	public:
		VulkanStorageBuffer(const VkDevice& device, VulkanContext::VulkanInstance& instance, Usage usage, std::uint32_t binding, std::uint32_t size, std::uint32_t framesCount, std::size_t resizeThreshold);
		virtual ~VulkanStorageBuffer();
		virtual void SetData(std::uint32_t size, const void* data, std::uint32_t frameIndex = 1, std::uint32_t offset = 0) override;
		virtual void Resize(std::uint32_t size) override;
		
		virtual std::uint32_t GetBinding() override;
		const VkBuffer GetBuffer() const;
		const std::vector<VkDescriptorBufferInfo>& GetDescriptorBufferInfos() { return m_DescriptorBufferInfos; }
		// TODO: May throw exeption
		const VkDescriptorBufferInfo& GetDescriptorBufferInfo(std::uint32_t frameIndex) { return m_DescriptorBufferInfos[frameIndex]; }
	private:
		std::uint32_t m_Binding = 0, m_FramesCount = 0, m_Size = 0;

		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VkDeviceMemory m_DeviceMemory = VK_NULL_HANDLE;
		VkBuffer m_StagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory m_StagingDeviceMemory = VK_NULL_HANDLE;
		std::vector<VkDescriptorBufferInfo> m_DescriptorBufferInfos;

		VkDevice m_VkDevice = VK_NULL_HANDLE;
		VulkanContext::VulkanInstance m_VkInstance;
		std::uint32_t m_AlignmentSize = 0;
	private:
		std::tuple<VkBuffer, VkDeviceMemory> CreateBuffer(VkDevice device, const VkAllocationCallbacks* pAllocator, const VkBufferCreateInfo* pCreateInfo, const VkMemoryPropertyFlags& properties);
		std::uint32_t GetBufferMinAlignment(std::size_t size);
		void Invalidate(std::uint32_t binding, std::uint32_t size, std::uint32_t framesCount, std::uint32_t resizeThreshold);
	};
}
