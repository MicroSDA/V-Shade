#pragma once
#include <shade/utils/Logger.h>
#include <shade/core/image/Image.h>
#include <vulkan/vulkan.h>

namespace shade
{
	namespace VKUtils
	{

		#define VK_CHECK_RESULT(expr, msg)					\
		{													\
			VkResult result = expr;							\
			if (result != VK_SUCCESS)						\
				SHADE_CORE_ERROR(msg);						\
		}

		VKAPI_ATTR VkBool32 VKAPI_CALL VulkanMessageCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT severity,
			VkDebugUtilsMessageTypeFlagsEXT type,
			const VkDebugUtilsMessengerCallbackDataEXT* data,
			void* userData);

		/* Give a user-friendly name to an object. */
		void SetDebugObjectName(VkInstance instance, const std::string& name, VkDevice device, const VkObjectType objectType, const void* objectHandle);

		std::uint32_t FindMemoryType(VkPhysicalDevice device, const std::uint32_t& typeFilter, const VkMemoryPropertyFlags& properties);

		VkFormat ToVulkanImageFormat(const render::Image::Format& format);
		render::Image::Format FromVulkanImageFormat(const VkFormat& format);
		// Check if image is some kind of depth.
		bool IsDepthFormat(const render::Image::Format& format);
		bool IsDepthStencilFormat(const render::Image::Format& format);
	}
}
