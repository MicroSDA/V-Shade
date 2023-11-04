#include "shade_pch.h"
#include "VulkanDescriptorSetPool.h"

shade::VulkanDescriptorSetPool::VulkanDescriptorSetPool(const VkDevice& device, const VulkanContext::VulkanInstance& instance, const VulkanDescriptorSetLayout& layout, std::uint32_t maxSets)
	:m_VkDevice(device), m_VkInstance(instance), m_DescriptorLayout(layout), m_MaxSets(maxSets)
{
	const auto& bindings = layout.GetDescriptorSetLayoutBindings();
	std::map<VkDescriptorType, std::uint32_t> descriptorTypes;

	// Count each type of descriptor set
	for (auto& binding : bindings)
	{
		descriptorTypes[binding.descriptorType] += binding.descriptorCount;
	}
	// Allocate pool sizes array
	m_DescriptorPoolSizes.resize(descriptorTypes.size());

	auto pollSizeIt = m_DescriptorPoolSizes.begin();

	// Fill pool size for each descriptor type count multiplied by the pool size
	for (auto& [type, count] : descriptorTypes)
	{
		pollSizeIt->type = type;

		pollSizeIt->descriptorCount = count * maxSets;

		++pollSizeIt;
	}
	
	m_MaxSets = maxSets;
}

shade::VulkanDescriptorSetPool::~VulkanDescriptorSetPool()
{
	for (auto& pool : m_DescriptorPools)
	{
		vkDestroyDescriptorPool(m_VkDevice, pool, m_VkInstance.AllocationCallbaks);
	}
}

VkDescriptorSet shade::VulkanDescriptorSetPool::AllocateDesciptorSet()
{
	m_PoolIndex = FindAvaliblePool(m_PoolIndex);
	
	VkDescriptorSetLayout descriptorSetLayout = m_DescriptorLayout.GetDescriptorSetLayout();

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.descriptorPool = m_DescriptorPools[m_PoolIndex],
		.descriptorSetCount = 1,
		.pSetLayouts = &descriptorSetLayout
	};

	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	// Allocate a new descriptor set from the current pool
	auto result = vkAllocateDescriptorSets(m_VkDevice, &descriptorSetAllocateInfo, &descriptorSet);

	if (result != VK_SUCCESS)
	{
		return VK_NULL_HANDLE;
	}

	VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Descriptor set :", (std::uint32_t)&descriptorSet), m_VkDevice, VK_OBJECT_TYPE_DESCRIPTOR_SET, descriptorSet);
	// So we have found some index and allocated pool for it, let's increment sets count of this pool
	++m_DescriptorPoolsSetsCount[m_PoolIndex];
	// Add mapping
	m_DescriptorsSetsPoolMapping.emplace(descriptorSet, m_PoolIndex);

	return descriptorSet;
}

std::uint32_t shade::VulkanDescriptorSetPool::FindAvaliblePool(std::uint32_t searchIndex)
{
	// Create a new pool
	if (m_DescriptorPools.size() <= searchIndex)
	{
		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = VK_NULL_HANDLE,
			.flags = 0,
			.maxSets = m_MaxSets,
			.poolSizeCount = std::uint32_t(m_DescriptorPoolSizes.size()),
			.pPoolSizes = m_DescriptorPoolSizes.data()
		};

		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

		// Create the Vulkan descriptor pool
		auto result = vkCreateDescriptorPool(m_VkDevice, &descriptorPoolCreateInfo, m_VkInstance.AllocationCallbaks, &descriptorPool);

		if (result != VK_SUCCESS)
			return 0;

		VKUtils::SetDebugObjectName(m_VkInstance.Instance, std::format("{0}:{1}", "Descriptor Pool :", (std::uint32_t)&descriptorPool), m_VkDevice, VK_OBJECT_TYPE_DESCRIPTOR_POOL, descriptorPool);
		// Store internally the Vulkan handle
		m_DescriptorPools.push_back(descriptorPool);

		m_DescriptorPoolsSetsCount.push_back(0);
		
		return searchIndex;
	}
	else if (m_DescriptorPoolsSetsCount[searchIndex] < m_MaxSets)
	{
		return searchIndex;
	}

	// Increment pool index
	return FindAvaliblePool(++searchIndex);
}
