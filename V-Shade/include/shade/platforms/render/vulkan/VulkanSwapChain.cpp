#include "shade_pch.h"
#include "VulkanSwapChain.h"
#include <shade/core/render/Renderer.h>

void shade::VulkanSwapChain::Initialize()
{
	m_LogicalDevice = VulkanContext::GetDevice();
	//VulkanContext::GetInstance();
}

void shade::VulkanSwapChain::CreateFrame(std::uint32_t* width, std::uint32_t* height, std::uint32_t framesCount, bool vsync)
{
	m_VSync = vsync;
	m_FramesCount = framesCount;

	VkDevice device = m_LogicalDevice->GetLogicalDevice();
	VkPhysicalDevice physicalDevice = m_LogicalDevice->GetPhysicalDevice()->GetDevice();
	VkSwapchainKHR oldSwapchain = m_SwapChain;

	auto& instnance = VulkanContext::GetInstance();

	// Get physical device surface properties and formats
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &surfaceCapabilities), "Failed to get physical device surface capabilities!");
	// Get available present modes
	std::uint32_t presentModeCount = 0;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, NULL), "Failed to get physical device surface present modes!");
	if (!presentModeCount)
		SHADE_CORE_ERROR("Physical device surface present modes is:{}", presentModeCount);

	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, presentModes.data()), "Failed to get physical device surface present modes!");

	VkExtent2D swapchainExtent;
	// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
	if (surfaceCapabilities.currentExtent.width == (uint32_t)-1)
	{
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swapchainExtent.width = *width;
		swapchainExtent.height = *height;
	}
	else
	{
		// If the surface size is defined, the swap chain size must match
		swapchainExtent = surfaceCapabilities.currentExtent;
		*width = surfaceCapabilities.currentExtent.width;
		*height = surfaceCapabilities.currentExtent.height;
	}

	m_Width = *width;
	m_Height = *height;

	// Select a present mode for the swapchain

	// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
	// This mode waits for the vertical blank ("v-sync")
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	// If v-sync is not requested, try to find a mailbox mode
	// It's the lowest latency non-tearing present mode available
	if (!vsync)
	{
		for (size_t i = 0; i < presentModeCount; i++)
		{
			if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}
			if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
			{
				swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}
	}

	// Determine the number of images
	std::uint32_t desiredNumberOfSwapchainImages = surfaceCapabilities.minImageCount + 1;
	if ((surfaceCapabilities.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfaceCapabilities.maxImageCount))
	{
		desiredNumberOfSwapchainImages = surfaceCapabilities.maxImageCount;
	}

	// Find the transformation of the surface
	VkSurfaceTransformFlagsKHR preTransform;
	if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		// We prefer a non-rotated transform
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = surfaceCapabilities.currentTransform;
	}

	// Find a supported composite alpha format (not all devices support alpha opaque)
	VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	// Simply select the first composite alpha format available
	std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags{
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};
	for (auto& compositeAlphaFlag : compositeAlphaFlags) {
		if (surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlag) {
			compositeAlpha = compositeAlphaFlag;
			break;
		};
	}
	// Create swapchain.
	VkSwapchainCreateInfoKHR swapchainCreateInfoKHR
	{
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, // sType
		VK_NULL_HANDLE, // pNex
		0, // flags
		m_Surface, // surface
		desiredNumberOfSwapchainImages, // minImageCount
		m_ColorFormat, // imageFormat
		m_ColorSpace, // imageColorSpace
		swapchainExtent, // imageExtent
		1, // imageArrayLayers
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, // imageUsage
		VK_SHARING_MODE_EXCLUSIVE, // imageSharingMode
		0, // queueFamilyIndexCount
		VK_NULL_HANDLE, // pQueueFamilyIndices
		static_cast<VkSurfaceTransformFlagBitsKHR>(preTransform), // preTransform
		compositeAlpha, // compositeAlpha
		swapchainPresentMode, // presentMode
		// Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
		VK_TRUE, // clipped
		oldSwapchain, // oldSwapchain
	};
	// Enable transfer source on swap chain images if supported
	if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
		swapchainCreateInfoKHR.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	// Enable transfer destination on swap chain images if supported
	if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
		swapchainCreateInfoKHR.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	VK_CHECK_RESULT(vkCreateSwapchainKHR(device, &swapchainCreateInfoKHR, instnance.AllocationCallbaks, &m_SwapChain), "Failed to create swapchain!");

	if (oldSwapchain)
		vkDestroySwapchainKHR(device, oldSwapchain, instnance.AllocationCallbaks);

	//for (auto& image : m_Images)
	//	vkDestroyImageView(device, image.View, instnance.AllocationCallbaks);
	//m_Images.clear();

	VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device, m_SwapChain, &m_ImageCount, VK_NULL_HANDLE), "Failed to get swap chain images khr!");
	if (m_ImageCount < m_FramesCount)
		SHADE_CORE_ERROR("Swap chain support only '{0}' frames in flight!", m_ImageCount)

		// Get the swap chain images
		std::vector<VkImage> swapchainImages, swapchainDepthImages;
	swapchainImages.resize(m_ImageCount); swapchainDepthImages.resize(m_ImageCount);

	VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device, m_SwapChain, &m_ImageCount, swapchainImages.data()), "Failed to get swap chain images khr!");

	//////////////
	m_FrameBuffers.clear();

	for (std::uint32_t i = 0; i < m_ImageCount; i++)
	{
		std::vector<SharedPointer<render::Image2D>> images;

		render::Image::Specification colorAttachmentSpecification;
		FrameBuffer::Specification frameBufferSpecification;

		colorAttachmentSpecification.Width = m_Width;
		colorAttachmentSpecification.Height = m_Height;
		colorAttachmentSpecification.Layers = 1;
		colorAttachmentSpecification.MipLevels = 1;
		colorAttachmentSpecification.Usage = render::Image::Usage::Attachment;

		// Color attachment
		colorAttachmentSpecification.Format = VKUtils::FromVulkanImageFormat(m_ColorFormat);
		images.emplace_back(render::Image2D::Create(colorAttachmentSpecification, reinterpret_cast<const void*>(swapchainImages[i])));
		frameBufferSpecification.Attachments.TextureAttachments.emplace_back(colorAttachmentSpecification.Format);

		// Depth stencil attachment
		colorAttachmentSpecification.Format = VKUtils::FromVulkanImageFormat(VulkanContext::GetPhysicalDevice()->GetDepthForamt());
		images.emplace_back(render::Image2D::Create(colorAttachmentSpecification));
		frameBufferSpecification.Attachments.TextureAttachments.emplace_back(colorAttachmentSpecification.Format);

		frameBufferSpecification.Width = m_Width;
		frameBufferSpecification.Height = m_Height;
		frameBufferSpecification.ClearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.f);

		m_FrameBuffers.emplace_back(FrameBuffer::Create(frameBufferSpecification, images));
	}

	// TIP: Trying to dont recriate command buffer if it was already exist bacause it will be unrecordered right after recriation
	if (!m_CommandBuffer)
		m_CommandBuffer = RenderCommandBuffer::Create(RenderCommandBuffer::Type::Primary, RenderCommandBuffer::Family::Graphic, m_ImageCount, "Swapchain command buffer.");
	if (!m_LayoutTransitionCommandBuffer)
		m_LayoutTransitionCommandBuffer = RenderCommandBuffer::Create(RenderCommandBuffer::Type::Primary, RenderCommandBuffer::Family::Graphic, m_ImageCount, "Layout transition swapchain command buffer.");

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Synchronization Objects
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (!m_Semaphores.RenderComplete || !m_Semaphores.PresentComplete)
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo
		{
			VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, // sType
			VK_NULL_HANDLE, // pNext
			0 // flags
		};

		VK_CHECK_RESULT(vkCreateSemaphore(m_LogicalDevice->GetLogicalDevice(), &semaphoreCreateInfo, instnance.AllocationCallbaks, &m_Semaphores.RenderComplete), "Failed to crate semaphore!");
		VKUtils::SetDebugObjectName(instnance.Instance, "Swapchain Semaphore Render complete", device, VK_OBJECT_TYPE_SEMAPHORE, m_Semaphores.RenderComplete);
		VK_CHECK_RESULT(vkCreateSemaphore(m_LogicalDevice->GetLogicalDevice(), &semaphoreCreateInfo, instnance.AllocationCallbaks, &m_Semaphores.PresentComplete), "Failed to crate semaphore!");
		VKUtils::SetDebugObjectName(instnance.Instance, "Swapchain Semaphore Present complete", device, VK_OBJECT_TYPE_SEMAPHORE, m_Semaphores.PresentComplete);
	}
}

void shade::VulkanSwapChain::OnResize(std::uint32_t width, std::uint32_t height)
{
	auto device = m_LogicalDevice->GetLogicalDevice();
	vkDeviceWaitIdle(device);
	CreateFrame(&width, &height, m_FramesCount, m_VSync);
	vkDeviceWaitIdle(device);
}

void shade::VulkanSwapChain::BeginFrame()
{
	// Resource release queue
	// TODO: UNCOMMENT !!!!!
	//auto& queue = Renderer::GetRenderResourceReleaseQueue(m_CurrentBufferIndex);

	//queue.Execute();
	m_CurrentImageIndex = AcquireNextImage();
	Renderer::BeginFrame(m_CurrentFrameIndex);
}

void shade::VulkanSwapChain::EndFrame()
{
	Renderer::EndFrame(m_CurrentFrameIndex);
}

void shade::VulkanSwapChain::Present(std::uint32_t timeout)
{
	auto& image = m_FrameBuffers[m_CurrentImageIndex]->As<VulkanFrameBuffer>().GetTextureAttachment(0)->GetImage()->As<VulkanImage2D>();
	image.LayoutTransition(m_LayoutTransitionCommandBuffer,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_TRANSFER_READ_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1, m_CurrentImageIndex);


	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkCommandBuffer commandBuffer = m_CommandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(m_CurrentFrameIndex);
	VkFence fence = m_CommandBuffer->As<VulkanCommandBuffer>().GetFence(m_CurrentFrameIndex);

	VkSubmitInfo submitInfo
	{
		VK_STRUCTURE_TYPE_SUBMIT_INFO, // sType
		VK_NULL_HANDLE, // pNext
		1,// waitSemaphoreCount
		&m_Semaphores.PresentComplete, // pWaitSemaphores
		&waitStageMask, // pWaitDstStageMask
		1, // commandBufferCount 
		&commandBuffer, // pCommandBuffers
		1, // signalSemaphoreCount
		&m_Semaphores.RenderComplete // pSignalSemaphores
	};

	VK_CHECK_RESULT(vkResetFences(m_LogicalDevice->GetLogicalDevice(), 1, &fence), "Failed to reset fences!");
	VK_CHECK_RESULT(vkQueueSubmit(m_LogicalDevice->m_GraphicsQueue, 1, &submitInfo, fence), "Present failed to submit!");

	// Present the current buffer to the swap chain
	// Pass the semaphore signaled by the command buffer submission from the submit info as the wait semaphore for swap chain presentation
	// This ensures that the image is not presented to the windowing system until all commands have been submitted
	VkResult result;

	VkPresentInfoKHR presentInfoKHR
	{
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, // sType
		VK_NULL_HANDLE, // pNext
		1, // waitSemaphoreCount
		&m_Semaphores.RenderComplete, // pWaitSemaphores
		1, // swapchainCount
		&m_SwapChain, // pSwapchains
		&m_CurrentImageIndex, // pImageIndices
		&result // pResults
	};

	vkQueuePresentKHR(m_LogicalDevice->m_PresentQueue, &presentInfoKHR);

	if (result != VK_SUCCESS)
	{
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
			OnResize(m_Width, m_Height);
		else
			VK_CHECK_RESULT(result, "Failed to present!");
	}
	{
		//const auto& config = Renderer::GetConfig();
		m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_FramesCount;
		VkResult result = vkWaitForFences(m_LogicalDevice->GetLogicalDevice(), 1, &fence, VK_TRUE, static_cast<std::uint64_t>(timeout));

		if (result != VK_SUCCESS)
			SHADE_CORE_WARNING("Failed to wait fance !");
	}
}

std::uint32_t shade::VulkanSwapChain::AcquireNextImage()
{
	uint32_t imageIndex;
	VK_CHECK_RESULT(vkAcquireNextImageKHR(m_LogicalDevice->GetLogicalDevice(), m_SwapChain, UINT64_MAX, m_Semaphores.PresentComplete, (VkFence)nullptr, &imageIndex), "Failed to acquire next image khr!");
	return imageIndex;
}

VkCommandBuffer shade::VulkanSwapChain::GetVulkanCommandBuffer()
{
	return m_CommandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(m_CurrentFrameIndex);
}

void shade::VulkanSwapChain::Destroy()
{
	auto device = m_LogicalDevice->GetLogicalDevice();
	vkDeviceWaitIdle(device);

	if (m_SwapChain)
		vkDestroySwapchainKHR(device, m_SwapChain, nullptr);

	m_FrameBuffers.clear();

	if (m_Semaphores.RenderComplete)
		vkDestroySemaphore(device, m_Semaphores.RenderComplete, nullptr);

	if (m_Semaphores.PresentComplete)
		vkDestroySemaphore(device, m_Semaphores.PresentComplete, nullptr);

	if (m_Surface)
		vkDestroySurfaceKHR(VulkanContext::GetInstance().Instance, m_Surface, nullptr);

	/*m_CommandBuffer.Reset();
	m_LayoutTransitionCommandBuffer.Reset();*/

	vkDeviceWaitIdle(device);

}

const std::uint32_t& shade::VulkanSwapChain::GetCurrentBufferIndex() const
{
	return m_CurrentFrameIndex;
}

shade::SharedPointer<shade::FrameBuffer>& shade::VulkanSwapChain::GetFrameBuffer()
{
	return m_FrameBuffers[m_CurrentImageIndex];
}

VkCommandBuffer shade::VulkanSwapChain::GetDrawCommandBuffer()
{
	return m_CommandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(m_CurrentImageIndex);
}

const std::uint32_t& shade::VulkanSwapChain::GetWidth() const
{
	return m_Width;
}

const std::uint32_t& shade::VulkanSwapChain::GetHeight() const
{
	return m_Height;
}

void shade::VulkanSwapChain::CreateSurface(GLFWwindow* window)
{
	VkPhysicalDevice physicalDevice = m_LogicalDevice->GetPhysicalDevice()->GetDevice();
	// Create window surface.
	VK_CHECK_RESULT(glfwCreateWindowSurface(VulkanContext::GetInstance().Instance, window, VulkanContext::GetInstance().AllocationCallbaks, &m_Surface), "Failed to create window surface!");

	// Get available queue family properties
	std::uint32_t queueCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, NULL);
	if (!queueCount)
		SHADE_CORE_ERROR("Queue family count :{}!", queueCount);

	std::vector<VkQueueFamilyProperties> queueProperties(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueProperties.data());

	// Find a queue with present support
	std::vector<VkBool32> supportsPresent(queueCount);
	for (uint32_t i = 0; i < queueCount; i++)
		VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_Surface, &supportsPresent[i]), "Failed to get physical device surface support!");

	// Search for a graphics and a present queue in the array of queue
	// families, try to find one that supports both

	std::uint32_t graphicsQueueNodeIndex = UINT32_MAX;
	std::uint32_t presentQueueNodeIndex = UINT32_MAX;

	for (std::uint32_t i = 0; i < queueCount; i++)
	{
		if ((queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			if (graphicsQueueNodeIndex == UINT32_MAX)
			{
				graphicsQueueNodeIndex = i;
			}

			if (supportsPresent[i] == VK_TRUE)
			{
				graphicsQueueNodeIndex = i;
				presentQueueNodeIndex = i;
				break;
			}
		}
	}
	if (presentQueueNodeIndex == UINT32_MAX)
	{
		// If there's no queue that supports both present and graphics
		// try to find a separate present queue
		for (uint32_t i = 0; i < queueCount; ++i)
		{
			if (supportsPresent[i] == VK_TRUE)
			{
				presentQueueNodeIndex = i;
				break;
			}
		}
	}

	if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX)
		SHADE_CORE_ERROR("Graphics queue node index or present queue node index were not been found!");

	m_LogicalDevice->m_PhysicalDevice->m_QueueFamilyIndices.Present = presentQueueNodeIndex;
	vkGetDeviceQueue(m_LogicalDevice->GetLogicalDevice(), presentQueueNodeIndex, 0, &m_LogicalDevice->m_PresentQueue);
	m_QueueNodeIndex = graphicsQueueNodeIndex;

	GetImageFormatAndColorSpace();
}

void shade::VulkanSwapChain::GetImageFormatAndColorSpace()
{
	VkPhysicalDevice physicalDevice = m_LogicalDevice->GetPhysicalDevice()->GetDevice();

	// Get list of supported surface formats
	std::uint32_t formatCount;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, NULL), "Failed to get physical device surface formats khr!");
	assert(formatCount > 0);

	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, surfaceFormats.data()), "Failed to get physical device surface formats khr!");

	// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
	// there is no preferered format, so we assume VK_FORMAT_B8G8R8A8_UNORM
	if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
	{
		m_ColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
		m_ColorSpace = surfaceFormats[0].colorSpace;
	}
	else
	{
		// iterate over the list of available surface format and
		// check for the presence of VK_FORMAT_B8G8R8A8_UNORM
		bool found_B8G8R8A8_UNORM = false;
		for (auto&& surfaceFormat : surfaceFormats)
		{
			if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
			{
				m_ColorFormat = surfaceFormat.format;
				m_ColorSpace = surfaceFormat.colorSpace;
				found_B8G8R8A8_UNORM = true;
				break;
			}
		}

		// in case VK_FORMAT_B8G8R8A8_UNORM is not available
		// select the first available color format
		if (!found_B8G8R8A8_UNORM)
		{
			m_ColorFormat = surfaceFormats[0].format;
			m_ColorSpace = surfaceFormats[0].colorSpace;
		}
	}
}