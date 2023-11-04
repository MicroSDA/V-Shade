#pragma once
#include <shade/core/render/RenderContext.h>
#include <shade/utils/Logger.h>

#include <shade/platforms/render/vulkan/VKUtils.h>
#include <shade/platforms/render/vulkan/VulkanDevice.h>
#include <GLFW/glfw3.h>

namespace shade
{
	class VulkanContext : public RenderContext
	{
	public:
		struct VulkanInstance
		{
			VkInstance Instance = VK_NULL_HANDLE;
			VkAllocationCallbacks* AllocationCallbaks = VK_NULL_HANDLE;
		};
	public:
		VulkanContext();
		virtual ~VulkanContext() = default;
		virtual void Initialize(const SystemsRequirements& requirements) override;
		virtual void ShutDown() override;


		static VulkanInstance& GetInstance();
		static SharedPointer<VulkanPhysicalDevice>& GetPhysicalDevice();
		static SharedPointer<VulkanDevice>& GetDevice();
	private:
		static VulkanInstance m_sVulkanInstance;
		static SharedPointer<VulkanPhysicalDevice> m_sPhysicalDevice;
		static SharedPointer<VulkanDevice> m_sDevice;


#ifdef SHADE_DEBUG
		VkDebugUtilsMessengerEXT m_DebugUtilsMessengerEXT = VK_NULL_HANDLE;
#endif // SHADE_DEBUG
	private:
		bool CheckLayersSupport(std::vector<const char*>& layers);
		void CreateDebugMessanger();

	};
}
