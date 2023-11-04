#include "shade_pch.h"
#include "SwapChain.h"
#include <shade/core/render/RenderAPI.h>
#include <shade/platforms/render/vulkan/VulkanSwapChain.h>

shade::UniquePointer<shade::SwapChain> shade::SwapChain::Create()
{
	switch (RenderAPI::GetCurrentAPI())
	{
	case RenderAPI::API::None:  SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	case RenderAPI::API::Vulkan: return UniquePointer<VulkanSwapChain>::Create();
	default: SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	}
}

void shade::SwapChain::SetFramesCount(std::uint32_t count)
{
	m_FramesCount = count;
}

std::uint32_t shade::SwapChain::GetFramesCount()
{
	return m_FramesCount;
}

shade::SharedPointer<shade::RenderCommandBuffer> shade::SwapChain::GetCommandBuffer()
{
	return m_CommandBuffer;
}
