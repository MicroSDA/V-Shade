#pragma once
#include <shade/core/render/SwapChain.h>
#include <shade/platforms/render/vulkan/VKUtils.h>
#include <shade/platforms/render/vulkan/VulkanDevice.h>
#include <shade/platforms/render/vulkan/VulkanContext.h>
#include <shade/platforms/render/vulkan/buffers/VulkanCommandBuffer.h>
#include <shade/platforms/render/vulkan/buffers/VulkanFrameBuffer.h>


namespace shade
{
	class VulkanSwapChain : public SwapChain
	{
	public:
		VulkanSwapChain() = default;
		virtual ~VulkanSwapChain() = default;

		virtual void Initialize() override;
		virtual void CreateFrame(std::uint32_t* width, std::uint32_t* height, std::uint32_t framesCount, bool vsync) override;
		virtual void OnResize(std::uint32_t width, std::uint32_t height) override;
		virtual void BeginFrame() override;
		virtual void EndFrame() override;
		virtual void Present(std::uint32_t timeout) override;
		virtual void Destroy() override;

		virtual const std::uint32_t& GetCurrentBufferIndex() const override;
		virtual SharedPointer<FrameBuffer>& GetFrameBuffer() override;
		VkCommandBuffer GetDrawCommandBuffer();
		VkFormat GetColorFormat() { return m_ColorFormat; }

		const std::uint32_t& GetWidth() const;
		const std::uint32_t& GetHeight() const;
		
		void CreateSurface(GLFWwindow* window);
		std::uint32_t AcquireNextImage();
		VkCommandBuffer GetVulkanCommandBuffer();
	private:

		friend class VulkanRenderAPI; // TODO: Remove

		SharedPointer<VulkanDevice> m_LogicalDevice;
		VkSurfaceKHR m_Surface;
		VkFormat m_ColorFormat = VK_FORMAT_UNDEFINED;
		VkColorSpaceKHR m_ColorSpace;
		VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
		VkSubmitInfo m_SubmitInfo;
		
		bool m_VSync = false;
		std::uint32_t m_Width = 0, m_Height = 0, m_QueueNodeIndex = -1;

		struct SwapChainImage
		{
			VkImage Image = VK_NULL_HANDLE;
			VkImageView View = VK_NULL_HANDLE;
		};

		std::uint32_t m_ImageCount = 0;
		struct
		{
			// Swap chain
			VkSemaphore PresentComplete = nullptr;
			// Command buffer
			VkSemaphore RenderComplete = nullptr;
		} m_Semaphores;
		std::uint32_t m_CurrentImageIndex = 0;
		std::uint32_t m_CurrentFrameIndex = 0;
	private:
		void GetImageFormatAndColorSpace();
		SharedPointer<RenderCommandBuffer> m_LayoutTransitionCommandBuffer;
	};
}
