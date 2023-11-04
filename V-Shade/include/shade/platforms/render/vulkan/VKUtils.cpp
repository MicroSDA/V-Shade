#include "shade_pch.h"
#include "VKUtils.h"
#include <shade/platforms/render/vulkan/VulkanContext.h>

VKAPI_ATTR VkBool32 VKAPI_CALL shade::VKUtils::VulkanMessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* data, void* userData)
{
	switch (severity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		SHADE_CORE_TRACE("VK_RENDER_INFO: {0}", data->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		SHADE_CORE_INFO("VK_RENDER_INFO: {0}", data->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		SHADE_CORE_WARNING("VK_RENDER_INFO: {0}", data->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		SHADE_CORE_ERROR("VK_RENDER_INFO: {0}", data->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
		break;
	default:
		break;
	}
	return VK_FALSE;
}

void shade::VKUtils::SetDebugObjectName(VkInstance instance, const std::string& name, VkDevice device, const VkObjectType objectType, const void* objectHandle)
{
	VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfoEXT = {
		VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
		VK_NULL_HANDLE,
		objectType,
		reinterpret_cast<std::uint64_t>(objectHandle),
		name.c_str()

	};
	// TODO: need some refactor !
	PFN_vkSetDebugUtilsObjectNameEXT CreateDebugReportCallback = VK_NULL_HANDLE;
	CreateDebugReportCallback = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
	if (CreateDebugReportCallback)
		CreateDebugReportCallback(device, &debugUtilsObjectNameInfoEXT);
	
	//VK_CHECK_RESULT(vkSetDebugUtilsObjectNameEXT(device, &debugUtilsObjectNameInfoEXT), "Failed to set debug utils object name :'{0}'", name);
}

std::uint32_t shade::VKUtils::FindMemoryType(VkPhysicalDevice device, const std::uint32_t& typeFilter, const VkMemoryPropertyFlags& properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	SHADE_CORE_ERROR("Unable to find suitable memory type!")
}

VkFormat shade::VKUtils::ToVulkanImageFormat(const render::Image::Format& format)
{
	switch (format)
	{
		case render::Image::Format::RED8UN:					return VK_FORMAT_R8_UNORM;
		case render::Image::Format::RED8UI:					return VK_FORMAT_R8_UINT;
		case render::Image::Format::RED16UI:				return VK_FORMAT_R16_UINT;
		case render::Image::Format::RED32UI:				return VK_FORMAT_R32_UINT;
		case render::Image::Format::RED32F:					return VK_FORMAT_R32_SFLOAT;
		case render::Image::Format::RG8:				    return VK_FORMAT_R8G8_UNORM;
		case render::Image::Format::RG16F:					return VK_FORMAT_R16G16_SFLOAT;
		case render::Image::Format::RG32F:					return VK_FORMAT_R32G32_SFLOAT;
		case render::Image::Format::RGBA:					return VK_FORMAT_R8G8B8A8_UNORM;
		case render::Image::Format::BGRA:					return VK_FORMAT_B8G8R8A8_UNORM;
		case render::Image::Format::RGBA16F:				return VK_FORMAT_R16G16B16A16_SFLOAT;
		case render::Image::Format::RGBA32F:				return VK_FORMAT_R32G32B32A32_SFLOAT;
		case render::Image::Format::B10R11G11UF:			return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
		case render::Image::Format::DEPTH32FSTENCIL8UINT:	return VK_FORMAT_D32_SFLOAT_S8_UINT;
		case render::Image::Format::DEPTH32F:				return VK_FORMAT_D32_SFLOAT; 
		case render::Image::Format::DEPTH24STENCIL8:		return VulkanContext::GetDevice()->GetPhysicalDevice()->GetDepthForamt();

	default: return VK_FORMAT_UNDEFINED;
	}
}

// Without depth
shade::render::Image::Format shade::VKUtils::FromVulkanImageFormat(const VkFormat& format)
{
	switch (format)
	{
		case VK_FORMAT_R8_UNORM:						return render::Image::Format::RED8UN;
		case VK_FORMAT_R8_UINT:							return render::Image::Format::RED8UI;
		case VK_FORMAT_R16_UINT:						return render::Image::Format::RED16UI;
		case VK_FORMAT_R32_UINT:						return render::Image::Format::RED32UI;
		case VK_FORMAT_R32_SFLOAT:						return render::Image::Format::RED32F;
		case VK_FORMAT_R8G8_UNORM:						return render::Image::Format::RG8;
		case VK_FORMAT_R16G16_SFLOAT:					return render::Image::Format::RG16F;
		case VK_FORMAT_R32G32_SFLOAT:					return render::Image::Format::RG32F;
		case VK_FORMAT_R8G8B8A8_UNORM:					return render::Image::Format::RGBA;
		case VK_FORMAT_B8G8R8A8_UNORM:					return render::Image::Format::BGRA;
		case VK_FORMAT_R16G16B16A16_SFLOAT:				return render::Image::Format::RGBA16F;
		case VK_FORMAT_R32G32B32A32_SFLOAT:				return render::Image::Format::RGBA32F;
		case VK_FORMAT_B10G11R11_UFLOAT_PACK32:			return render::Image::Format::B10R11G11UF;
		case VK_FORMAT_D32_SFLOAT_S8_UINT:				return render::Image::Format::DEPTH32FSTENCIL8UINT;
		case VK_FORMAT_D32_SFLOAT:						return render::Image::Format::DEPTH32F;


		default: return render::Image::Format::Undefined;
	}
}

bool shade::VKUtils::IsDepthFormat(const render::Image::Format& format)
{
	if (format == render::Image::Format::DEPTH32F)
		return true;

	return false;
}

bool shade::VKUtils::IsDepthStencilFormat(const render::Image::Format& format)
{
	if (format == render::Image::Format::DEPTH24STENCIL8 || format == render::Image::Format::DEPTH32FSTENCIL8UINT)
		return true;

	return false;
}

