#include "shade_pch.h"
#include "VulkanSwapChain.h"
#include <shade/core/render/Renderer.h>

void shade::VulkanSwapChain::Initialize()
{
	m_LogicalDevice = VulkanContext::GetLogicalDevice();
}

void shade::VulkanSwapChain::CreateFrame(std::uint32_t* width, std::uint32_t* height, std::uint32_t framesCount, bool vsync)
{
	//------------------------------------------------------------------------
	// Initialization and Configuration
	//------------------------------------------------------------------------

	// Set the VSync and FramesCount settings
	m_VSync = vsync, m_FramesCount = framesCount;
	
	//------------------------------------------------------------------------
	// Vulkan Instance and Device Setup
	//------------------------------------------------------------------------

	// Initialize Vulkan instance and retrieve logical and physical devices
	VulkanContext::VulkanInstance&  instance		= VulkanContext::GetInstance();
	VkDevice						logicalDevice	= m_LogicalDevice->GetDevice();
	VkPhysicalDevice				physicalDevice	= m_LogicalDevice->GetPhysicalDevice()->GetDevice();
	VkSwapchainKHR					oldSwapchain	= m_SwapChain;

	//------------------------------------------------------------------------
	// Surface Capabilities and Present Modes
	//------------------------------------------------------------------------

	// Get physical device surface properties and formats
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &surfaceCapabilities), "Failed to get physical device surface capabilities!");

	// Check if the surface size is zero, indicating that the window might be minimized
	if (!surfaceCapabilities.currentExtent.width && !surfaceCapabilities.currentExtent.height)
	{
		// Log a warning and skip the swapchain creation if the window is minimized
		SHADE_CORE_WARNING("Skipping VulkanSwapChain::CreateFrame, VkSurfaceCapabilitiesKHR::curentExtent :{0}, {0}", surfaceCapabilities.currentExtent.width, surfaceCapabilities.currentExtent.height);
		return;
	}

	// Get available present modes for the physical device
	std::uint32_t presentModeCount = 0;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, NULL), "Failed to get physical device surface present modes!");
	if (!presentModeCount) SHADE_CORE_ERROR("Physical device surface present modes is: {}", presentModeCount);

	// Retrieve the available present modes
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, presentModes.data()), "Failed to get physical device surface present modes!");

	//------------------------------------------------------------------------
	// Swapchain Extent Configuration
	//------------------------------------------------------------------------

	// Determine the swapchain extent (resolution)

	// If the surface size is undefined, set the swapchain extent to the requested image size
	if (surfaceCapabilities.currentExtent.width == (uint32_t)-1)
	{
		surfaceCapabilities.currentExtent.width  = *width;
		surfaceCapabilities.currentExtent.height = *height;
	}
	else
	{
		// Otherwise, use the surface's current extent and update the width and height
		*width = surfaceCapabilities.currentExtent.width;
		*height = surfaceCapabilities.currentExtent.height;
	}

	// Store the swapchain dimensions
	m_Width = *width, m_Height = *height;
	
	//------------------------------------------------------------------------
	// Present Mode Selection
	//------------------------------------------------------------------------

	// Select a present mode for the swapchain
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR; // Default mode with v-sync

	// If v-sync is not requested, look for a low-latency present mode
	if (!vsync)
	{
		for (size_t i = 0; i < presentModeCount; i++)
		{
			// Prefer MAILBOX mode for its low latency
			if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}
			// Fall back to IMMEDIATE mode if MAILBOX is not available
			if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
			{
				swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}
	}

	//------------------------------------------------------------------------
	// Swapchain Image Configuration
	//------------------------------------------------------------------------

	// Determine the number of swapchain images
	std::uint32_t desiredNumberOfSwapchainImages = surfaceCapabilities.minImageCount + 1;
	if ((surfaceCapabilities.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfaceCapabilities.maxImageCount))
	{
		// Ensure the number of images doesn't exceed the maximum supported
		desiredNumberOfSwapchainImages = surfaceCapabilities.maxImageCount;
	}

	// Select the surface transformation, preferring no rotation if supported
	VkSurfaceTransformFlagsKHR preTransform;
	if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		// Fall back to the current transform if identity is not supported
		preTransform = surfaceCapabilities.currentTransform;
	}

	//------------------------------------------------------------------------
	// Composite Alpha Selection
	//------------------------------------------------------------------------

	// Choose a supported composite alpha format
	VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Default to opaque
	std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags{
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};

	// Select the first supported composite alpha format
	for (auto& compositeAlphaFlag : compositeAlphaFlags) {
		if (surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlag) {
			compositeAlpha = compositeAlphaFlag;
			break;
		}
	}

	//------------------------------------------------------------------------
	// Swapchain Creation
	//------------------------------------------------------------------------

	// Create the swapchain with the selected settings
	VkSwapchainCreateInfoKHR swapchainCreateInfoKHR
	{
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, // sType
		VK_NULL_HANDLE, // pNext
		0, // flags
		m_Surface, // surface
		desiredNumberOfSwapchainImages, // minImageCount
		m_ColorFormat, // imageFormat
		m_ColorSpace, // imageColorSpace
		surfaceCapabilities.currentExtent, // imageExtent
		1, // imageArrayLayers
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, // imageUsage
		VK_SHARING_MODE_EXCLUSIVE, // imageSharingMode
		0, // queueFamilyIndexCount
		VK_NULL_HANDLE, // pQueueFamilyIndices
		static_cast<VkSurfaceTransformFlagBitsKHR>(preTransform), // preTransform
		compositeAlpha, // compositeAlpha
		swapchainPresentMode, // presentMode
		VK_TRUE, // clipped, allows the implementation to discard rendering outside of the surface area
		oldSwapchain, // oldSwapchain
	};

	// Enable additional image usage flags if supported
	if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
		swapchainCreateInfoKHR.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}
	if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
		swapchainCreateInfoKHR.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	// Create the swapchain
	VK_CHECK_RESULT(vkCreateSwapchainKHR(logicalDevice, &swapchainCreateInfoKHR, instance.AllocationCallbaks, &m_SwapChain), "Failed to create swapchain!");

	// Clean up the old swapchain if it exists
	if (oldSwapchain)
		vkDestroySwapchainKHR(logicalDevice, oldSwapchain, instance.AllocationCallbaks);

	//------------------------------------------------------------------------
	// Swapchain Image Retrieval
	//------------------------------------------------------------------------

	// Retrieve the number of swapchain images
	VK_CHECK_RESULT(vkGetSwapchainImagesKHR(logicalDevice, m_SwapChain, &m_ImageCount, VK_NULL_HANDLE), "Failed to get swap chain images khr!");
	if (m_ImageCount < m_FramesCount)
		SHADE_CORE_ERROR("Swap chain support only '{0}' frames in flight!", m_ImageCount)

		// Get the swapchain images
		std::vector<VkImage> swapchainImages, swapchainDepthImages;
	swapchainImages.resize(m_ImageCount);
	swapchainDepthImages.resize(m_ImageCount);

	VK_CHECK_RESULT(vkGetSwapchainImagesKHR(logicalDevice, m_SwapChain, &m_ImageCount, swapchainImages.data()), "Failed to get swap chain images khr!");

	//------------------------------------------------------------------------
	// Framebuffer Setup
	//------------------------------------------------------------------------

	// Clear the current frame buffers
	m_FrameBuffers.clear();

	for (std::uint32_t i = 0; i < m_ImageCount; i++)
	{
		// Prepare attachments and frame buffer specifications
		std::vector<SharedPointer<render::Image2D>> images;

		render::Image::Specification colorAttachmentSpecification;
		FrameBuffer::Specification frameBufferSpecification;

		// Set color attachment specifications
		colorAttachmentSpecification.Width = m_Width;
		colorAttachmentSpecification.Height = m_Height;
		colorAttachmentSpecification.Layers = 1;
		colorAttachmentSpecification.MipLevels = 1;
		colorAttachmentSpecification.Usage = render::Image::Usage::Attachment;
		colorAttachmentSpecification.Format = VKUtils::FromVulkanImageFormat(m_ColorFormat);
		images.emplace_back(render::Image2D::Create(colorAttachmentSpecification, reinterpret_cast<const void*>(swapchainImages[i])));
		frameBufferSpecification.Attachments.TextureAttachments.emplace_back(colorAttachmentSpecification.Format);

		// Set depth-stencil attachment specifications
		colorAttachmentSpecification.Format = VKUtils::FromVulkanImageFormat(m_LogicalDevice->GetPhysicalDevice()->GetDepthForamt());
		images.emplace_back(render::Image2D::Create(colorAttachmentSpecification));
		frameBufferSpecification.Attachments.TextureAttachments.emplace_back(colorAttachmentSpecification.Format);

		// Set frame buffer specifications
		frameBufferSpecification.Width = m_Width;
		frameBufferSpecification.Height = m_Height;
		frameBufferSpecification.ClearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.f);

		// Create and store the frame buffer
		m_FrameBuffers.emplace_back(FrameBuffer::Create(frameBufferSpecification, images));
	}

	//------------------------------------------------------------------------
	// Command Buffer Creation
	//------------------------------------------------------------------------

	// Ensure command buffers are created only once to avoid unnecessary reallocation
	if (!m_CommandBuffer)
		m_CommandBuffer = RenderCommandBuffer::Create(RenderCommandBuffer::Type::Primary, RenderCommandBuffer::Family::Graphic, m_ImageCount, "Swapchain command buffer.");

	if (!m_LayoutTransitionCommandBuffer)
		m_LayoutTransitionCommandBuffer = RenderCommandBuffer::Create(RenderCommandBuffer::Type::Primary, RenderCommandBuffer::Family::Graphic, m_ImageCount, "Layout transition swapchain command buffer.");

	//------------------------------------------------------------------------
	// Synchronization Objects Creation
	//------------------------------------------------------------------------

	// Create synchronization semaphores if they do not already exist
	if (!m_Semaphores.RenderComplete || !m_Semaphores.PresentComplete)
	{
		// Create semaphores for rendering and presentation synchronization
		VkSemaphoreCreateInfo semaphoreCreateInfo
		{
			VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, // sType
			VK_NULL_HANDLE, // pNext
			0 // flags
		};

		// Create RenderComplete semaphore
		VK_CHECK_RESULT(vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, instance.AllocationCallbaks, &m_Semaphores.RenderComplete), "Failed to create semaphore!");
		VKUtils::SetDebugObjectName(instance.Instance, "Swapchain Semaphore Render complete", logicalDevice, VK_OBJECT_TYPE_SEMAPHORE, m_Semaphores.RenderComplete);

		// Create PresentComplete semaphore
		VK_CHECK_RESULT(vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, instance.AllocationCallbaks, &m_Semaphores.PresentComplete), "Failed to create semaphore!");
		VKUtils::SetDebugObjectName(instance.Instance, "Swapchain Semaphore Present complete", logicalDevice, VK_OBJECT_TYPE_SEMAPHORE, m_Semaphores.PresentComplete);
	}
}

void shade::VulkanSwapChain::OnResize(std::uint32_t width, std::uint32_t height)
{
	//vkDeviceWaitIdle(m_LogicalDevice->GetDevice());

	// Create new swap chain frame depends on new frame size 
	CreateFrame(&width, &height, m_FramesCount, m_VSync);

	//vkDeviceWaitIdle(m_LogicalDevice->GetDevice());
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
	//------------------------------------------------------------------------
	// Image Layout Transition
	//------------------------------------------------------------------------

	// Retrieve the Vulkan logical device to be used for issuing commands
	VkDevice logicalDevice = m_LogicalDevice->GetDevice();

	// Get the image from the current framebuffer's texture attachment
	auto& image = m_FrameBuffers[m_CurrentImageIndex]->As<VulkanFrameBuffer>().GetTextureAttachment(0)->GetImage()->As<VulkanImage2D>();

	// Perform a layout transition on the image to ensure it's in the correct layout before presenting
	image.LayoutTransition(
		m_LayoutTransitionCommandBuffer,		// Command buffer used for the layout transition
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,		// New image layout for presentation
		VK_ACCESS_SHADER_READ_BIT,				// Source access mask (previous access)
		VK_ACCESS_TRANSFER_READ_BIT,			// Destination access mask (upcoming access)
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,	// Source pipeline stage (fragment shader)
		VK_PIPELINE_STAGE_TRANSFER_BIT,			// Destination pipeline stage (transfer operation)
		VK_IMAGE_ASPECT_COLOR_BIT,				// Aspect mask (color aspect)
		0, 1, 0, 1,								// Mip level and layer ranges
		m_CurrentImageIndex						// Index of the current image
	);

	//------------------------------------------------------------------------
	// Command Buffer Submission
	//------------------------------------------------------------------------

	// Define the pipeline stage where the image layout transition waits (color attachment output)
	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	// Retrieve the primary command buffer for the current frame
	VkCommandBuffer commandBuffer = m_CommandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(m_CurrentFrameIndex);

	// Retrieve the fence associated with the current frame to synchronize GPU execution
	VkFence fence = m_CommandBuffer->As<VulkanCommandBuffer>().GetFence(m_CurrentFrameIndex);

	// Fill the structure with information about the command buffer submission to the graphics queue
	VkSubmitInfo submitInfo{
		VK_STRUCTURE_TYPE_SUBMIT_INFO,      // Structure type
		VK_NULL_HANDLE,                     // Next pointer (no extensions)
		1,                                  // Number of semaphores to wait on before executing the command buffer
		&m_Semaphores.PresentComplete,      // Semaphores to wait on before executing the command buffer
		&waitStageMask,                     // Pipeline stage at which the waiting occurs
		1,                                  // Number of command buffers to submit
		&commandBuffer,                     // Pointer to the command buffer to submit
		1,                                  // Number of semaphores to signal once command buffer execution finishes
		&m_Semaphores.RenderComplete        // Semaphores to signal once the command buffer execution finishes
	};

	// Reset the fence to prepare it for the next GPU synchronization
	VK_CHECK_RESULT(vkResetFences(logicalDevice, 1, &fence), "Failed to reset fences!");

	// Submit the command buffer to the graphics queue for execution
	VK_CHECK_RESULT(vkQueueSubmit(m_LogicalDevice->m_GraphicsQueue, 1, &submitInfo, fence), "Present failed to submit!");

	//------------------------------------------------------------------------
	// Presenting the Image to the Swapchain
	//------------------------------------------------------------------------

	// Prepare the present info structure to present the image to the swap chain
	VkResult result; VkPresentInfoKHR presentInfoKHR
	{
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, // Structure type
		VK_NULL_HANDLE,                     // Next pointer (no extensions)
		1,                                  // Number of semaphores to wait on before presenting the image
		&m_Semaphores.RenderComplete,       // Semaphore to wait on before presenting the image
		1,                                  // Number of swapchains to present to
		&m_SwapChain,                       // Pointer to the swapchain
		&m_CurrentImageIndex,               // Pointer to the index of the image to present
		&result                             // Pointer to store the result of the presentation
	};

	// Present the image to the swapchain
	vkQueuePresentKHR(m_LogicalDevice->m_PresentQueue, &presentInfoKHR);

	// Handle potential presentation errors
	if (result != VK_SUCCESS)
	{
		// If the swapchain is out of date or suboptimal, trigger a resize
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
			OnResize(m_Width, m_Height);
		else
			VK_CHECK_RESULT(result, "Failed to present!");
	}

	//------------------------------------------------------------------------
	// Frame Synchronization and Index Management
	//------------------------------------------------------------------------

	// Increment the current frame index, wrapping around if necessary
	m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_FramesCount;

	// Wait for the fence to ensure the GPU has finished processing the current frame
	result = vkWaitForFences(logicalDevice, 1, &fence, VK_TRUE, static_cast<std::uint64_t>(timeout));

	// Log a warning if waiting for the fence fails
	if (result != VK_SUCCESS) SHADE_CORE_WARNING("Failed to wait for fence!");
}

std::uint32_t shade::VulkanSwapChain::AcquireNextImage(std::uint64_t timeout)
{
	// Initialize a static variable to keep track of the current image index in the swapchain
	static uint32_t imageIndex = 0;

	//------------------------------------------------------------------------
	// Acquiring the Next Swapchain Image
	//------------------------------------------------------------------------

	// Check if the swapchain is valid before attempting to acquire the next image
	if (m_SwapChain)
	{
		// Acquire the next image from the swapchain
		// This function returns the index of the next available image in the swapchain
		// The semaphore 'PresentComplete' is signaled when the image is ready to be used
		VK_CHECK_RESULT(vkAcquireNextImageKHR(
			m_LogicalDevice->GetDevice(),  // Logical device that owns the swapchain
			m_SwapChain,                   // Handle to the swapchain
			timeout,					   // Timeout in nanoseconds (UINT64_MAX disables timeout)
			m_Semaphores.PresentComplete,  // Semaphore to signal when the image is ready
			(VkFence)nullptr,              // Optional fence (not used here)
			&imageIndex                    // Pointer to the index of the acquired image
		), "Failed to acquire next image khr!");
	}

	//------------------------------------------------------------------------
	// Returning the Acquired Image Index
	//------------------------------------------------------------------------

	// Return the index of the image that was acquired from the swapchain
	return imageIndex;
}

void shade::VulkanSwapChain::Destroy()
{
	//------------------------------------------------------------------------
	// Device Synchronization Before Destruction
	//------------------------------------------------------------------------

	VkDevice device = m_LogicalDevice->GetDevice();

	// Wait until the device has finished all queued operations before proceeding with cleanup
	vkDeviceWaitIdle(device);

	//------------------------------------------------------------------------
	// Swapchain Destruction
	//------------------------------------------------------------------------

	if (m_SwapChain) vkDestroySwapchainKHR(device, m_SwapChain, VulkanContext::GetInstance().AllocationCallbaks);
	
	//------------------------------------------------------------------------
	// Framebuffer Cleanup
	//------------------------------------------------------------------------

	m_FrameBuffers.clear();

	//------------------------------------------------------------------------
	// Semaphore Destruction
	//------------------------------------------------------------------------

	if (m_Semaphores.RenderComplete) vkDestroySemaphore(device, m_Semaphores.RenderComplete, VulkanContext::GetInstance().AllocationCallbaks);
	if (m_Semaphores.PresentComplete) vkDestroySemaphore(device, m_Semaphores.PresentComplete, VulkanContext::GetInstance().AllocationCallbaks);
	
	//------------------------------------------------------------------------
	// Surface Destruction
	//------------------------------------------------------------------------

	if (m_Surface) vkDestroySurfaceKHR(VulkanContext::GetInstance().Instance, m_Surface, VulkanContext::GetInstance().AllocationCallbaks);
		
	//------------------------------------------------------------------------
	// Command Buffer Reset (Commented Out)
	//------------------------------------------------------------------------

	// These lines, if uncommented, would reset the command buffers, releasing their resources.
	// m_CommandBuffer.Reset();
	// m_LayoutTransitionCommandBuffer.Reset();

	//------------------------------------------------------------------------
	// Final Device Synchronization
	//------------------------------------------------------------------------

	vkDeviceWaitIdle(device);
}

void shade::VulkanSwapChain::CreateSurface(GLFWwindow* window)
{
	//------------------------------------------------------------------------
	// Retrieve Physical Device
	//------------------------------------------------------------------------

	VkPhysicalDevice physicalDevice = m_LogicalDevice->GetPhysicalDevice()->GetDevice();

	//------------------------------------------------------------------------
	// Create Window Surface
	//------------------------------------------------------------------------

	// Create a Vulkan surface for rendering to the specified window
	// Check if the surface creation is successful
	VK_CHECK_RESULT(
		glfwCreateWindowSurface(
			VulkanContext::GetInstance().Instance,				// Vulkan instance
			window,												// GLFW window
			VulkanContext::GetInstance().AllocationCallbaks,	// Allocation callbacks
			&m_Surface											// Output handle for the created surface
		), "Failed to create window surface!");

	//------------------------------------------------------------------------
	// Retrieve Queue Family Properties
	//------------------------------------------------------------------------

	// Retrieve the number of queue families available on the physical device
	std::uint32_t queueCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, NULL);

	// Error handling if no queue families are found
	if (!queueCount) SHADE_CORE_ERROR("Queue family count :{}!", queueCount);
	
	// Retrieve properties for each queue family
	std::vector<VkQueueFamilyProperties> queueProperties(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueProperties.data());

	//------------------------------------------------------------------------
	// Check Queue Support for Presenting
	//------------------------------------------------------------------------

	// Check which queue families support presentation to the surface
	std::vector<VkBool32> supportsPresent(queueCount);
	for (uint32_t i = 0; i < queueCount; i++)
	{
		VK_CHECK_RESULT(
			vkGetPhysicalDeviceSurfaceSupportKHR(
				physicalDevice,   // Physical device
				i,                // Queue family index
				m_Surface,        // Surface to check support for
				&supportsPresent[i] // Output support status
			), "Failed to get physical device surface support!");
	}

	//------------------------------------------------------------------------
	// Find Suitable Queue Families
	//------------------------------------------------------------------------

	// Find a queue family that supports both graphics and presentation
	std::uint32_t graphicsQueueNodeIndex	= UINT32_MAX;
	std::uint32_t presentQueueNodeIndex		= UINT32_MAX;

	for (std::uint32_t i = 0; i < queueCount; i++)
	{
		// Check if the queue supports graphics operations
		if ((queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			// If this queue also supports presentation, use it for both
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

	// If no single queue supports both, look for a separate presentation queue
	if (presentQueueNodeIndex == UINT32_MAX)
	{
		for (uint32_t i = 0; i < queueCount; ++i)
		{
			if (supportsPresent[i] == VK_TRUE)
			{
				presentQueueNodeIndex = i;
				break;
			}
		}
	}

	// Error handling if no suitable queues are found
	if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX)
		SHADE_CORE_ERROR("Graphics queue node index or present queue node index were not been found!");

	//------------------------------------------------------------------------
	// Retrieve Queue Handles
	//------------------------------------------------------------------------

	// Store the queue family index that supports presentation
	m_LogicalDevice->m_PhysicalDevice->m_QueueFamilyIndices.Present = presentQueueNodeIndex;

	// Retrieve the Vulkan queue for presentation
	vkGetDeviceQueue(m_LogicalDevice->GetDevice(), presentQueueNodeIndex, 0, &m_LogicalDevice->m_PresentQueue);

	// Store the index of the graphics queue
	m_QueueNodeIndex = graphicsQueueNodeIndex;

	//------------------------------------------------------------------------
	// Retrieve Image Format and Color Space
	//------------------------------------------------------------------------

	GetImageFormatAndColorSpace();
}

void shade::VulkanSwapChain::GetImageFormatAndColorSpace()
{
	//------------------------------------------------------------------------
	// Retrieve Physical Device
	//------------------------------------------------------------------------

	VkPhysicalDevice physicalDevice = m_LogicalDevice->GetPhysicalDevice()->GetDevice();

	//------------------------------------------------------------------------
	// Get Supported Surface Formats
	//------------------------------------------------------------------------

	std::uint32_t formatCount;

	// Query the number of supported surface formats
	VK_CHECK_RESULT(
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			physicalDevice,
			m_Surface,
			&formatCount,
			nullptr
		),
		"Failed to get physical device surface formats khr!"
	);
	assert(formatCount > 0);

	// Retrieve the list of supported surface formats
	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	VK_CHECK_RESULT(
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			physicalDevice,
			m_Surface,
			&formatCount,
			surfaceFormats.data()
		),
		"Failed to get physical device surface formats khr!"
	);

	//------------------------------------------------------------------------
	// Determine Preferred Surface Format
	//------------------------------------------------------------------------

	// If only one format is available and it is VK_FORMAT_UNDEFINED
	// assume VK_FORMAT_B8G8R8A8_UNORM as the preferred format
	if (formatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		m_ColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
		m_ColorSpace = surfaceFormats[0].colorSpace;
	}
	else
	{
		// Check if VK_FORMAT_B8G8R8A8_UNORM is available in the list
		bool B8G8R8A8_UNORM = false;
		for (const auto& surfaceFormat : surfaceFormats)
		{
			if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
			{
				m_ColorFormat = surfaceFormat.format;
				m_ColorSpace = surfaceFormat.colorSpace;
				B8G8R8A8_UNORM = true;
				break;
			}
		}
		// If VK_FORMAT_B8G8R8A8_UNORM is not available, use the first format in the list
		if (!B8G8R8A8_UNORM)
		{
			m_ColorFormat = surfaceFormats[0].format;
			m_ColorSpace = surfaceFormats[0].colorSpace;
		}
	}
}