#pragma once
#include <vulkan/vulkan.h>
#include <shade/utils/Utils.h>
#include <shade/platforms/render/vulkan/VulkanContext.h>

namespace shade
{
	class VulkanDescriptorSetLayout
	{
	public:
		VulkanDescriptorSetLayout(const VkDevice& device,
								  const VulkanContext::VulkanInstance& instance,
								  const std::initializer_list<VkDescriptorSetLayoutBinding>& bindings);
		VulkanDescriptorSetLayout(const VkDevice& device,
			const VulkanContext::VulkanInstance& instance,
			std::vector<VkDescriptorSetLayoutBinding>& bindings);

		~VulkanDescriptorSetLayout();
		const VkDescriptorSetLayout& GetDescriptorSetLayout() const;
		const std::vector<VkDescriptorSetLayoutBinding>& GetDescriptorSetLayoutBindings() const;
		// Returns ref to VkDescriptorSetLayoutBinding by binding index, otherwise will return NULL.
		const VkDescriptorSetLayoutBinding& GetDescriptorSetLayoutBinding(std::size_t index) const;

		VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& other) noexcept;

		VulkanDescriptorSetLayout & operator=(VulkanDescriptorSetLayout && other) noexcept = default;
		VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout& other) noexcept = delete;
		VulkanDescriptorSetLayout& operator=(const VulkanDescriptorSetLayout& other) noexcept = delete;
	private:
		VkDescriptorSetLayoutCreateInfo m_DescriptorSetLayoutCreateInfo;
		VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;

		std::vector<VkDescriptorSetLayoutBinding> m_LayoutBindings;
		std::unordered_map<std::size_t, VkDescriptorSetLayoutBinding*> m_Bindings;
	private:
		VkDevice m_VkDevice = VK_NULL_HANDLE;
		VulkanContext::VulkanInstance m_VkInstance;
	};
}

namespace std
{
	template <>
	struct hash<shade::VulkanDescriptorSetLayout>
	{
		std::size_t operator()(const shade::VulkanDescriptorSetLayout& layout) const 
		{
			std::size_t result = 0;

			shade::HashCombine(result, layout.GetDescriptorSetLayout());

			return result;
		}
	};
}