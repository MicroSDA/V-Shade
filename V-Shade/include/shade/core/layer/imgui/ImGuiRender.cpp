#include "shade_pch.h"
#include "ImGuiRender.h"
#include <shade/core/render/RenderAPI.h>
#include <shade/platforms/render/vulkan/VulkanImGuiRender.h>

shade::SharedPointer<shade::ImGuiRender> shade::ImGuiRender::Create()
{
	switch (RenderAPI::GetCurrentAPI())
	{
		case RenderAPI::API::None:  SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
		case RenderAPI::API::Vulkan: return SharedPointer<VulkanImGuiRender>::Create();
		default: SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	}
    return SharedPointer<ImGuiRender>();
}

ImGuiContext* shade::ImGuiRender::GetImGuiContext()
{
	return m_ImGuiContext;
}
