#include "shade_pch.h"
#include "VulkanStorageBuffer.h"

shade::VulkanStorageBuffer::VulkanStorageBuffer(const VkDevice& device, VulkanContext::VulkanInstance& instance, Usage usage, std::uint32_t binding, std::uint32_t size, std::uint32_t framesCount, std::size_t resizeThreshold) :
	m_Binding(binding), 
	m_Size(size), 
	m_FramesCount(framesCount),
	m_VkDevice(device),
	m_VkInstance(instance)
{
	m_Usage = usage;
	Invalidate(binding, size, framesCount, resizeThreshold);
}

shade::VulkanStorageBuffer::~VulkanStorageBuffer()
{
	if (m_Buffer != VK_NULL_HANDLE)
	{
		vkFreeMemory(m_VkDevice, m_DeviceMemory, m_VkInstance.AllocationCallbaks);
		vkDestroyBuffer(m_VkDevice, m_Buffer, m_VkInstance.AllocationCallbaks);
	}
}

void shade::VulkanStorageBuffer::Invalidate(std::uint32_t binding, std::uint32_t size, std::uint32_t framesCount, std::uint32_t resizeThreshold)
{
	m_AlignmentSize = GetBufferMinAlignment(size);
	m_Size = size;
	m_Binding = binding;
	m_ResizeThreshold = resizeThreshold;
	m_FramesCount = framesCount;

	m_DescriptorBufferInfos.resize(framesCount);

	if (m_Buffer != VK_NULL_HANDLE)
	{
		vkFreeMemory(m_VkDevice, m_DeviceMemory, m_VkInstance.AllocationCallbaks);
		vkDestroyBuffer(VulkanContext::GetLogicalDevice()->GetDevice(), m_Buffer, m_VkInstance.AllocationCallbaks);
		m_Buffer = VK_NULL_HANDLE;
		m_DeviceMemory = VK_NULL_HANDLE;
	}

	static std::uint32_t queus[2] = { VulkanContext::GetPhysicalDevice()->GetQueueFamilyIndices().Graphics, VulkanContext::GetPhysicalDevice()->GetQueueFamilyIndices().Compute };
	
	VkBufferCreateInfo bufferCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = 0,
		.size = static_cast<std::uint64_t>(m_AlignmentSize * m_FramesCount),
		.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	// Use CreateBuffer function to create a buffer for the device also with the memory allocation to it.
	auto buffer = CreateBuffer(m_VkDevice, m_VkInstance.AllocationCallbaks, &bufferCreateInfo,
		(m_Usage == Usage::GPU) ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	// Store the allocated buffer and device memory.
	m_Buffer = std::get<0>(buffer);
	m_DeviceMemory = std::get<1>(buffer);

	for (auto i = 0; i < m_DescriptorBufferInfos.size(); i++)
	{
		m_DescriptorBufferInfos[i].buffer	= m_Buffer;
		m_DescriptorBufferInfos[i].offset	= m_AlignmentSize * i;
		m_DescriptorBufferInfos[i].range	= m_AlignmentSize;
	}
}

void shade::VulkanStorageBuffer::SetData(std::uint32_t size, const void* data, std::uint32_t frmaeIndex, std::uint32_t offset)
{
	if (size)
	{
		// if the usage is GPU
		if (m_Usage == Usage::GPU)
		{
			// check if the staging buffer is not null
			if (m_StagingBuffer != VK_NULL_HANDLE)
			{
				// free the memory of the staging device and allocation callbacks
				vkFreeMemory(m_VkDevice, m_StagingDeviceMemory, m_VkInstance.AllocationCallbaks);
				// destroy the staging buffer and allocation callbacks
				vkDestroyBuffer(m_VkDevice, m_StagingBuffer, m_VkInstance.AllocationCallbaks);
			}
			// Create a buffer with properties for use as a staging buffer.
			VkBufferCreateInfo stagingbufferCreateInfo
			{
				.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.pNext = VK_NULL_HANDLE,
				.flags = 0,
				.size = static_cast<std::uint64_t>(size),
				.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.queueFamilyIndexCount = 0,
				.pQueueFamilyIndices = VK_NULL_HANDLE
			};

			// Use CreateBuffer function to make staging buffer and allocate memory to it.
			auto stagingBuffer = CreateBuffer(m_VkDevice, m_VkInstance.AllocationCallbaks, &stagingbufferCreateInfo,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			// Store the staging buffer and its device memory.
			m_StagingBuffer = std::get<0>(stagingBuffer);
			m_StagingDeviceMemory = std::get<1>(stagingBuffer);

			// Map the staging device memory to pData to copy the data to it.
			void* pData; vkMapMemory(m_VkDevice, m_StagingDeviceMemory, 0, size, 0, &pData);
			memcpy(pData, data, std::size_t(size)); // TIP: Crash here !
			vkUnmapMemory(m_VkDevice, m_StagingDeviceMemory);

			// Create RenderCommandBuffer to include in buffer copying.
			auto copyComamndBuffer = RenderCommandBuffer::Create(RenderCommandBuffer::Type::Primary, RenderCommandBuffer::Family::Transfer, 1);

			// Create and initiate CommandBufferBeginInfo and BufferCopy structs.
			VkCommandBufferBeginInfo copyCommandBufferBeginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
			VkBufferCopy copyRegion{ 0, (frmaeIndex * m_AlignmentSize) + offset, size };

			// Begin the copy command buffer, copy staging buffer data to device buffer, and end.
			copyComamndBuffer->Begin();
			vkCmdCopyBuffer(copyComamndBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(), m_StagingBuffer, m_Buffer, 1, &copyRegion);
			copyComamndBuffer->End();

			// Submit the copy command buffer, flush the staging buffer data to the device buffer.
			copyComamndBuffer->Submit();

			// Free the staging memory and destroy the staging buffer.
			vkFreeMemory(m_VkDevice, m_StagingDeviceMemory, m_VkInstance.AllocationCallbaks);
			vkDestroyBuffer(m_VkDevice, m_StagingBuffer, m_VkInstance.AllocationCallbaks);

			// Set the staging buffer and device memory to null.
			m_StagingBuffer = VK_NULL_HANDLE;
			m_StagingDeviceMemory = VK_NULL_HANDLE;
		}
		else
		{
			// Map the staging device memory to pData to copy the data to it.
			const std::uint32_t newOffset = (frmaeIndex * m_AlignmentSize) + offset;
			void* pData; vkMapMemory(m_VkDevice, m_DeviceMemory, newOffset, size, 0, &pData);
			memcpy(pData, data, std::size_t(size));
			vkUnmapMemory(m_VkDevice, m_DeviceMemory);
		}
	}
	else
	{
		// vkMapMemory() : Attempting to map memory range of size zero
	}
}

void shade::VulkanStorageBuffer::Resize(std::uint32_t size)
{
	// If size is not null and the current size must be resized according to the resize threshold
	if (size && HasToBeResized(m_Size, size, m_ResizeThreshold))
	{
		// Invalidate the object taking into account the usage, new size and the resize threshold
		Invalidate(m_Binding, size, m_FramesCount, m_ResizeThreshold);
	}
}

const VkBuffer shade::VulkanStorageBuffer::GetBuffer() const
{
	return m_Buffer;
}

std::tuple<VkBuffer, VkDeviceMemory> shade::VulkanStorageBuffer::CreateBuffer(VkDevice device, const VkAllocationCallbacks* pAllocator, const VkBufferCreateInfo* pCreateInfo, const VkMemoryPropertyFlags& properties)
{
	VkBuffer buffer;
	VK_CHECK_RESULT(vkCreateBuffer(device, pCreateInfo, pAllocator, &buffer), "Failed to create buffer!");
	VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Storage buffer", (std::uint32_t)buffer), device, VK_OBJECT_TYPE_BUFFER, buffer);

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo
	{
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		VK_NULL_HANDLE,
		memRequirements.size,
		VKUtils::FindMemoryType(VulkanContext::GetPhysicalDevice()->GetDevice(), memRequirements.memoryTypeBits,
			properties)
	};

	VkDeviceMemory bufferMemory;

	VK_CHECK_RESULT(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &bufferMemory), "Failed to allocate memory!");
	VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Storage buffer memory", (std::uint32_t)bufferMemory), device, VK_OBJECT_TYPE_DEVICE_MEMORY, bufferMemory);

	vkBindBufferMemory(device, buffer, bufferMemory, 0);

	return { buffer, bufferMemory };
}

std::uint32_t shade::VulkanStorageBuffer::GetBufferMinAlignment(std::size_t size)
{
	// Calculate required alignment based on minimum device offset alignment
	size_t minSboAlignment = VulkanContext::GetPhysicalDevice()->GetPhysicalDeviceProperties().limits.minStorageBufferOffsetAlignment;
	size_t alignedSize = size;
	if (minSboAlignment > 0)
		alignedSize = (alignedSize + minSboAlignment - 1) & ~(minSboAlignment - 1);

	return alignedSize;
}

std::uint32_t shade::VulkanStorageBuffer::GetBinding()
{
	return m_Binding;
}
