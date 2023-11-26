#pragma once
#include <shade/utils/Utils.h>
#include <shade/platforms/render/vulkan/VulkanContext.h>
#include <shade/platforms/render/vulkan/descriptors/VulkanDescriptorSetLayout.h>
#include <shade/platforms/render/vulkan/descriptors/VulkanDescriptorSetPool.h>

namespace shade
{
	template<typename T>
	using DescriptorBingingsMap = std::map<std::size_t, std::map<std::size_t, T>>;

	struct DescriptorBufferBindings
	{
		DescriptorBingingsMap<VkDescriptorBufferInfo> Buffers;
		DescriptorBingingsMap<VkDescriptorImageInfo>  Images;
	};

	class VulkanDescriptorSet
	{
	public:
		// Set index, exmp: layout(set = index )
		enum class Set : std::uint32_t
		{
			Global = 0,
			PerInstance = 1
		};
		/* Relative to vulkan specification */
		enum Type : std::uint32_t
		{
			Sampler = 0,
			CombinedImageSampler = 1,
			SampledImage = 2,
			StorageImage = 3,
			UniformTexelBuffer = 4,
			StorageTexelBuffer = 5,
			UniformBuffer = 6,
			StorageBuffer = 7,
			UniformBufferDynamic = 8,
			StorageBufferDymanic = 9,
			InputAttachment = 10
		};

		VulkanDescriptorSet(const VkDevice& device,
							const VulkanContext::VulkanInstance& instance,
							const VulkanDescriptorSetLayout& layout,
							std::shared_ptr<VulkanDescriptorSetPool>& pool,
							const DescriptorBufferBindings& bufferInfos);

		~VulkanDescriptorSet() = default;
		const VkDescriptorSet& GetDescriptorSet() const { return m_DescriptorSet; };
		
		void Update(const DescriptorBingingsMap<VkDescriptorBufferInfo>& bufferInfos, const DescriptorBingingsMap<VkDescriptorImageInfo>& imageInfos);
		void SetDepricated(bool is) { m_IsDeprecated = is; }
		bool IsDepricated() const { return m_IsDeprecated; }
	private:
		VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
		const VulkanDescriptorSetLayout& m_Layout;
		std::shared_ptr<VulkanDescriptorSetPool> m_Pool;

		VkDevice m_VkDevice = VK_NULL_HANDLE;
		VulkanContext::VulkanInstance m_VkInstance;
		bool m_IsDeprecated = false;
	};
}

namespace std
{
	template <>
	struct hash<shade::VulkanDescriptorSet>
	{
		std::size_t operator()(const shade::VulkanDescriptorSet& set) const
		{
			std::size_t result = 0;
			shade::HashCombine(result, set.GetDescriptorSet());
			return result;
		}
	};
}