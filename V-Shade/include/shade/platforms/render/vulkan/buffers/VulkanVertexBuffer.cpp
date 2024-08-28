#include "shade_pch.h"
#include "VulkanVertexBuffer.h"

shade::VulkanVertexBuffer::VulkanVertexBuffer(const VkDevice& device, VulkanContext::VulkanInstance& instance, Usage usage, std::uint32_t size, std::size_t resizeThreshold, const void* data) :
	m_VkDevice(device), m_VkInstance(instance)
{
	Invalidate(usage, size, resizeThreshold);
	// If data is not null, calls SetData to specify the vertex data for the buffer. 
	// 0 represents the byte offset in the buffer to begin writing data. 
	if (data)
		SetData(size, data, 0);
}

shade::VulkanVertexBuffer::~VulkanVertexBuffer()
{
	// If the buffer is not null handle
	if (m_Buffer != VK_NULL_HANDLE)
	{
		// Free memory of the device memory
		vkFreeMemory(m_VkDevice, m_DeviceMemory, m_VkInstance.AllocationCallbaks);
		// Destroy buffer for the given device and instance
		vkDestroyBuffer(m_VkDevice, m_Buffer, m_VkInstance.AllocationCallbaks);
	}
	// If the staging buffer is not null handle
	if (m_StagingBuffer != VK_NULL_HANDLE)
	{
		// Free memory of the staging device memory
		vkFreeMemory(m_VkDevice, m_StagingDeviceMemory, m_VkInstance.AllocationCallbaks);
		// Destroy staging buffer for the given device and instance
		vkDestroyBuffer(m_VkDevice, m_StagingBuffer, m_VkInstance.AllocationCallbaks);
	}
}

void shade::VulkanVertexBuffer::SetData(std::uint32_t size, const void* data, std::uint32_t offset)
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
		memcpy(pData, data, (std::size_t)size);
		vkUnmapMemory(m_VkDevice, m_StagingDeviceMemory);

		// Create RenderCommandBuffer to include in buffer copying.
		
		VkBufferCopy copyRegion { .srcOffset = 0, .dstOffset = offset, .size = size };
		auto copyComamndBuffer = RenderCommandBuffer::Create(RenderCommandBuffer::Type::Primary, RenderCommandBuffer::Family::Transfer);

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
		void* pData; vkMapMemory(m_VkDevice, m_DeviceMemory, offset, size, 0, &pData);
		memcpy(pData, data, (std::size_t)size);
		vkUnmapMemory(m_VkDevice, m_DeviceMemory);
	}
}

void shade::VulkanVertexBuffer::Resize(std::uint32_t size)
{
	// If size is not null and the current size must be resized according to the resize threshold
	if (size && HasToBeResized(m_Size, size, m_ResizeThreshold))
	{
		// Invalidate the object taking into account the usage, new size and the resize threshold
		Invalidate(m_Usage, size, m_ResizeThreshold);
	}
}

const VkBuffer* shade::VulkanVertexBuffer::GetBuffer() const
{
	return &m_Buffer;
}

void shade::VulkanVertexBuffer::Bind(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex, std::uint32_t binding, std::uint32_t offset) const
{
	VkDeviceSize instanceOffset[1] = { offset };
	vkCmdBindVertexBuffers(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex), binding, 1, &m_Buffer, instanceOffset);
}

void shade::VulkanVertexBuffer::Invalidate(Usage usage, std::uint32_t size, std::size_t resizeThreshold)
{
	// Set buffer size, usage and resize threshold.
	m_Size = size;
	m_Usage = usage;
	m_ResizeThreshold = resizeThreshold;

	// If buffer exists, destroy it and its memory.  
	if (m_Buffer != VK_NULL_HANDLE)
	{
		vkFreeMemory(m_VkDevice, m_DeviceMemory, m_VkInstance.AllocationCallbaks);
		vkDestroyBuffer(m_VkDevice, m_Buffer, m_VkInstance.AllocationCallbaks);
		m_Buffer = VK_NULL_HANDLE;
		m_DeviceMemory = VK_NULL_HANDLE;
	}

	// Create a buffer for use in the device with properties to be used as a vertex buffer.
	VkBufferCreateInfo bufferCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = 0,
		.size = static_cast<std::uint64_t>(size),
		.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = VK_NULL_HANDLE
	};

	// Use CreateBuffer function to create a buffer for the device also with the memory allocation to it.
	auto buffer = CreateBuffer(m_VkDevice, m_VkInstance.AllocationCallbaks, &bufferCreateInfo,
		(m_Usage == Usage::GPU) ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	// Store the allocated buffer and device memory.
	m_Buffer = std::get<0>(buffer);
	m_DeviceMemory = std::get<1>(buffer);
}

std::tuple<VkBuffer, VkDeviceMemory> shade::VulkanVertexBuffer::CreateBuffer(VkDevice device, const VkAllocationCallbacks* pAllocator, const VkBufferCreateInfo* pCreateInfo, const VkMemoryPropertyFlags& properties)
{
	// creates a buffer
	VkBuffer buffer;
	VK_CHECK_RESULT(vkCreateBuffer(device, pCreateInfo, pAllocator, &buffer), "Failed to create buffer!");

	// sets a debug name for the newly created buffer
	VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "VertexBuffer", (std::uint32_t)buffer), device, VK_OBJECT_TYPE_BUFFER, buffer);

	// retrieves memory requirements for the buffer
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	// allocates memory for the buffer
	VkMemoryAllocateInfo memoryAllocateInfo
	{
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		VK_NULL_HANDLE,
		memRequirements.size,
		VKUtils::FindMemoryType(VulkanContext::GetPhysicalDevice()->GetDevice(), memRequirements.memoryTypeBits, properties)
	};
	VkDeviceMemory bufferMemory;
	VK_CHECK_RESULT(vkAllocateMemory(device, &memoryAllocateInfo, pAllocator, &bufferMemory), "Failed to allocate memory!");

	// sets a debug name for the buffer memory
	VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "VertexBuffer memory", (std::uint32_t)buffer), device, VK_OBJECT_TYPE_DEVICE_MEMORY, bufferMemory);

	// binds the buffer with the allocated memory
	vkBindBufferMemory(device, buffer, bufferMemory, 0);

	// returns the newly created buffer and its assigned memory
	return { buffer, bufferMemory };
}