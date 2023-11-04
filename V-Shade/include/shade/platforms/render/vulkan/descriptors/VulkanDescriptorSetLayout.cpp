#include "shade_pch.h"
#include "VulkanDescriptorSetLayout.h"

shade::VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(
	const VkDevice& device, 
	const VulkanContext::VulkanInstance& instance,
	const std::initializer_list<VkDescriptorSetLayoutBinding>& bindings) :
	m_VkDevice(device), m_VkInstance(instance), m_LayoutBindings(bindings)
{
	assert(m_LayoutBindings.size() && "Bindings count is 0 !");

	m_DescriptorSetLayoutCreateInfo = 
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = 0,
		.bindingCount = std::uint32_t(m_LayoutBindings.size()),
		.pBindings = m_LayoutBindings.data()
	};

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &m_DescriptorSetLayoutCreateInfo, instance.AllocationCallbaks, &m_DescriptorSetLayout), "Failed to create descriptor set layout!");
	VKUtils::SetDebugObjectName(instance.Instance, std::format("{0}: {1}", "Descriptor set layout", (std::uint32_t)&m_DescriptorSetLayout), device, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, m_DescriptorSetLayout);

	for (auto& binding : m_LayoutBindings)
		m_Bindings.emplace(binding.binding, &binding);
}

shade::VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(
	const VkDevice& device, 
	const VulkanContext::VulkanInstance& instance,
	std::vector<VkDescriptorSetLayoutBinding>& bindings):
	m_VkDevice(device), m_VkInstance(instance), m_LayoutBindings(bindings)
{
	assert(m_LayoutBindings.size() && "Bindings count is 0 !");

	m_DescriptorSetLayoutCreateInfo =
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = 0,
		.bindingCount = std::uint32_t(m_LayoutBindings.size()),
		.pBindings = m_LayoutBindings.data()
	};

	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &m_DescriptorSetLayoutCreateInfo, instance.AllocationCallbaks, &m_DescriptorSetLayout), "Failed to create descriptor set layout!");
	VKUtils::SetDebugObjectName(instance.Instance, std::format("{0}:{1}", "Descriptor set layout", (std::uint32_t)&m_DescriptorSetLayout), device, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, m_DescriptorSetLayout);

	for (auto& binding : m_LayoutBindings)
		m_Bindings.emplace(binding.binding, &binding);
}

shade::VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
{
	if(m_DescriptorSetLayout != VK_NULL_HANDLE)
		vkDestroyDescriptorSetLayout(m_VkDevice, m_DescriptorSetLayout, m_VkInstance.AllocationCallbaks);
}

const VkDescriptorSetLayout& shade::VulkanDescriptorSetLayout::GetDescriptorSetLayout() const
{
	return m_DescriptorSetLayout;
}

const std::vector<VkDescriptorSetLayoutBinding>& shade::VulkanDescriptorSetLayout::GetDescriptorSetLayoutBindings() const
{
	return m_LayoutBindings;
}

const VkDescriptorSetLayoutBinding& shade::VulkanDescriptorSetLayout::GetDescriptorSetLayoutBinding(std::size_t index) const
{
	if (m_Bindings.find(index) != m_Bindings.end())
		return *m_Bindings.at(index);
	else
		return *static_cast<VkDescriptorSetLayoutBinding*>(nullptr);
}

shade::VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& other) noexcept :
	m_VkDevice(other.m_VkDevice), m_VkInstance(other.m_VkInstance),
	m_DescriptorSetLayoutCreateInfo(other.m_DescriptorSetLayoutCreateInfo),
	m_Bindings(std::move(other.m_Bindings)),
	m_LayoutBindings(std::move(other.m_LayoutBindings)),
	m_DescriptorSetLayout(other.m_DescriptorSetLayout)
{
	other.m_DescriptorSetLayout = VK_NULL_HANDLE;
}
