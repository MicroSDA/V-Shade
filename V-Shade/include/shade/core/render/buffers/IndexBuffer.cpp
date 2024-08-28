#include "shade_pch.h"
#include "IndexBuffer.h"
#include <shade/core/render/RenderAPI.h>
#include <shade/platforms/render/vulkan/buffers/VulkanIndexBuffer.h>

shade::SharedPointer<shade::IndexBuffer> shade::IndexBuffer::Create(Usage usage, std::uint32_t size, std::uint32_t resizeThreshold, const void* data)
{
	switch (RenderAPI::GetCurrentAPI())
	{
		case RenderAPI::API::None:  SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
		case RenderAPI::API::Vulkan: return SharedPointer<VulkanIndexBuffer>::Create(VulkanContext::GetLogicalDevice()->GetDevice(), VulkanContext::GetInstance(), usage, size, resizeThreshold, data);
		default: SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	}
}

bool shade::IndexBuffer::HasToBeResized(std::uint32_t oldSize, std::uint32_t newSize, std::uint32_t threshold)
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

