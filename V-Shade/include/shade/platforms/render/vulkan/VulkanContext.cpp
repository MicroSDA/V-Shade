#include "shade_pch.h"
#include "VulkanContext.h"

shade::VulkanContext::VulkanInstance shade::VulkanContext::m_sVulkanInstance;

shade::SharedPointer<shade::VulkanPhysicalDevice> shade::VulkanContext::m_sPhysicalDevice;
shade::SharedPointer<shade::VulkanDevice> shade::VulkanContext::m_sDevice;


std::set<std::string> get_supported_extensions() {
	VkResult result = VK_SUCCESS;

	/*
	 * From the link above:
	 * If `pProperties` is NULL, then the number of extensions properties
	 * available is returned in `pPropertyCount`.
	 *
	 * Basically, gets the number of extensions.
	 */
	uint32_t count = 0;
	result = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
	if (result != VK_SUCCESS) {
		// Throw an exception or log the error
	}

	std::vector<VkExtensionProperties> extensionProperties(count);

	// Get the extensions
	result = vkEnumerateInstanceExtensionProperties(nullptr, &count, extensionProperties.data());
	if (result != VK_SUCCESS) {
		// Throw an exception or log the error
	}

	std::set<std::string> extensions;
	for (auto& extension : extensionProperties) {
		extensions.insert(extension.extensionName);
	}

	return extensions;
}

shade::VulkanContext::VulkanContext()
{
}

void shade::VulkanContext::Initialize(const SystemsRequirements& requirements)
{
	SHADE_CORE_DEBUG("Creating Vulkan Context.");
	/* Chech Vulkan support */
	if (glfwVulkanSupported() == GLFW_FALSE)
		SHADE_CORE_ERROR("GLFW must support Vulkan!");

	/* Check Vulkan Drivers version */
	uint32_t instanceVersion;
	vkEnumerateInstanceVersion(&instanceVersion);
	if (instanceVersion < VK_API_VERSION_1_3)
		SHADE_CORE_ERROR("Engine requires at least Vulkan version {}.{}.{}",
			VK_API_VERSION_MAJOR(VK_API_VERSION_1_3),
			VK_API_VERSION_MINOR(VK_API_VERSION_1_3),
			VK_API_VERSION_PATCH(VK_API_VERSION_1_3));

	/* Getting required extension */
	std::uint32_t requiredExtensionCount = 0;
	const char** requiredExtensions;
	requiredExtensions = glfwGetRequiredInstanceExtensions(&requiredExtensionCount);
	// Create set of extensions
	std::vector<const char*> extensions;
	for (std::uint32_t i = 0; i < requiredExtensionCount; i++)
		extensions.emplace_back(requiredExtensions[i]);


	auto s = get_supported_extensions();

#ifdef SHADE_DEBUG
	// Add extension for debug messanger.
	extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	extensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	//extensions.emplace_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
	//extensions.emplace_back("VK_EXT_depth_range_unrestricted");
#endif 

	extensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

	std::vector<const char*> layers;
#ifdef SHADE_DEBUG
	// Enable validation layers for debug purposes.
	layers.emplace_back("VK_LAYER_KHRONOS_validation");
	//// Print everething in console
	//layers.emplace_back("VK_LAYER_LUNARG_api_dump");
	if (CheckLayersSupport(layers));
		/*for (const auto& layer : layers)
			SHADE_CORE_DEBUG("Enabled layer: {0}", layer);*/
#endif // SHADE_DEBUG

	// Create application info.
	VkApplicationInfo applicationInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	applicationInfo.apiVersion = VK_API_VERSION_1_3; // At least vulkan 1.3
	applicationInfo.pEngineName = "Shade";
	applicationInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);

	applicationInfo.pApplicationName = "Shade"; // TODO: Make conigurable.
	applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);  // TODO: Make conigurable.
	applicationInfo.pNext = VK_NULL_HANDLE;

	// Create application instance info.
	VkInstanceCreateInfo instanceCreateInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size()); 
	instanceCreateInfo.ppEnabledExtensionNames = extensions.data(); // Enable extensions.
	instanceCreateInfo.enabledLayerCount = static_cast<std::uint32_t>(layers.size());
	instanceCreateInfo.ppEnabledLayerNames = layers.data(); // Enable layers.

	// Creating Vulkan instance.
	VK_CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, m_sVulkanInstance.AllocationCallbaks, &m_sVulkanInstance.Instance), "Failed to create Vulkan instance !");

#ifdef SHADE_DEBUG
	CreateDebugMessanger();
#endif // SHADE_DEBUG
	
	// Select physical device.
	m_sPhysicalDevice = VulkanPhysicalDevice::Create(requirements);
	if(!m_sPhysicalDevice->IsDeviceReady())
		SHADE_CORE_ERROR("Failed to find physical device that meets requirements!")
	// Create logical device based on the physical device.
	m_sDevice = VulkanDevice::Create(m_sPhysicalDevice);

	SHADE_CORE_INFO("Vulkan render initialized successfuly with following parameters:");
	auto& deviceProperties = m_sPhysicalDevice->GetPhysicalDeviceProperties();
	auto& deviceMemoryProperties = m_sPhysicalDevice->GetPhysicalDeviceMemoryProperties();

	SHADE_CORE_INFO("	 Selected GPU : {}", deviceProperties.deviceName);
	SHADE_CORE_INFO("	 GPU Driver version: {0}.{1}.{2}",
		VK_VERSION_MAJOR(deviceProperties.driverVersion),
		VK_VERSION_MINOR(deviceProperties.driverVersion),
		VK_VERSION_PATCH(deviceProperties.driverVersion));

	SHADE_CORE_INFO("	 Vulkan API version: {0}.{1}.{2}",
		VK_VERSION_MAJOR(deviceProperties.apiVersion),
		VK_VERSION_MINOR(deviceProperties.apiVersion),
		VK_VERSION_PATCH(deviceProperties.apiVersion));

	for (std::uint32_t j = 0; j < deviceMemoryProperties.memoryHeapCount; j++)
	{
		float memorySize = (((float)deviceMemoryProperties.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);
		if (deviceMemoryProperties.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
			SHADE_CORE_INFO("	 Local GPU memory: {0} gib", memorySize);
	}
}

void shade::VulkanContext::ShutDown()
{

	// Destroy debug messanger
#ifdef SHADE_DEBUG
	if (m_DebugUtilsMessengerEXT)
	{
		PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_sVulkanInstance.Instance, "vkDestroyDebugUtilsMessengerEXT");
		func(m_sVulkanInstance.Instance, m_DebugUtilsMessengerEXT, m_sVulkanInstance.AllocationCallbaks);
	}
#endif // SHADE_DEBUG

	m_sDevice->Destroy();
	vkDestroyInstance(m_sVulkanInstance.Instance, m_sVulkanInstance.AllocationCallbaks);
	m_sVulkanInstance.Instance = nullptr;
}

shade::VulkanContext::VulkanInstance& shade::VulkanContext::GetInstance()
{
	return m_sVulkanInstance;
}

shade::SharedPointer<shade::VulkanPhysicalDevice>& shade::VulkanContext::GetPhysicalDevice()
{
	return m_sPhysicalDevice;
}

shade::SharedPointer<shade::VulkanDevice> & shade::VulkanContext::GetDevice()
{
	return m_sDevice;
}

bool shade::VulkanContext::CheckLayersSupport(std::vector<const char*>& layers)
{
	std::uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (int i = 0; i < layers.size(); i++)
	{
		bool found = false;
		for (const auto& aL : availableLayers)
		{
			if (strcmp(layers[i], aL.layerName) == 0)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			SHADE_CORE_WARNING("{0} is not supported !", layers[i]);
			layers.erase(layers.begin() + i);
		}
	}

	return layers.size();
}

void shade::VulkanContext::CreateDebugMessanger()
{
#ifdef SHADE_DEBUG
	std::uint32_t severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	// for advance debugging
	// VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT

	VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
	debugUtilsCreateInfo.messageSeverity = severity;
	debugUtilsCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	debugUtilsCreateInfo.pfnUserCallback = VKUtils::VulkanMessageCallback;

	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_sVulkanInstance.Instance, "vkCreateDebugUtilsMessengerEXT");

	if (!vkCreateDebugUtilsMessengerEXT)
		SHADE_CORE_WARNING("Failed to create Vulkan debugger!")
	else
	{
		if (vkCreateDebugUtilsMessengerEXT(m_sVulkanInstance.Instance, &debugUtilsCreateInfo, m_sVulkanInstance.AllocationCallbaks, &m_DebugUtilsMessengerEXT) != VK_SUCCESS)
			SHADE_CORE_ERROR("Failed to create Vulkan debugger!")
		else
			SHADE_CORE_DEBUG("Vulkan debugger initialized successfully.");
	}
#endif // SHADE_DEBUG
}
