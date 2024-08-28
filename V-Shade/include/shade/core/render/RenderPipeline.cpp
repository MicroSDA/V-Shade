#include "shade_pch.h"
#include "RenderPipeline.h"
#include <shade/core/render/RenderAPI.h>
#include <shade/platforms/render/vulkan/VulkanPipeline.h>
#include <shade/platforms/render/vulkan/VulkanComputePipeline.h>

shade::SharedPointer<shade::RenderPipeline> shade::RenderPipeline::Create(const RenderPipeline::Specification& specification)
{
	switch (RenderAPI::GetCurrentAPI())
	{
		case RenderAPI::API::None:  SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
		case RenderAPI::API::Vulkan: return SharedPointer<VulkanPipeline>::Create(specification);
		default: SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	}
}

shade::RenderPipeline::Specification& shade::RenderPipeline::GetSpecification()
{
	return const_cast<RenderPipeline::Specification&>(const_cast<const RenderPipeline*>(this)->GetSpecification());
}

const shade::RenderPipeline::Specification& shade::RenderPipeline::GetSpecification() const
{
	return m_Specification;
}

shade::SharedPointer<shade::ComputePipeline> shade::ComputePipeline::Create(const ComputePipeline::Specification& specification)
{
	switch (RenderAPI::GetCurrentAPI())
	{
		case RenderAPI::API::None:  SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
		case RenderAPI::API::Vulkan: return SharedPointer<VulkanComputePipeline>::Create(VulkanContext::GetLogicalDevice()->GetDevice(), VulkanContext::GetInstance(), specification);
		default: SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	}
}

