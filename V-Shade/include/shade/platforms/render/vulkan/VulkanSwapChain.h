#pragma once
#include <shade/core/render/SwapChain.h>
#include <shade/platforms/render/vulkan/VKUtils.h>
#include <shade/platforms/render/vulkan/VulkanDevice.h>
#include <shade/platforms/render/vulkan/VulkanContext.h>
#include <shade/platforms/render/vulkan/buffers/VulkanCommandBuffer.h>
#include <shade/platforms/render/vulkan/buffers/VulkanFrameBuffer.h>

namespace shade
{
	// This class represents a Vulkan swap chain and provides methods for 
	// initializing the swap chain, creating frames, handling resizing, 
	// presenting frames, and destroying resources. It also provides access 
	// to the current buffer index, frame buffer, and Vulkan command buffers.
	class VulkanSwapChain : public SwapChain
	{
	public:

		// This struct contains Vulkan semaphore objects used for synchronization 
		// between the rendering and presentation stages. Semaphores are used to 
		// ensure that rendering commands are completed before presenting the 
		// rendered image to the swap chain.
		struct Semaphores
		{
			// Semaphore to signal that the presentation of an image is complete.
			// This semaphore is signaled by the presentation engine when the image 
			// has been fully presented and is ready to be used for rendering.
			VkSemaphore PresentComplete = NULL;
			// Semaphore to signal that rendering commands have been completed.
			// This semaphore is used to synchronize command buffer execution, 
			// ensuring that all rendering commands have finished before presenting 
			// the image.
			VkSemaphore RenderComplete = NULL;
		};
	public:
		VulkanSwapChain() = default;
		virtual ~VulkanSwapChain() = default;
		// @brief Initialize the Vulkan swap chain.
		virtual void Initialize() override;

		// @brief Create a frame for rendering.
		// @param width: Pointer to the width of the frame buffer.
		// @param height: Pointer to the height of the frame buffer.
		// @param framesCount: Number of frames to be used.
		// @param vsync: Whether vertical synchronization (vsync) is enabled.
		virtual void CreateFrame(std::uint32_t* width, std::uint32_t* height, std::uint32_t framesCount, bool vsync) override;

		// @brief Handle resizing of the swap chain.
		// @param width: New width of the swap chain.
		// @param height: New height of the swap chain.
		virtual void OnResize(std::uint32_t width, std::uint32_t height) override;

		// @brief Begin the rendering frame.
		virtual void BeginFrame() override;

		// @brief End the rendering frame.
		virtual void EndFrame() override;

		// @brief Present the rendered frame to the swap chain.
		// @param timeout: The timeout duration for waiting on the fence.
		virtual void Present(std::uint32_t timeout) override;

		// @brief Destroy the Vulkan swap chain and associated resources.
		virtual void Destroy() override;

		// @brief Get the index of the current buffer.
		// @return The index of the current buffer.
		SHADE_INLINE virtual const std::uint32_t& GetCurrentBufferIndex() const override
		{
			return m_CurrentFrameIndex;
		}

		// @brief Get the current frame buffer.
		// @return Shared pointer to the current frame buffer.
		SHADE_INLINE virtual SharedPointer<FrameBuffer>& GetFrameBuffer() override
		{
			return m_FrameBuffers[m_CurrentImageIndex];
		}

		// @brief Get the color format of the swap chain.
		// @return The color format.
		SHADE_INLINE VkFormat GetColorFormat() const 
		{
			return m_ColorFormat; 
		}

		// @brief Get the width of the swap chain.
		// @return The width.
		SHADE_INLINE std::uint32_t GetWidth() const 
		{
			return m_Width;
		}

		// @brief Get the height of the swap chain.
		// @return The height.
		SHADE_INLINE std::uint32_t GetHeight() const 
		{ 
			return m_Height; 
		}

		// @brief Get the Vulkan command buffer for the current frame.
		// @return Vulkan command buffer.
		SHADE_INLINE VkCommandBuffer GetVulkanCommandBuffer() 
		{
			return m_CommandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(m_CurrentFrameIndex); 
		}

		// @brief Create a Vulkan surface for the specified GLFW window.
		// @param window: The GLFW window for which to create the surface.
		void CreateSurface(GLFWwindow* window);

		// @brief Acquire the next image from the swap chain.
		// @param timeout: Timeout duration for acquiring the image.
		// @return The index of the acquired image.
		std::uint32_t AcquireNextImage(std::uint64_t timeout = UINT64_MAX);
	private:
		// Shared pointer to the Vulkan device.
		SharedPointer<VulkanDevice> m_LogicalDevice;

		// Vulkan surface for the swap chain.
		VkSurfaceKHR m_Surface;

		// Color format and color space for the swap chain.
		VkFormat m_ColorFormat = VK_FORMAT_UNDEFINED;
		VkColorSpaceKHR m_ColorSpace;

		// Vulkan swap chain handle.
		VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;

		// Submit info for command buffer submission.
		VkSubmitInfo m_SubmitInfo;

		// Synchronization objects.
		bool m_VSync = false;

		std::uint32_t m_Width = 0;
		std::uint32_t m_Height = 0;
		std::uint32_t m_QueueNodeIndex = UINT32_MAX;

		std::uint32_t m_ImageCount = 0;
		Semaphores    m_Semaphores;

		std::uint32_t m_CurrentImageIndex = 0;
		std::uint32_t m_CurrentFrameIndex = 0;

		// Retrieve the image format and color space for the swap chain.
		
		void GetImageFormatAndColorSpace();

		// Command buffer for layout transitions.
		SharedPointer<RenderCommandBuffer> m_LayoutTransitionCommandBuffer;
	};
}
