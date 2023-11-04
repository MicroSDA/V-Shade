#include "shade_pch.h"
#include "RenderContext.h"
#include <shade/core/render/RenderAPI.h>
#include <shade/platforms/render/vulkan/VulkanContext.h>

shade::UniquePointer<shade::RenderContext> shade::RenderContext::Create()
{
	switch (RenderAPI::GetCurrentAPI())
	{
	case RenderAPI::API::None:  SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	case RenderAPI::API::Vulkan: return UniquePointer<VulkanContext>::Create();
	default: SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	}
}
