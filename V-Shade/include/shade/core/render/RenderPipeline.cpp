#include "shade_pch.h"
#include "RenderPipeline.h"
#include <shade/core/render/RenderAPI.h>
#include <shade/platforms/render/vulkan/VulkanPipeline.h>
#include <shade/platforms/render/vulkan/VulkanComputePipeline.h>

shade::SharedPointer<shade::RenderPipeline> shade::RenderPipeline::Create(const Pipeline::Specification& specification)
{
	switch (RenderAPI::GetCurrentAPI())
	{
		case RenderAPI::API::None:  SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
		case RenderAPI::API::Vulkan: return SharedPointer<VulkanPipeline>::Create(specification);
		default: SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	}
}

shade::SharedPointer<shade::ComputePipeline> shade::ComputePipeline::Create(const Pipeline::Specification& specification)
{
	switch (RenderAPI::GetCurrentAPI())
	{
		case RenderAPI::API::None:  SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
		case RenderAPI::API::Vulkan: return SharedPointer<VulkanComputePipeline>::Create(VulkanContext::GetLogicalDevice()->GetDevice(), VulkanContext::GetInstance(), specification);
		default: SHADE_CORE_ERROR("Undefined render API!"); return nullptr;
	}
}

shade::Pipeline::Specification& shade::Pipeline::GetSpecification()
{
	return m_Specification;
}

const shade::Pipeline::Specification& shade::Pipeline::GetSpecification() const
{
	return m_Specification;
}

float shade::Pipeline::GetTimeQuery() const
{
	return m_TimeQuery;
}

void shade::Pipeline::SetTimeQuery(float time)
{
	m_TimeQuery = time;
}

bool shade::Pipeline::IsActive() const
{
	return m_IsActive;
}

void shade::Pipeline::SetActive(bool is)
{
	m_IsActive = is;
}
