#include "shade_pch.h"
#include "RenderAPI.h"
#include <shade/platforms/render/vulkan/VulkanRenderAPI.h>

shade::RenderAPI::API shade::RenderAPI::m_sRenderAPI = shade::RenderAPI::API::None;
std::uint32_t shade::RenderAPI::m_sCurrentFrameIndex = 0;
shade::SystemsRequirements shade::RenderAPI::m_sSystemsRequirements;

shade::render::SubmitedSceneRenderData shade::RenderAPI::m_sSubmitedSceneRenderData;
ankerl::unordered_dense::map<std::size_t, shade::render::SubmitedInstances> shade::RenderAPI::m_sSubmitedPipelines;
shade::RenderAPI::SceneRenderData shade::RenderAPI::m_sSceneRenderData;
shade::RenderAPI::RenderSettings shade::RenderAPI::m_sRenderSettings;


std::uint32_t shade::RenderAPI::GetCurrentFrameIndex()
{
	return m_sCurrentFrameIndex;
}

std::uint32_t shade::RenderAPI::GetFramesCount()
{
	return m_sSystemsRequirements.FramesInFlight;
}

shade::RenderAPI::API shade::RenderAPI::GetCurrentAPI()
{
    return m_sRenderAPI;
}

const char* shade::RenderAPI::GetCurrentAPIAsString()
{
	switch (m_sRenderAPI)
	{
		case API::None:		return "None";
		case API::OpenGL:	return "OpenGL";
		case API::Vulkan:	return "Vulkan";
	}
}

shade::UniquePointer<shade::RenderAPI> shade::RenderAPI::Create(const RenderAPI::API& api)
{
    m_sRenderAPI = api;
	switch (m_sRenderAPI)
	{
		case RenderAPI::API::Vulkan: return UniquePointer<VulkanRenderAPI>::Create();
		default:SHADE_CORE_ERROR("Only Vulkan api is supported!"); return nullptr;
	}

}

void shade::RenderAPI::SetCurrentFrameIndex(std::uint32_t frameIndex)
{
	m_sCurrentFrameIndex = frameIndex;
}
