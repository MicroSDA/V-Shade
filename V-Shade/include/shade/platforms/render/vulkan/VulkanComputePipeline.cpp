#include "shade_pch.h"
#include "VulkanComputePipeline.h"
#include <shade/platforms/render/vulkan/VulkanRenderAPI.h>
#include <shade/core/render/Renderer.h>

shade::VulkanComputePipeline::VulkanComputePipeline(const VkDevice& device, const VulkanContext::VulkanInstance& instance, const ComputePipeline::Specification& specification):
	m_VkDevice(device), m_VkInstance(instance)
{
	m_Specification = specification;
	Invalidate();
}

shade::VulkanComputePipeline::~VulkanComputePipeline()
{
	if (m_PipelineLayout != VK_NULL_HANDLE)
		vkDestroyPipelineLayout(m_VkDevice, m_PipelineLayout, m_VkInstance.AllocationCallbaks);
	if (m_Pipeline != VK_NULL_HANDLE)
		vkDestroyPipeline(m_VkDevice, m_Pipeline, m_VkInstance.AllocationCallbaks);
}

void shade::VulkanComputePipeline::Invalidate()
{
	std::vector<VkPushConstantRange> pushConstants;

	for (auto& [set, data] : m_Specification.Shader->As<VulkanShader>().GetReflectedData())
	{
		std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBinding;

		if (set != std::uint32_t(Pipeline::Set::Global))
		{
			// Storage Buffer
			for (auto& [name, buffer] : data.StorageBuffers)
			{
				VkDescriptorSetLayoutBinding layoutSetBinding =
				{
					.binding = buffer.Binding,
					.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
					.descriptorCount = 1,
					.stageFlags = static_cast<VkShaderStageFlags>(buffer.ShaderType),
					.pImmutableSamplers = VK_NULL_HANDLE
				};

				descriptorSetLayoutBinding.emplace_back(layoutSetBinding);
			}
			// Uniform Buffer
			for (auto& [name, buffer] : data.UniformBuffers)
			{
				VkDescriptorSetLayoutBinding layoutSetBinding =
				{
					.binding = buffer.Binding,
					.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.descriptorCount = 1,
					.stageFlags = static_cast<VkShaderStageFlags>(buffer.ShaderType),
					.pImmutableSamplers = VK_NULL_HANDLE
				};

				descriptorSetLayoutBinding.emplace_back(layoutSetBinding);
			}
			// Sampler 
			for (auto& [name, image] : data.ImageSamplers)
			{
				VkDescriptorSetLayoutBinding layoutSetBinding =
				{
					.binding = image.Binding,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount = 1,
					.stageFlags = static_cast<VkShaderStageFlags>(image.ShaderType),
					.pImmutableSamplers = VK_NULL_HANDLE
				};

				descriptorSetLayoutBinding.emplace_back(layoutSetBinding);
			}
			// Storage Sampler
			for (auto& [name, image] : data.ImageStorage)
			{
				VkDescriptorSetLayoutBinding layoutSetBinding =
				{
					.binding = image.Binding,
					.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
					.descriptorCount = 1,
					.stageFlags = static_cast<VkShaderStageFlags>(image.ShaderType),
					.pImmutableSamplers = VK_NULL_HANDLE
				};

				descriptorSetLayoutBinding.emplace_back(layoutSetBinding);
			}
			m_DescriptorsLayouts.emplace(static_cast<Pipeline::Set>(set), VulkanDescriptorSetLayout(m_VkDevice, m_VkInstance, descriptorSetLayoutBinding));
		}

		// Push constant is out of set
		for (auto& [name, constant] : data.PushConstants)
		{
			VkPushConstantRange range
			{
				.stageFlags = static_cast<VkShaderStageFlags>(constant.ShaderType),
				.offset = 0,
				.size = constant.Size
			};

			pushConstants.emplace_back(range);
		}
	}

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts(1, VulkanRenderAPI::GetGlobalDescriptorSetLayout()->GetDescriptorSetLayout());
	//std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	// There no compute stage in global push constant at this moment !
	//std::vector<VkPushConstantRange> pushConstanRanges = VulkanRenderAPI::GlobalPushConstantRanges();
	
	// Linking compute stage: Only one push_constant block is allowed per stage, so it doesnt work 
	for (std::size_t i = 1; i < pushConstants.size(); i++)
		pushConstants[i].offset = pushConstants[i - 1].size;

	for (auto& [set, layout] : m_DescriptorsLayouts)
		descriptorSetLayouts.emplace_back(layout.GetDescriptorSetLayout());

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = 0,
		.setLayoutCount = static_cast<std::uint32_t>(descriptorSetLayouts.size()),
		.pSetLayouts = descriptorSetLayouts.data(),
		.pushConstantRangeCount = static_cast<std::uint32_t>(pushConstants.size()),
		.pPushConstantRanges = pushConstants.data()
	};

	VK_CHECK_RESULT(vkCreatePipelineLayout(m_VkDevice, &pipelineLayoutCreateInfo, m_VkInstance.AllocationCallbaks, &m_PipelineLayout), "Failed to create compute pipeline layout!");
	
	VkComputePipelineCreateInfo computePipelineCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = 0,
		// Should be one stage, compute, so 0 index 
		.stage = m_Specification.Shader->As<VulkanShader>().GetPipelineShaderStageCreateInfo()[0],
		.layout = m_PipelineLayout,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = 0
	};

	VK_CHECK_RESULT(vkCreateComputePipelines(m_VkDevice, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, m_VkInstance.AllocationCallbaks, &m_Pipeline), "Failed to create compute pipeline!");
}

void shade::VulkanComputePipeline::UpdateResources(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex)
{
	BindDescriptorsSets(commandBuffer, frameIndex);
}

void shade::VulkanComputePipeline::BindDescriptorsSets(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex)
{
	vkCmdBindDescriptorSets(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex), VK_PIPELINE_BIND_POINT_COMPUTE,
		m_PipelineLayout, std::uint32_t(Pipeline::Set::Global), 1, &VulkanRenderAPI::GetGlobalDescriptorSet(frameIndex)->GetDescriptorSet(), 0, VK_NULL_HANDLE);

	for (auto& [set, layout] : m_DescriptorsLayouts)
	{
		if (set != Pipeline::Set::Global)
		{
			auto resourcesAtSetIndex = m_Resources.find(set);
			if (resourcesAtSetIndex != m_Resources.end())
			{
				auto& resources = resourcesAtSetIndex->second;
				vkCmdBindDescriptorSets(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex), VK_PIPELINE_BIND_POINT_COMPUTE,
					m_PipelineLayout, std::uint32_t(set), 1, &VulkanDescriptorsManager::ReciveDescriptor(layout, resources, frameIndex)->GetDescriptorSet(), 0, VK_NULL_HANDLE);
			}
			else
			{
				for (auto& binding : layout.GetDescriptorSetLayoutBindings())
				{
					std::string type;

					switch (VulkanDescriptorSet::Type(binding.descriptorType))
					{
					case VulkanDescriptorSet::Type::StorageBuffer: type = "StorageBuffer"; break;
					case VulkanDescriptorSet::Type::UniformBuffer: type = "UniformBuffer"; break;
					case VulkanDescriptorSet::Type::CombinedImageSampler: type = "CombinedImageSampler"; break;
					default:
						break;
					}

					SHADE_CORE_WARNING("Descriptor set layout requires resource type = '{0}', set = '{1}', bindidng = '{2}' but it has not been providet!", type, set, binding.binding);
				}
			}
		}
	}
}

void shade::VulkanComputePipeline::Begin(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex)
{
	vkCmdBindPipeline(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex), VK_PIPELINE_BIND_POINT_COMPUTE, m_Pipeline);
}

void shade::VulkanComputePipeline::Dispatch(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t groupCountX, std::uint32_t groupCountY, std::uint32_t groupCountZ, std::uint32_t frameIndex)
{
	//vkCmdBindPipeline(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex), VK_PIPELINE_BIND_POINT_COMPUTE, m_Pipeline);
	vkCmdDispatch(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex), groupCountX, groupCountY, groupCountZ);
}

void shade::VulkanComputePipeline::SetTexture(Asset<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex)
{
	if (texture)
		m_Resources[set].Images[binding][0] = texture->As<VulkanTexture2D>().GetDescriptorImageInfo();
	else
		m_Resources[set].Images[binding][0] = Renderer::GetDefaultDiffuseTexture()->As<VulkanTexture2D>().GetDescriptorImageInfo();
}

void shade::VulkanComputePipeline::SetTexture(SharedPointer<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex)
{
	if (texture)
		m_Resources[set].Images[binding][0] = texture->As<VulkanTexture2D>().GetDescriptorImageInfo();
	else
		m_Resources[set].Images[binding][0] = Renderer::GetDefaultDiffuseTexture()->As<VulkanTexture2D>().GetDescriptorImageInfo();
}

void shade::VulkanComputePipeline::SetTexturePerMipLevel(Asset<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t mipLevel)
{
	if (texture)
		m_Resources[set].Images[binding][0] = texture->As<VulkanTexture2D>().GetDescriptorImageInfoMip(mipLevel);
	else
		m_Resources[set].Images[binding][0] = Renderer::GetDefaultDiffuseTexture()->As<VulkanTexture2D>().GetDescriptorImageInfoMip(mipLevel);
}

void shade::VulkanComputePipeline::SetTexturePerMipLevel(SharedPointer<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t mipLevel)
{
	if (texture)
		m_Resources[set].Images[binding][0] = texture->As<VulkanTexture2D>().GetDescriptorImageInfoMip(mipLevel);
	else
		m_Resources[set].Images[binding][0] = Renderer::GetDefaultDiffuseTexture()->As<VulkanTexture2D>().GetDescriptorImageInfoMip(mipLevel);
}

void shade::VulkanComputePipeline::SetTexturePerLayer(Asset<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t layer)
{
	if (texture)
		m_Resources[set].Images[binding][0] = texture->As<VulkanTexture2D>().GetDescriptorImageInfoLayer(layer);
	else
		m_Resources[set].Images[binding][0] = Renderer::GetDefaultDiffuseTexture()->As<VulkanTexture2D>().GetDescriptorImageInfoLayer(layer);
}

void shade::VulkanComputePipeline::SetTexturePerLayer(SharedPointer<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t layer)
{
	if (texture)
		m_Resources[set].Images[binding][0] = texture->As<VulkanTexture2D>().GetDescriptorImageInfoLayer(layer);
	else
		m_Resources[set].Images[binding][0] = Renderer::GetDefaultDiffuseTexture()->As<VulkanTexture2D>().GetDescriptorImageInfoLayer(layer);
}

void shade::VulkanComputePipeline::SetResource(SharedPointer<StorageBuffer> storageBuffer, Pipeline::Set set, std::uint32_t frameIndex, std::uint32_t offset)
{
	if (storageBuffer->As<VulkanStorageBuffer>().GetDescriptorBufferInfos().size() >= frameIndex)
	{
		m_Resources[set].Buffers[storageBuffer->GetBinding()][0] = storageBuffer->As<VulkanStorageBuffer>().GetDescriptorBufferInfo(frameIndex);
		m_Resources[set].Buffers[storageBuffer->GetBinding()][0].offset += offset;
		m_Resources[set].Buffers[storageBuffer->GetBinding()][0].range -= offset;
	}
	else
	{
		SHADE_CORE_WARNING("Trying to set storage buffer at frame index = {0}, but blocks count = {1} !", frameIndex, storageBuffer->As<VulkanStorageBuffer>().GetDescriptorBufferInfos().size());
	}
}

void shade::VulkanComputePipeline::SetResource(SharedPointer<UniformBuffer> storageBuffer, Pipeline::Set set, std::uint32_t frameIndex, std::uint32_t offset)
{
	if (storageBuffer->As<VulkanUniformBuffer>().GetDescriptorBufferInfos().size() >= frameIndex)
	{
		m_Resources[set].Buffers[storageBuffer->GetBinding()][0] = storageBuffer->As<VulkanUniformBuffer>().GetDescriptorBufferInfo(frameIndex);
		m_Resources[set].Buffers[storageBuffer->GetBinding()][0].offset += offset;
		m_Resources[set].Buffers[storageBuffer->GetBinding()][0].range -= offset;
	}
	else
	{
		SHADE_CORE_WARNING("Trying to set uniformbuffer at frame index = {0}, but blocks count = {1} !", frameIndex, storageBuffer->As<VulkanUniformBuffer>().GetDescriptorBufferInfos().size());
	}
}

void shade::VulkanComputePipeline::SetUniform(SharedPointer<RenderCommandBuffer>& commandBuffer, std::size_t size, const void* data, std::uint32_t frameIndex, std::uint32_t offset)
{
	vkCmdPushConstants(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex),
		m_PipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, offset, size, data);
}

void shade::VulkanComputePipeline::SetBarrier(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<StorageBuffer> buffer, Stage srcStage, Stage dstStage, Access srcAccess, Access dstAccces, std::uint32_t frameIndex)
{
	VkBufferMemoryBarrier bufferMemoryBarier
	{
		.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		.pNext = VK_NULL_HANDLE,
		.srcAccessMask = static_cast<VkAccessFlags>(srcAccess),
		.dstAccessMask = static_cast<VkAccessFlags>(dstAccces),
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.buffer = buffer->As<VulkanStorageBuffer>().GetDescriptorBufferInfo(frameIndex).buffer,
		.offset = buffer->As<VulkanStorageBuffer>().GetDescriptorBufferInfo(frameIndex).offset,
		 //TIP: Im not user that size is range
		.size = buffer->As<VulkanStorageBuffer>().GetDescriptorBufferInfo(frameIndex).range,
	};
	vkCmdPipelineBarrier(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex), // commandBuffer
		static_cast<VkPipelineStageFlags>(srcStage), // srcStageMask
		static_cast<VkPipelineStageFlags>(dstStage), // dstStageMask
		0, // dependencyFlags
		0, // memoryBarrierCount
		VK_NULL_HANDLE, // pMemoryBarriers
		1, // bufferMemoryBarrierCount
		&bufferMemoryBarier, // pBufferMemoryBarriers
		0, // imageMemoryBarrierCount
		VK_NULL_HANDLE); // pImageMemoryBarriers
}

void shade::VulkanComputePipeline::SetBarrier(SharedPointer<RenderCommandBuffer>& commandBuffer, Asset<Texture2D> texture, Stage srcStage, Stage dstStage, Access srcAccess, Access dstAccces, std::uint32_t frameIndex, std::uint32_t mip)
{
	VkImageMemoryBarrier imageMemoryBarier
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = VK_NULL_HANDLE,
		.srcAccessMask = static_cast<VkAccessFlags>(srcAccess),
		.dstAccessMask = static_cast<VkAccessFlags>(dstAccces),
		.oldLayout = texture->GetImage()->As<VulkanImage2D>().GetImageLayout(),
		.newLayout = VK_IMAGE_LAYOUT_GENERAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = texture->GetImage()->As<VulkanImage2D>().GetImage(),
		.subresourceRange =
		{
				texture->As<VulkanTexture2D>().GetImage()->As<VulkanImage2D>().GetAspectMask(), // aspectMask
				(mip) ? mip : 0, // baseMipLevel
				(mip) ? 1 : texture->GetImage()->GetSpecification().MipLevels, // levelCount
				0, // baseArrayLayer
				texture->GetImage()->GetSpecification().Layers // layerCount
		}
	}; ;
	vkCmdPipelineBarrier(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex), 
		static_cast<VkPipelineStageFlags>(srcStage),
		static_cast<VkPipelineStageFlags>(dstStage),
		0,
		0,
		VK_NULL_HANDLE, 
		0,
		VK_NULL_HANDLE, 
		1,
		&imageMemoryBarier);
}
void shade::VulkanComputePipeline::SetBarrier(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<Texture2D> texture, Stage srcStage, Stage dstStage, Access srcAccess, Access dstAccces, std::uint32_t frameIndex, std::uint32_t mip)
{
	VkImageMemoryBarrier imageMemoryBarier
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = VK_NULL_HANDLE,
		.srcAccessMask = static_cast<VkAccessFlags>(srcAccess),
		.dstAccessMask = static_cast<VkAccessFlags>(dstAccces),
		.oldLayout = VK_IMAGE_LAYOUT_GENERAL,
		.newLayout = VK_IMAGE_LAYOUT_GENERAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = texture->GetImage()->As<VulkanImage2D>().GetImage(),
		.subresourceRange =
		{
				texture->GetImage()->As<VulkanImage2D>().GetAspectMask(), // aspectMask
				(mip) ? mip : 0, // baseMipLevel
				(mip) ? 1 : texture->GetImage()->GetSpecification().MipLevels, // levelCount
				0, // baseArrayLayer
				texture->GetImage()->GetSpecification().Layers // layerCount
		}
	};
	vkCmdPipelineBarrier(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex),
		static_cast<VkPipelineStageFlags>(srcStage),
		static_cast<VkPipelineStageFlags>(dstStage),
		0,
		0,
		VK_NULL_HANDLE,
		0,
		VK_NULL_HANDLE,
		1,
		&imageMemoryBarier);
}

void shade::VulkanComputePipeline::SetBarrier(SharedPointer<RenderCommandBuffer>& commandBuffer, Stage srcStage, Stage dstStage, Access srcAccess, Access dstAccces, std::uint32_t frameIndex)
{
	VkMemoryBarrier memoryBarrier
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
		.pNext = VK_NULL_HANDLE,
		.srcAccessMask = static_cast<VkAccessFlags>(srcAccess),
		.dstAccessMask = static_cast<VkAccessFlags>(dstAccces),
	};
	vkCmdPipelineBarrier(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex),
		static_cast<VkPipelineStageFlags>(srcStage),
		static_cast<VkPipelineStageFlags>(dstStage),
		0,
		1,
		&memoryBarrier,
		0,
		VK_NULL_HANDLE,
		0,
		VK_NULL_HANDLE);

}

void shade::VulkanComputePipeline::Recompile()
{
	m_Specification.Shader = Shader::Create(m_Specification.Shader->GetFilePath());
	Invalidate();
}

