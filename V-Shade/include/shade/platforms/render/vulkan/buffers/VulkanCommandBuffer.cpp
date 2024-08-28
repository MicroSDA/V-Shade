#include "shade_pch.h"
#include "VulkanCommandBuffer.h"
#include <shade/platforms/render/vulkan/VulkanContext.h>
#include <shade/core/application/Application.h>

shade::VulkanCommandBuffer::VulkanCommandBuffer(const Type& type, const Family& family, const std::uint32_t& count, const std::string& name) :
	m_VkDevice(VulkanContext::GetLogicalDevice()), m_VkPhysicalDevice(VulkanContext::GetPhysicalDevice()), m_VkInstance(VulkanContext::GetInstance())
{
	// Set the receiver-type, family and name variables
	m_Type = type; m_Family = family; m_Name = name;

	// Resize the command buffers using the count variable
	m_CommandBuffers.resize(count);
	
	//Loop through the Command Buffers, setting the queue family index based on the type of family and create command pool with the created info. Also, set debug object name.
	for (auto& commandBuffer : m_CommandBuffers)
	{
		VkCommandPoolCreateInfo commandPoolCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = VK_NULL_HANDLE,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
		};

		switch (family)
		{
		case Family::Present:
			commandPoolCreateInfo.queueFamilyIndex = static_cast<std::uint32_t>(m_VkPhysicalDevice->GetQueueFamilyIndices().Present);
			break;
		case Family::Graphic:
			commandPoolCreateInfo.queueFamilyIndex = static_cast<std::uint32_t>(m_VkPhysicalDevice->GetQueueFamilyIndices().Graphics);
			break;
		case Family::Transfer:
			commandPoolCreateInfo.queueFamilyIndex = static_cast<std::uint32_t>(m_VkPhysicalDevice->GetQueueFamilyIndices().Transfer);
			break;
		case Family::Compute:
			commandPoolCreateInfo.queueFamilyIndex = static_cast<std::uint32_t>(m_VkPhysicalDevice->GetQueueFamilyIndices().Compute);
			break;
		}

		VK_CHECK_RESULT(vkCreateCommandPool(m_VkDevice->GetDevice(), &commandPoolCreateInfo, m_VkInstance.AllocationCallbaks, &commandBuffer.Pool), "Failed to create command pool!");
		VKUtils::SetDebugObjectName(m_VkInstance.Instance, "Command pool", m_VkDevice->GetDevice(), VK_OBJECT_TYPE_COMMAND_POOL, commandBuffer.Pool);

		//Allocate command buffers using the created info and allocation count variable. Also, set debug object name.
		VkCommandBufferAllocateInfo commandBufferAllocateInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = VK_NULL_HANDLE,
			.commandPool = commandBuffer.Pool,
			.level = (type == Type::Primary) ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
			.commandBufferCount = 1
		};

		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_VkDevice->GetDevice(), &commandBufferAllocateInfo, &commandBuffer.Buffer), "Failed to allocate command buffers!");
		VKUtils::SetDebugObjectName(m_VkInstance.Instance, m_Name, m_VkDevice->GetDevice(), VK_OBJECT_TYPE_COMMAND_BUFFER, commandBuffer.Buffer);
	}
	//Resize the wait fences using the count variable.
	m_WaitFences.resize(count);
	
	//Loop through the Wait Fences, using the VK_FENCE_CREATE_SIGNALED_BIT flag, create with created info variable and set the debug object name.
	for (auto& fence : m_WaitFences)
	{
		VkFenceCreateInfo fenceCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = VK_NULL_HANDLE,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};

		VK_CHECK_RESULT(vkCreateFence(m_VkDevice->GetDevice(), &fenceCreateInfo, m_VkInstance.AllocationCallbaks, &fence), "Failed to create fence!");
		VKUtils::SetDebugObjectName(m_VkInstance.Instance, "Fence ", m_VkDevice->GetDevice(), VK_OBJECT_TYPE_FENCE, fence);
	}
}

shade::VulkanCommandBuffer::~VulkanCommandBuffer()
{
	for (auto i = 0u; i < m_CommandBuffers.size(); i++)
	{
		vkWaitForFences(m_VkDevice->GetDevice(), 1, &m_WaitFences[i], VK_TRUE, UINT64_MAX);
		vkFreeCommandBuffers(m_VkDevice->GetDevice(), m_CommandBuffers[i].Pool, 1, &m_CommandBuffers[i].Buffer);
		vkResetCommandPool(m_VkDevice->GetDevice(), m_CommandBuffers[i].Pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
		vkDestroyCommandPool(m_VkDevice->GetDevice(), m_CommandBuffers[i].Pool, VulkanContext::GetInstance().AllocationCallbaks);
		vkDestroyFence(m_VkDevice->GetDevice(), m_WaitFences[i], m_VkInstance.AllocationCallbaks);
	}
}

void shade::VulkanCommandBuffer::Begin(std::uint32_t index)
{
	// Create  commandBufferBeginInfo
	VkCommandBufferBeginInfo commandBufferBeginInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = VK_NULL_HANDLE
	};

	// Reset the command pool associated with the buffer
	VK_CHECK_RESULT(vkResetCommandPool(m_VkDevice->GetDevice(), m_CommandBuffers[index].Pool, 0), "Failed to reset command pool!");
	// Begin recording command buffer
	VK_CHECK_RESULT(vkBeginCommandBuffer(m_CommandBuffers[index].Buffer, &commandBufferBeginInfo), "Failed to begin recording command buffer!");
}

void shade::VulkanCommandBuffer::End(std::uint32_t index)
{
	// End recording of a command buffer
	VK_CHECK_RESULT(vkEndCommandBuffer(m_CommandBuffers[index].Buffer), "Failed to end command buffer!");
}

void shade::VulkanCommandBuffer::Submit(std::uint32_t index, std::uint32_t timeout)
{
	// Define a VkSubmitInfo struct with the appropriate values set for a submit operation
	VkSubmitInfo submitInfo =
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, //Define the Type of Struct for VkSubmitInfo
		.pNext = VK_NULL_HANDLE, //This value is used with extensions in Vulkan. Here, the value will be NULL as no extension is being used
		.waitSemaphoreCount = 0, //Number of semaphores to Wait (set to 0)
		.pWaitSemaphores = VK_NULL_HANDLE, //Pointer to the semaphores to wait (set to NULL)
		.pWaitDstStageMask = VK_NULL_HANDLE, //Pointer to the stages in which Wait should occur (set to NULL)
		.commandBufferCount = 1, //Number of command buffers to be submitted (set to 1)
		.pCommandBuffers = &m_CommandBuffers[index].Buffer, //Pointer to the command buffer to be submitted
		.signalSemaphoreCount = 0, //Number of semaphores to be signaled after the execution of the submitted command buffer(s) (set to 0)
		.pSignalSemaphores = VK_NULL_HANDLE //Pointer to the semaphores to be signaled after the execution of the submitted command buffer(s) (set to NULL)
	};

	//Set the Queue to which the submit operation will be sent depending on queue family defined
	VkQueue queue = VK_NULL_HANDLE;
	switch (m_Family)
	{
	case Family::Present:
		queue = m_VkDevice->GetPresentQueue(); //Set the Present Queue as the Queue to which the submit operation will be sent
		break;
	case Family::Graphic:
		queue = m_VkDevice->GetGraphicsQueue(); //Set the Graphics Queue as the Queue to which the submit operation will be sent
		break;
	case Family::Transfer:
		queue = m_VkDevice->GetTransferQueue(); //Set the Transfer Queue as the Queue to which the submit operation will be sent
		break;
	case Family::Compute:
		queue = m_VkDevice->GetComputeQueue(); //Set the Compute Queue as the Queue to which the submit operation will be sent
	}

	// TIP: Blinkin screen probably here !
	// TIP: Family::Present or Family::Graphic is using directly by swap chain, and it's not covered by mutex!
	VK_CHECK_RESULT(vkResetFences(m_VkDevice->GetDevice(), 1, &m_WaitFences[index]), "Failed to reset fences!");
	// Reset a fence before submitting a command buffer which uses it
	m_sMutexs[m_Family].lock();
	//Submit a command buffer to a queue for execution
	VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, m_WaitFences[index]), "Failed to queue submit!");
	m_sMutexs[m_Family].unlock();
	//Wait for the fence to signal a completion state for a given timeout value
	VK_CHECK_RESULT(vkWaitForFences(m_VkDevice->GetDevice(), 1, &m_WaitFences[index], VK_TRUE, timeout), "Failed to wait fance !");
}

VkCommandBuffer shade::VulkanCommandBuffer::GetCommandBuffer(std::uint32_t index)
{
	return m_CommandBuffers[index].Buffer;
}

VkFence shade::VulkanCommandBuffer::GetFence(std::uint32_t index)
{
	return m_WaitFences[index];
}
