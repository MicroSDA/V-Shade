#include "shade_pch.h"
#include "VulkanDescriptorSet.h"

shade::VulkanDescriptorSet::VulkanDescriptorSet(const VkDevice& device,
	const VulkanContext::VulkanInstance& instance,
	const VulkanDescriptorSetLayout& layout,
	std::shared_ptr<VulkanDescriptorSetPool>& pool,
	const DescriptorBufferBindings& bufferInfos):
	m_VkDevice(device), m_VkInstance(instance), m_Layout(layout), m_Pool(pool), m_DescriptorSet(pool->AllocateDesciptorSet())
{

	if (!bufferInfos.Buffers.empty() || !bufferInfos.Images.empty())
		Update(bufferInfos.Buffers, bufferInfos.Images);
}

void shade::VulkanDescriptorSet::Update(const DescriptorBingingsMap<VkDescriptorBufferInfo>& bufferInfos, const DescriptorBingingsMap<VkDescriptorImageInfo>& imageInfos)
{
	std::vector<VkWriteDescriptorSet> writeDescriptorSets;

	// Iterate over all buffer bindings
	for (auto& [binding, set] : bufferInfos)
	{
		for (auto& [element, buffer] : set)
		{
			auto& layoutBinding = m_Layout.GetDescriptorSetLayoutBinding(binding);

			if (&layoutBinding != NULL)
			{
				VkWriteDescriptorSet writeDescriptorSet
				{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.pNext = VK_NULL_HANDLE,
					.dstSet = m_DescriptorSet,
					.dstBinding = std::uint32_t(binding),
					.dstArrayElement = std::uint32_t(element),
					.descriptorCount = 1,
					.descriptorType = layoutBinding.descriptorType,
					.pImageInfo = VK_NULL_HANDLE,
					.pBufferInfo = &buffer,
					.pTexelBufferView = VK_NULL_HANDLE
				};

				writeDescriptorSets.push_back(writeDescriptorSet);
			}
		}
	}
	// Iterate over all buffer bindings
	for (auto& [binding, set] : imageInfos)
	{
		for (auto& [element, buffer] : set)
		{
			auto& layoutBinding = m_Layout.GetDescriptorSetLayoutBinding(binding);

			if (&layoutBinding != NULL)
			{
				VkWriteDescriptorSet writeDescriptorSet
				{
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.pNext = VK_NULL_HANDLE,
					.dstSet = m_DescriptorSet,
					.dstBinding = std::uint32_t(binding),
					.dstArrayElement = std::uint32_t(element),
					.descriptorCount = 1,
					.descriptorType = layoutBinding.descriptorType,
					.pImageInfo = &buffer,
					.pBufferInfo = VK_NULL_HANDLE,
					.pTexelBufferView = VK_NULL_HANDLE
				};

				writeDescriptorSets.push_back(writeDescriptorSet);
			}
		}
	}

	vkUpdateDescriptorSets(m_VkDevice, std::uint32_t(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}