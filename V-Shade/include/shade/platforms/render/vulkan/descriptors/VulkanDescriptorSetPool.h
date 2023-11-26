#pragma once
#include <shade/utils/Utils.h>
#include <vulkan/vulkan.h>
#include <shade/platforms/render/vulkan/VulkanContext.h>
#include <shade/platforms/render/vulkan/descriptors/VulkanDescriptorSetLayout.h>

namespace shade
{
	class VulkanDescriptorSetPool
	{
	public:
		static const std::uint32_t MAX_SETS_PER_POOL = 10;
		VulkanDescriptorSetPool(const VkDevice& device, const VulkanContext::VulkanInstance& instance, const VulkanDescriptorSetLayout& layout, std::uint32_t maxSets = MAX_SETS_PER_POOL);
		~VulkanDescriptorSetPool();
		const std::vector<VkDescriptorPool>& GetDescriptorPool() const { return m_DescriptorPools; }
		const VulkanDescriptorSetLayout& GetDescriptorSetLayout() const { return m_DescriptorLayout; }
		VkDescriptorSet AllocateDesciptorSet();
	private:
		const VulkanDescriptorSetLayout& m_DescriptorLayout;
		std::vector<VkDescriptorPool> m_DescriptorPools;
		std::vector<std::uint32_t> m_DescriptorPoolsSetsCount;
		// Descriptor addr - > pool index
		std::unordered_map<VkDescriptorSet, std::uint32_t> m_DescriptorsSetsPoolMapping;

		std::vector<VkDescriptorPoolSize> m_DescriptorPoolSizes;

		std::uint32_t m_MaxSets = 0, m_PoolIndex = 0;
		VkDevice m_VkDevice = VK_NULL_HANDLE;
		VulkanContext::VulkanInstance m_VkInstance;

	private:
		std::uint32_t FindAvaliblePool(std::uint32_t searchIndex);
		
	};
}

namespace std
{
	template <>
	struct hash<shade::VulkanDescriptorSetPool>
	{
		std::size_t operator()(const shade::VulkanDescriptorSetPool& set) const
		{
			std::size_t result = 0;

			shade::HashCombine(result, set.GetDescriptorSetLayout());

			return result;
		}
	};
}