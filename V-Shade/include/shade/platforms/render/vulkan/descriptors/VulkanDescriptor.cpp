#include "shade_pch.h"
#include "VulkanDescriptor.h"
#include <shade/core/render/RenderAPI.h>

std::vector<std::unordered_map<std::size_t, shade::VulkanDescriptorSetPool>> shade::VulkanDescriptorsManager::m_sDescriptorPools;
std::vector<std::unordered_map<std::size_t, shade::VulkanDescriptorSet>>  shade::VulkanDescriptorsManager::m_sDescriptorSets;

VkDevice shade::VulkanDescriptorsManager::m_sVkDevice = VK_NULL_HANDLE;
shade::VulkanContext::VulkanInstance shade::VulkanDescriptorsManager::m_sVkInstance;

std::uint32_t  shade::VulkanDescriptorsManager::m_sFramesCount = 0;

void shade::VulkanDescriptorsManager::Initialize(VkDevice& device, const VulkanContext::VulkanInstance& instance, std::uint32_t framesCount)
{
	m_sVkDevice = device;
	m_sVkInstance = instance;
	m_sFramesCount = framesCount;

	if (m_sDescriptorPools.size() != m_sFramesCount)
		m_sDescriptorPools.resize(m_sFramesCount);

	if (m_sDescriptorSets.size() != m_sFramesCount)
		m_sDescriptorSets.resize(m_sFramesCount);

}

void shade::VulkanDescriptorsManager::ShutDown()
{
	m_sDescriptorPools.clear();
	m_sDescriptorSets.clear();
}

void shade::VulkanDescriptorsManager::ResetAllDescripotrs(std::uint32_t frameIndex)
{
	m_sDescriptorSets.at(frameIndex).clear();
	m_sDescriptorPools.at(frameIndex).clear();
}

shade::VulkanDescriptorSet& shade::VulkanDescriptorsManager::ReciveDescriptor(
	const VulkanDescriptorSetLayout& layout,
	const DescriptorBufferBindings& bufferInfos,
	std::uint32_t frameIndex)
{

	auto& descriptorPool = GetResource(m_sDescriptorPools.at(frameIndex), layout);
	return GetResource(m_sDescriptorSets.at(frameIndex), layout, descriptorPool, bufferInfos);
}
