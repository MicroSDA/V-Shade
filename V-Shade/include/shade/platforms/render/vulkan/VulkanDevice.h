#pragma once
#include <shade/core/system/SystemsRequirements.h>
#include <shade/core/memory/Memory.h>
#include <shade/platforms/render/vulkan/VKUtils.h>

namespace shade
{
	// Class that represent system physical device.
	class VulkanPhysicalDevice
	{
	public:
		struct QueueFamilyIndices
		{
			std::int32_t Present = -1;
			std::int32_t Graphics = -1;
			std::int32_t Compute = -1;
			std::int32_t Transfer = -1;
		};

		VulkanPhysicalDevice(const SystemsRequirements& requirements);
		static SharedPointer<VulkanPhysicalDevice> Create(const SystemsRequirements& requirements);
		virtual ~VulkanPhysicalDevice();

		const VkPhysicalDeviceFeatures& GetPhysicalDeviceFeatures() const;
		const VkPhysicalDeviceMemoryProperties& GetPhysicalDeviceMemoryProperties() const;
		const VkPhysicalDeviceProperties& GetPhysicalDeviceProperties() const;

		const std::unordered_set<std::string>& GetSupportedExtensions() const;
		const std::vector<const char*>& GetRequiredExtensions() const;
		const QueueFamilyIndices& GetQueueFamilyIndices() const;

		const VkPhysicalDevice& GetDevice() const;
		const VkFormat& GetDepthForamt() const;

		// Check if required device has been found.
		const bool IsDeviceReady() const;
	private:
		std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties;
		std::vector<VkDeviceQueueCreateInfo> m_QueueCreateInfos;
		std::vector<const char*> m_RequiredExtensions;
		std::unordered_set<std::string> m_SupportedExtensions;

		VkPhysicalDeviceFeatures m_PhysicalDeviceFeatures;
		VkPhysicalDeviceMemoryProperties m_PhysicalDeviceMemoryProperties;
		VkPhysicalDeviceProperties m_PhysicalDeviceProperties;

		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkFormat m_DepthFormat = VK_FORMAT_UNDEFINED;

		QueueFamilyIndices m_QueueFamilyIndices;
	private:
		bool DeviceMeetsRequirements(const SystemsRequirements& requirements, VkPhysicalDevice& device, const VkPhysicalDeviceType& deviceType);
		QueueFamilyIndices GetQueueFamilyIndices(const int& flags);
		VkFormat FindDepthFormat(VkPhysicalDevice& device);

		friend class VulkanDevice;
		friend class VulkanSwapChain;
	};
	// Class that represent cogical device.
	class VulkanDevice
	{
	public:
		VulkanDevice(const SharedPointer<VulkanPhysicalDevice>& physicalDevice);
		static SharedPointer<VulkanDevice> Create(const SharedPointer<VulkanPhysicalDevice>& physicalDevice);
		virtual ~VulkanDevice();

		const SharedPointer<VulkanPhysicalDevice>& GetPhysicalDevice() const;
		const VkDevice& GetLogicalDevice() const;
		void Destroy();


		VkQueue GetPresentQueue()  { return m_PresentQueue; }
		VkQueue GetGraphicsQueue() { return m_GraphicsQueue; }
		VkQueue GetTransferQueue() { return m_TransferQueue; }
		VkQueue GetComputeQueue()  { return m_ComputeQueue;  }

	private:
		SharedPointer<VulkanPhysicalDevice> m_PhysicalDevice;
		VkDevice m_LogicalDevice = VK_NULL_HANDLE;
		VkQueue  m_GraphicsQueue, m_TransferQueue, m_ComputeQueue, m_PresentQueue;
		VkCommandPool m_GraphicCommandPool = VK_NULL_HANDLE, m_TransferCommandPool = VK_NULL_HANDLE, m_ComputeCommandPool = VK_NULL_HANDLE;

		friend class VulkanSwapChain;
	};
}