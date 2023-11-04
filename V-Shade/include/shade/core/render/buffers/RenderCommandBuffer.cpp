#include "shade_pch.h"
#include "RenderCommandBuffer.h"
#include <shade/core/render/RenderAPI.h>
#include <shade/platforms/render/vulkan/buffers/VulkanCommandBuffer.h>
#include <shade/core/application/Application.h>

std::array<std::mutex, shade::RenderCommandBuffer::Family::FAMILY_MAX_ENUM> shade::RenderCommandBuffer::m_sMutexs;

shade::SharedPointer<shade::RenderCommandBuffer> shade::RenderCommandBuffer::Create(const Type& type, const Family& family, const std::uint32_t& framesInFlight, const std::string& name)
{
	switch (RenderAPI::GetCurrentAPI())
	{
		case RenderAPI::API::None:  SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
		case RenderAPI::API::Vulkan: return SharedPointer<VulkanCommandBuffer>::Create(type, family, framesInFlight, name);
		default: SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	}
}

shade::SharedPointer<shade::RenderCommandBuffer> shade::RenderCommandBuffer::CreateFromSwapChain()
{
	switch (RenderAPI::GetCurrentAPI())
	{
		case RenderAPI::API::None:  SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
		case RenderAPI::API::Vulkan: return Application::GetWindow()->GetSwapChain()->GetCommandBuffer();
		default: SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	}
}

