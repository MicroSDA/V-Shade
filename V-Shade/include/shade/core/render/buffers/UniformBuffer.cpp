#include "shade_pch.h"
#include "UniformBuffer.h"
#include <shade/core/render/RenderAPI.h>
#include <shade/platforms/render/vulkan/buffers/VulkanUniformBuffer.h>

shade::SharedPointer<shade::UniformBuffer> shade::UniformBuffer::Create(Usage usage, std::uint32_t binding, std::uint32_t size, std::uint32_t framesCount, std::size_t resizeThreshold)
{
	switch (RenderAPI::GetCurrentAPI())
	{
		case RenderAPI::API::None:  SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
		case RenderAPI::API::Vulkan: return SharedPointer<VulkanUniformBuffer>::Create(VulkanContext::GetLogicalDevice()->GetDevice(), VulkanContext::GetInstance(), usage, binding, size, framesCount, resizeThreshold);
		default: SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	}
}

bool shade::UniformBuffer::HasToBeResized(std::uint32_t oldSize, std::uint32_t newSize, std::uint32_t threshold)
{
	// Checks if the new size is larger than the old size
	if (oldSize < newSize)
	{
		return true;
	}
	// Checks if the increase in size is more than the given threshold percentage of old size
	else if (oldSize > (newSize + ((oldSize / 100) * threshold)))
	{
		return true;
	}

	// If none of the above conditions are no true, returns false
	return false;
}
