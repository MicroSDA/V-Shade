#pragma once
#include <shade/platforms/render/vulkan/VKUtils.h>
#include <shade/platforms/render/vulkan/VulkanContext.h>
#include <shade/platforms/render/vulkan/descriptors/VulkanDescriptorSetLayout.h>
#include <shade/platforms/render/vulkan/descriptors/VulkanDescriptorSet.h>
#include <shade/platforms/render/vulkan/descriptors/VulkanDescriptorSetPool.h>
// Need to change place after 
namespace std
{
	template <>
	struct hash<VkDescriptorBufferInfo>
	{
		size_t operator()(const VkDescriptorBufferInfo& descriptor_buffer_info) const
		{
			size_t result = 0;

			shade::HashCombine(result, descriptor_buffer_info.buffer);
			shade::HashCombine(result, descriptor_buffer_info.range);
			shade::HashCombine(result, descriptor_buffer_info.offset);

			return result;
		}
	};
	template <>
	struct hash<VkDescriptorImageInfo>
	{
		std::size_t operator()(const VkDescriptorImageInfo& descriptor_image_info) const
		{
			std::size_t result = 0;

			shade::HashCombine(result, descriptor_image_info.imageView);
			shade::HashCombine(result, descriptor_image_info.imageLayout);
			shade::HashCombine(result, descriptor_image_info.sampler);

			return result;
		}
	};
}

namespace shade
{
	class VulkanDescriptorsManager
	{
	public:
		static void Initialize(VkDevice& device, const VulkanContext::VulkanInstance& instance, std::uint32_t framesCount);
		static std::shared_ptr<VulkanDescriptorSet> ReciveDescriptor(
			const VulkanDescriptorSetLayout& layout,
			const DescriptorBufferBindings& bufferInfos,
			std::uint32_t frameIndex);

		static void ShutDown();
		template<typename T, typename... Args>
		static std::shared_ptr<T> GetPool(ankerl::unordered_dense::map<std::size_t, std::weak_ptr<T>>& resources, Args&... args);
		template<typename T, typename... Args>
		static std::shared_ptr<T> GetDescriptor(ankerl::unordered_dense::map<std::size_t, std::shared_ptr<T>>& resources, Args&... args);

		static void ResetAllDescripotrs(std::uint32_t frameIndex);
		static void ResetDepricated(std::uint32_t frameIndex);
	private:
		// Frame index - > (Descriptor Layout) Hash - > DescriptorPool
		static inline std::vector<ankerl::unordered_dense::map<std::size_t, std::weak_ptr<VulkanDescriptorSetPool>>> m_sDescriptorPools;
		// Frame index - > (Descriptor Layout + Buffers + ImageBuffers) Hashes - > DescriptorSet
		static inline std::vector<ankerl::unordered_dense::map<std::size_t, std::shared_ptr<VulkanDescriptorSet>>>  m_sDescriptorSets;

		static VkDevice m_sVkDevice;
		static VulkanContext::VulkanInstance m_sVkInstance;
		static std::uint32_t m_sFramesCount;

	private:
		template<typename T>
		static void GenerateHash(std::size_t& seed, const T& value);
		template <typename T, typename... Args>
		static inline void GenerateHash(size_t& seed, const T& first_arg, const Args &... args);
	};
	template<typename T, typename... Args>
	inline std::shared_ptr<T> VulkanDescriptorsManager::GetPool(ankerl::unordered_dense::map<std::size_t, std::weak_ptr<T>>& resources, Args&... args)
	{
		std::size_t hash = 0U;
		GenerateHash(hash, args...);

		auto resource = resources.find(hash);
		if (resource != resources.end())
		{
			return resource->second.lock();
		}
		else
		{
			std::shared_ptr<T> newResource = std::make_shared<T>(m_sVkDevice, m_sVkInstance, args...);
			resources.emplace(hash, newResource);
			return newResource;
		}
	}
	template<typename T, typename... Args>
	inline std::shared_ptr<T> VulkanDescriptorsManager::GetDescriptor(ankerl::unordered_dense::map<std::size_t, std::shared_ptr<T>>& resources, Args&... args)
	{
		std::size_t hash = 0U;
		GenerateHash(hash, args...);

		auto resource = resources.find(hash);
		if (resource != resources.end())
		{
			resource->second->SetDepricated(false);
			return resource->second;
		}
		else
		{
			std::shared_ptr<T> newResource = std::make_shared<T>(m_sVkDevice, m_sVkInstance, args...);
			resources.emplace(hash, newResource);
			return newResource;
		}
	}
	template <typename T>
	inline void VulkanDescriptorsManager::GenerateHash(std::size_t& seed, const T& value)
	{
		HashCombine(seed, value);
	}
	template <>
	inline void VulkanDescriptorsManager::GenerateHash<DescriptorBufferBindings>(
		size_t& seed,
		const DescriptorBufferBindings& value)
	{
		for (auto& binding_set : value.Buffers)
		{
			HashCombine(seed, binding_set.first);

			for (auto& binding_element : binding_set.second)
			{
				HashCombine(seed, binding_element.first);
				HashCombine(seed, binding_element.second);
			}
		}

		for (auto& binding_set : value.Images)
		{
			HashCombine(seed, binding_set.first);

			for (auto& binding_element : binding_set.second)
			{
				HashCombine(seed, binding_element.first);
				HashCombine(seed, binding_element.second);
			}
		}
	}
	template <typename T, typename... Args>
	inline void VulkanDescriptorsManager::GenerateHash(std::size_t& seed, const T& first_arg, const Args &... args)
	{
		GenerateHash(seed, first_arg);

		GenerateHash(seed, args...);
	}
	
}