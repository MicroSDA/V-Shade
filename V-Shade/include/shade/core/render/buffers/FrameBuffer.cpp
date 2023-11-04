#include "shade_pch.h"
#include "FrameBuffer.h"
#include <shade/core/render/RenderAPI.h>
#include <shade/platforms/render/vulkan/buffers/VulkanFrameBuffer.h>
#include <shade/core/application/Application.h>

shade::SharedPointer<shade::FrameBuffer> shade::FrameBuffer::Create(const FrameBuffer::Specification& specification)
{
	switch (RenderAPI::GetCurrentAPI())
	{
		case RenderAPI::API::None:  SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
		case RenderAPI::API::Vulkan: return SharedPointer<VulkanFrameBuffer>::Create(VulkanContext::GetDevice()->GetLogicalDevice(), VulkanContext::GetInstance(), specification);
		default: SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	}
}

shade::SharedPointer<shade::FrameBuffer> shade::FrameBuffer::Create(const FrameBuffer::Specification& specification, const std::vector<SharedPointer<render::Image2D>>& images)
{
	switch (RenderAPI::GetCurrentAPI())
	{
		case RenderAPI::API::None:  SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
		case RenderAPI::API::Vulkan: return SharedPointer<VulkanFrameBuffer>::Create(VulkanContext::GetDevice()->GetLogicalDevice(), VulkanContext::GetInstance(), specification, images);
		default: SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	}
}

std::vector<shade::SharedPointer<shade::FrameBuffer>> shade::FrameBuffer::CreateFromSwapChain()
{
	switch (RenderAPI::GetCurrentAPI())
	{
		case RenderAPI::API::None:  SHADE_CORE_ERROR("Undefined render API!");
		case RenderAPI::API::Vulkan: return Application::GetWindow()->GetSwapChain()->GetFrameBuffers();
		default: SHADE_CORE_ERROR("Undefined render API!");
	}
}

shade::FrameBuffer::Specification& shade::FrameBuffer::GetSpecification()
{
	return const_cast<FrameBuffer::Specification&>(const_cast<const FrameBuffer*>(this)->GetSpecification());
}

const shade::FrameBuffer::Specification& shade::FrameBuffer::GetSpecification() const
{
	return m_Specification;
}
