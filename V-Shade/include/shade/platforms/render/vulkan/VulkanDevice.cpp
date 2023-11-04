#include "shade_pch.h"
#include "VulkanDevice.h"
#include <shade/platforms/render/vulkan/VulkanContext.h>

shade::VulkanPhysicalDevice::VulkanPhysicalDevice(const SystemsRequirements& requirements)
{
	auto& context = VulkanContext::GetInstance();
	// Get all physical devices.
	std::uint32_t physicalDeviceCount = 0;
	VK_CHECK_RESULT(vkEnumeratePhysicalDevices(context.Instance, &physicalDeviceCount, nullptr), "There are no devices which support Vulkan!");
	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	VK_CHECK_RESULT(vkEnumeratePhysicalDevices(context.Instance, &physicalDeviceCount, physicalDevices.data()), "There are no devices which support Vulkan!");

	/* If we found some devices that support Vulkan so we need to choose one which fits requirements. */
	for (auto& device : physicalDevices)
	{
		/* Getting all device propeties. */
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);

		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(device, &features);

		VkPhysicalDeviceMemoryProperties memory;
		vkGetPhysicalDeviceMemoryProperties(device, &memory);

		//TODO: If no one of devices are not meeting requirement we heed to handle this, bacuse we just crashing right now
		if (DeviceMeetsRequirements(requirements, device, properties.deviceType))
		{
			m_PhysicalDevice = device;
			m_PhysicalDeviceFeatures = features;
			m_PhysicalDeviceMemoryProperties = memory;
			m_PhysicalDeviceProperties = properties;
			m_DepthFormat = FindDepthFormat(m_PhysicalDevice);
			break;
		}
	}
}

VkFormat shade::VulkanPhysicalDevice::FindDepthFormat(VkPhysicalDevice& device)
{
	// Since all depth formats may be optional, we need to find a suitable depth format to use
		// Start with the highest precision packed format
	std::vector<VkFormat> depthFormats = {
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	// TODO: Move to VulkanPhysicalDevice
	for (auto& format : depthFormats)
	{
		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &formatProps);
		// Format must support depth stencil attachment for optimal tiling
		if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			return format;
	}
	return VK_FORMAT_UNDEFINED;
}

shade::SharedPointer<shade::VulkanPhysicalDevice> shade::VulkanPhysicalDevice::Create(const SystemsRequirements& requirements)
{
	return SharedPointer<VulkanPhysicalDevice>::Create(requirements);
}

shade::VulkanPhysicalDevice::~VulkanPhysicalDevice()
{
}

const VkPhysicalDeviceFeatures& shade::VulkanPhysicalDevice::GetPhysicalDeviceFeatures() const
{
	return m_PhysicalDeviceFeatures;
}

const VkPhysicalDeviceMemoryProperties& shade::VulkanPhysicalDevice::GetPhysicalDeviceMemoryProperties() const
{
	return m_PhysicalDeviceMemoryProperties;
}

const VkPhysicalDeviceProperties& shade::VulkanPhysicalDevice::GetPhysicalDeviceProperties() const
{
	return m_PhysicalDeviceProperties;
}

const std::unordered_set<std::string>& shade::VulkanPhysicalDevice::GetSupportedExtensions() const
{
	return m_SupportedExtensions;
}

const std::vector<const char*>& shade::VulkanPhysicalDevice::GetRequiredExtensions() const
{
	return m_RequiredExtensions;
}

const shade::VulkanPhysicalDevice::QueueFamilyIndices& shade::VulkanPhysicalDevice::GetQueueFamilyIndices() const
{
	return m_QueueFamilyIndices;
}

const VkPhysicalDevice& shade::VulkanPhysicalDevice::GetDevice() const
{
	return m_PhysicalDevice;
}

const VkFormat& shade::VulkanPhysicalDevice::GetDepthForamt() const
{
	return m_DepthFormat;
}

const bool shade::VulkanPhysicalDevice::IsDeviceReady() const
{
	return (m_PhysicalDevice) ? true : false;
}

bool shade::VulkanPhysicalDevice::DeviceMeetsRequirements(const SystemsRequirements& requirements, VkPhysicalDevice& device, const VkPhysicalDeviceType& deviceType)
{
	// If gpu is discrete and we want it.
	if (requirements.GPU.Discrete)
		if (deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			return false;

	// Trying to find family queue count.
	std::uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, 0);
	if (queueFamilyCount == 0)
		SHADE_CORE_ERROR("Physical device queue family count is 0!");

	m_QueueFamilyProperties.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, m_QueueFamilyProperties.data());

	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	if (extensionCount > 0)
	{
		std::vector<VkExtensionProperties> extensions(extensionCount);
		if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, &extensions.front()) == VK_SUCCESS)
		{
			SHADE_CORE_DEBUG("Selected physical device has {0} extensions.", extensions.size());
			for (const auto& extension : extensions)
				m_SupportedExtensions.emplace(extension.extensionName);
		}
	}
	else
		return false;

	// As far as we are working with swapchain we need this extension to be enabled.
	m_RequiredExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
	m_RequiredExtensions.push_back(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
	m_RequiredExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	m_RequiredExtensions.push_back(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
	m_RequiredExtensions.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
	

	bool isNotFound = false;
	for (auto& ext : m_RequiredExtensions)
	{
		if (m_SupportedExtensions.find(ext) == m_SupportedExtensions.end())
		{
			SHADE_CORE_WARNING("Graphic device doesn't support required exstension: {0}", ext);
			isNotFound = true;
		}	
	}
	// Just to print them all and quit after. 
	if(isNotFound) return false;
		
	static const float defaultQueuePriority(0.0f);
	// TODO: Get from requirements
	int requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
	QueueFamilyIndices queueFamilyIndices = GetQueueFamilyIndices(requestedQueueTypes);

	// Graphics queue
	if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
	{
		VkDeviceQueueCreateInfo queueInfo{};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = queueFamilyIndices.Graphics;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &defaultQueuePriority;
		m_QueueCreateInfos.push_back(queueInfo);
	}
	// Compute queue
	if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
	{
		if (queueFamilyIndices.Compute != queueFamilyIndices.Graphics)
		{
			// If compute family index differs, we need an additional queue create info for the compute queue
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamilyIndices.Compute;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			m_QueueCreateInfos.push_back(queueInfo);
		}
	}
	// Transfer queue
	if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
	{
		if ((queueFamilyIndices.Transfer != queueFamilyIndices.Graphics) && (queueFamilyIndices.Transfer != queueFamilyIndices.Compute))
		{
			// If compute family index differs, we need an additional queue create info for the compute queue
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamilyIndices.Transfer;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			m_QueueCreateInfos.push_back(queueInfo);
		}
	}

	if (queueFamilyIndices.Graphics > -1 && queueFamilyIndices.Transfer > -1 && queueFamilyIndices.Compute > -1)
	{
		m_QueueFamilyIndices = queueFamilyIndices;
		// TODO: There
		return true;
	}

	return false;
}

shade::VulkanPhysicalDevice::QueueFamilyIndices shade::VulkanPhysicalDevice::GetQueueFamilyIndices(const int& flags)
{
	QueueFamilyIndices indices;

	// Dedicated queue for compute
	// Try to find a queue family index that supports compute but not graphics
	if (flags & VK_QUEUE_COMPUTE_BIT)
	{
		for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++)
		{
			auto& queueFamilyProperties = m_QueueFamilyProperties[i];
			if ((queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) && ((queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
			{
				indices.Compute = i;
				break;
			}
		}
	}

	// Dedicated queue for transfer
	// Try to find a queue family index that supports transfer but not graphics and compute
	if (flags & VK_QUEUE_TRANSFER_BIT)
	{
		for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++)
		{
			auto& queueFamilyProperties = m_QueueFamilyProperties[i];
			if ((queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT) && ((queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
			{
				indices.Transfer = i;
				break;
			}
		}
	}

	// For other queue types or if no separate compute queue is present, return the first one to support the requested flags
	for (uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++)
	{
		if ((flags & VK_QUEUE_TRANSFER_BIT) && indices.Transfer == -1)
		{
			if (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
				indices.Transfer = i;
		}

		if ((flags & VK_QUEUE_COMPUTE_BIT) && indices.Compute == -1)
		{
			if (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
				indices.Compute = i;
		}

		if (flags & VK_QUEUE_GRAPHICS_BIT)
		{
			if (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indices.Graphics = i;
		}
	}

	return indices;
}

shade::VulkanDevice::VulkanDevice(const SharedPointer<VulkanPhysicalDevice>& physicalDevice):
	m_PhysicalDevice(physicalDevice)
{
	// Create logical device 
	VkDeviceCreateInfo deviceCreateInfo
	{ 
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.flags = 0,
		.queueCreateInfoCount = static_cast<uint32_t>(physicalDevice->m_QueueCreateInfos.size()),
		.pQueueCreateInfos = physicalDevice->m_QueueCreateInfos.data(),
	};

	deviceCreateInfo.pQueueCreateInfos = physicalDevice->m_QueueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &physicalDevice->GetPhysicalDeviceFeatures();

	//deviceCreateInfo.pEnabledFeatures = &physicalDevice->GetPhysicalDeviceFeatures();

	if (m_PhysicalDevice->GetRequiredExtensions().size() > 0)
	{
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_PhysicalDevice->GetRequiredExtensions().size());
		deviceCreateInfo.ppEnabledExtensionNames = m_PhysicalDevice->GetRequiredExtensions().data();
		
		static VkPhysicalDeviceVulkan13Features  physicalDeviceVulkan13Features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		static VkPhysicalDeviceVulkan12Features  physicalDeviceVulkan12Features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };

		// Allows us to use gl_Layer assigment in vertex shader instend of geometry
		physicalDeviceVulkan12Features.shaderOutputViewportIndex = true;
		physicalDeviceVulkan12Features.shaderOutputLayer = true;
		physicalDeviceVulkan12Features.pNext = &physicalDeviceVulkan13Features;
		// Dynamic Render
		physicalDeviceVulkan13Features.dynamicRendering = VK_TRUE;
		//Compute shader use "LocalSizeId"
		physicalDeviceVulkan13Features.maintenance4 = VK_TRUE;
		deviceCreateInfo.pNext = &physicalDeviceVulkan12Features;
	}

	VK_CHECK_RESULT(vkCreateDevice(physicalDevice->GetDevice(), &deviceCreateInfo, VK_NULL_HANDLE, &m_LogicalDevice), "Failed to create logical device!");

	// Get a graphics queue from the device.
	vkGetDeviceQueue(m_LogicalDevice, m_PhysicalDevice->m_QueueFamilyIndices.Graphics, 0, &m_GraphicsQueue);
	// Get a transfer queue from the device.
	vkGetDeviceQueue(m_LogicalDevice, m_PhysicalDevice->m_QueueFamilyIndices.Transfer, 0, &m_TransferQueue);
	// Get a compute queue from the device.
	vkGetDeviceQueue(m_LogicalDevice, m_PhysicalDevice->m_QueueFamilyIndices.Compute, 0, &m_ComputeQueue);
	// TIP: Present queue in Swapchain !

	VkCommandPoolCreateInfo cmdPoolInfo 
	{
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, // sType
		VK_NULL_HANDLE, // pNext
		VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
	};
	
	cmdPoolInfo.queueFamilyIndex = m_PhysicalDevice->m_QueueFamilyIndices.Graphics;
	VK_CHECK_RESULT(vkCreateCommandPool(m_LogicalDevice, &cmdPoolInfo, nullptr, &m_GraphicCommandPool), "Failed to create command pool!");

	cmdPoolInfo.queueFamilyIndex = m_PhysicalDevice->m_QueueFamilyIndices.Transfer;
	VK_CHECK_RESULT(vkCreateCommandPool(m_LogicalDevice, &cmdPoolInfo, nullptr, &m_TransferCommandPool), "Failed to create command pool!");

	cmdPoolInfo.queueFamilyIndex = m_PhysicalDevice->m_QueueFamilyIndices.Compute;
	VK_CHECK_RESULT(vkCreateCommandPool(m_LogicalDevice, &cmdPoolInfo, nullptr, &m_ComputeCommandPool), "Failed to create command pool!");
}

shade::SharedPointer<shade::VulkanDevice> shade::VulkanDevice::Create(const SharedPointer<VulkanPhysicalDevice>& physicalDevice)
{
	return SharedPointer<VulkanDevice>::Create(physicalDevice);
}

shade::VulkanDevice::~VulkanDevice()
{
}

const shade::SharedPointer<shade::VulkanPhysicalDevice>& shade::VulkanDevice::GetPhysicalDevice() const
{
	return m_PhysicalDevice;
}

const VkDevice& shade::VulkanDevice::GetLogicalDevice() const
{
	return m_LogicalDevice;
}

void shade::VulkanDevice::Destroy()
{
	auto& instance = VulkanContext::GetInstance();

	vkDestroyCommandPool(m_LogicalDevice, m_GraphicCommandPool,  instance.AllocationCallbaks);
	vkDestroyCommandPool(m_LogicalDevice, m_TransferCommandPool, instance.AllocationCallbaks);
	vkDestroyCommandPool(m_LogicalDevice, m_ComputeCommandPool,  instance.AllocationCallbaks);

	vkDeviceWaitIdle(m_LogicalDevice);
	vkDestroyDevice(m_LogicalDevice, instance.AllocationCallbaks);
}
