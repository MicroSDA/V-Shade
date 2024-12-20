#include "shade_pch.h"
#include "VulkanPipeline.h"
#include <shade/core/render/Renderer.h>
#include <shade/core/application/Application.h>
#include <shade/platforms/render/vulkan/VulkanRenderAPI.h>
#include <shade/platforms/render/vulkan/descriptors/VulkanDescriptor.h>
#include <glm/glm/gtx/hash.hpp>

shade::VulkanPipeline::VulkanPipeline(const Pipeline::Specification& specification)
{
	m_Specification = specification;
	Invalidate();
}

void shade::VulkanPipeline::Invalidate()
{
	// TODO: Refactor this part
	auto& device		= VulkanContext::GetLogicalDevice()->GetDevice();
	auto& instance		= VulkanContext::GetInstance();
	auto& shader =		 m_Specification.Shader->As<VulkanShader>();

	if (&shader) // Check if the shader is valid
	{
		std::vector<VkPushConstantRange> pushConstants;

		for (auto& [set, data] : shader.GetReflectedData())
		{
			std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBinding;
			
			if (set != std::uint32_t(Pipeline::Set::Global))
			{
				//------------------------------------------------------------------------
			    // Descriptor Set Layout Bindings: Storage Buffers
			    //------------------------------------------------------------------------
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
				//------------------------------------------------------------------------
				// Descriptor Set Layout Bindings: Uniform Buffers
				//------------------------------------------------------------------------
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
				//------------------------------------------------------------------------
				// Descriptor Set Layout Bindings: Image Samplers
				//------------------------------------------------------------------------
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
				
				m_DescriptorsLayouts.emplace(static_cast<Pipeline::Set>(set), VulkanDescriptorSetLayout(device, instance, descriptorSetLayoutBinding));
			}
			//------------------------------------------------------------------------
			// Push Constants
			//------------------------------------------------------------------------
			for (auto& [name, constant] : data.PushConstants)
			{
				VkPushConstantRange range
				{
					.stageFlags = constant.ShaderType,
					.offset = 0,
					.size = constant.Size
				};

				pushConstants.emplace_back(range);
			}
		}

		//------------------------------------------------------------------------
		// Pipeline Layout Creation
		//------------------------------------------------------------------------

		/* Alongside of all the State structs, we will need a VkPipelineLayout object for our pipeline.
		Unlike the other state structs, this one is an actual full Vulkan object, and needs to be created separately from the pipeline.
		Pipeline layouts contain the information about shader inputs of a given pipeline.
		It�s here where you would configure your push-constants and descriptor sets. */

		std::vector<VkDescriptorSetLayout>	descriptorSetLayouts(1, VulkanRenderAPI::GetGlobalDescriptorSetLayout()->GetDescriptorSetLayout());
		std::vector<VkPushConstantRange>	pushConstanRanges = VulkanRenderAPI::GlobalPushConstantRanges();

		std::map<VkShaderStageFlagBits, uint32_t> stagesOffsets;  // Map to hold stage offsets for push constants
		 
		for (auto& pushConstant : pushConstants) {
			VkShaderStageFlags stageFlags = pushConstant.stageFlags;

			// Iterate through all possible shader stages
			for (VkShaderStageFlagBits stageBit = VK_SHADER_STAGE_VERTEX_BIT;
				stageBit <= VK_SHADER_STAGE_COMPUTE_BIT;
				stageBit = static_cast<VkShaderStageFlagBits>(stageBit << 1)) 
			{
				if (stageFlags & stageBit) 
				{
					// Find the offset for the current stage in the map
					auto stage = stagesOffsets.find(stageBit);

					// If the stage is not found, add it to the map with an initial offset of 0
					if (stage == stagesOffsets.end()) 
					{
						stagesOffsets[stageBit] = 0;
					}
					else 
					{
						// Increment the offset for the current stage
						stage->second += pushConstant.size;
					}

					// Update the offset in the pushConstant structure
					pushConstant.offset = stagesOffsets[stageBit];
				}
			}
		}
		/*for (std::size_t i = 1; i < pushConstants.size(); i++)
			pushConstants[i].offset = pushConstants[i - 1].size;*/

		for (auto& [set, layout] : m_DescriptorsLayouts)
			descriptorSetLayouts.emplace_back(layout.GetDescriptorSetLayout());

		//------------------------------------------------------------------------
	    // Pipeline Layout Create Info
	    //------------------------------------------------------------------------
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

		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, VulkanContext::GetInstance().AllocationCallbaks, &m_PipelineLayout), "Failed to create pipeline layout!");

		//------------------------------------------------------------------------
	    // Input Assembly State
	    //------------------------------------------------------------------------

		/*  Contains the configuration for what kind of topology will be drawn.
			This is where you set it to draw triangles, lines, points, or others like triangle-list. */
		VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.pNext = VK_NULL_HANDLE,
			.flags = 0,
			.topology = static_cast<VkPrimitiveTopology>(m_Specification.Topology),
			.primitiveRestartEnable = VK_FALSE
		};

		//------------------------------------------------------------------------
		// Rasterization State
		//------------------------------------------------------------------------

		/*  Configuration for the fixed-function rasterization.
			In here is where we enable or disable backface culling, and set line width or wireframe drawing. */
		VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.pNext = VK_NULL_HANDLE,
			.flags = 0,
			.depthClampEnable = m_Specification.DepthClamp,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = static_cast<VkPolygonMode>(m_Specification.PolygonMode),
			.cullMode = (m_Specification.BackFalceCull) ? static_cast<VkCullModeFlags>(VK_CULL_MODE_BACK_BIT) : static_cast<VkCullModeFlags>(VK_CULL_MODE_NONE),
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_TRUE,

			.depthBiasConstantFactor = m_Specification.DepsBiasConstantFactor,
			.depthBiasClamp = m_Specification.DepthBiasClamp,
			.depthBiasSlopeFactor = m_Specification.DepthBiasSlopeFactor,

			.lineWidth = m_Specification.LineWidth,
		};
		
		//------------------------------------------------------------------------
		// Dynamic State Configuration
		//------------------------------------------------------------------------

		std::vector<VkDynamicState> dynamicStates
		{
		   VK_DYNAMIC_STATE_VIEWPORT,
		   //VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT,

		   VK_DYNAMIC_STATE_SCISSOR,
		   //VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT
		};

		VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.pNext = VK_NULL_HANDLE,
			.flags = 0,
			.viewportCount = 0,
			.pViewports = VK_NULL_HANDLE,
			.scissorCount = 0,
			.pScissors = VK_NULL_HANDLE
		};

		for (auto& state : dynamicStates)
		{
			if (state == VK_DYNAMIC_STATE_VIEWPORT)
				pipelineViewportStateCreateInfo.viewportCount++;
			if (state == VK_DYNAMIC_STATE_SCISSOR)
				pipelineViewportStateCreateInfo.scissorCount++;
			// TODO:
			/*if (instance->m_Specification.Topology == PrimitiveTopology::Lines || instance->m_Specification.Topology == PrimitiveTopology::LineStrip || instance->m_Specification.Wireframe)
				dynamicStateEnables.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);*/
		}

		//------------------------------------------------------------------------
	    // Dynamic State Create Info
	    //------------------------------------------------------------------------
		VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.pNext = VK_NULL_HANDLE,
			.flags = 0,
			.dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size()),
			.pDynamicStates = dynamicStates.data()
		};

		//------------------------------------------------------------------------
	    // Depth Stencil State
	    //------------------------------------------------------------------------
		// Actually we need check evereting from shader that we need to create bcs we can no create stensil or sometning.!
		VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.pNext = VK_NULL_HANDLE,
			.flags = 0,
			.depthTestEnable		= m_Specification.DepthTestEnabled,
			.depthWriteEnable		= m_Specification.DepthTestEnabled,
			.depthCompareOp			= static_cast<VkCompareOp>(m_Specification.DepthTest),
			.depthBoundsTestEnable	= VK_FALSE,
			.stencilTestEnable		= VK_FALSE,
			.front =
			{
				.failOp = VK_STENCIL_OP_KEEP, 
				.passOp = VK_STENCIL_OP_KEEP,
				.compareOp = VK_COMPARE_OP_ALWAYS
			},
			.back =  
			{
				.failOp = VK_STENCIL_OP_KEEP,
				.passOp = VK_STENCIL_OP_KEEP,
				.compareOp = VK_COMPARE_OP_NEVER
			},
			.minDepthBounds =  0.f,
			.maxDepthBounds =  1.f
		};

		//------------------------------------------------------------------------
		// Multisample State
		//------------------------------------------------------------------------
		/*  This allows us to configure MSAA for this pipeline. We are not going to use MSAA at this time,
			so we are going to default it to 1 sample and MSAA disabled.
			If you wanted to enable MSAA, you would need to set rasterizationSamples to more than 1,
			and enable sampleShading. Keep in mind that for MSAA to work, your renderpass also has to support it,
			which complicates things significantly. */

		VkPipelineMultisampleStateCreateInfo  pipelineMultisampleStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.pNext = VK_NULL_HANDLE,
			.flags = 0,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE,
			.minSampleShading = .2f,
			
		};
		
		/*	A vertex binding describes at which rate to load data from memory throughout the vertices.
			It specifies the number of bytes between data entries and whether to move to the next data entry after each vertex or after each instance. */
		std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions(m_Specification.VertexLayout.GetElementLayouts().size());
			for (std::uint32_t i = 0; i < vertexInputBindingDescriptions.size(); ++i)
		{
			vertexInputBindingDescriptions[i] =
			{
				.binding = i,
				.stride = m_Specification.VertexLayout.GetStride(i),
				.inputRate = static_cast<VkVertexInputRate>(m_Specification.VertexLayout.GetElementLayouts()[i].Usage)
			};
		}
		/* The structure that describes how to handle vertex input. */
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescription(m_Specification.VertexLayout.GetCount());

		std::uint32_t elementIndex = 0;
		for (std::uint32_t i = 0; i < m_Specification.VertexLayout.GetElementLayouts().size(); ++i)
		{
			for (std::uint32_t j = 0; j < m_Specification.VertexLayout.GetElementLayouts()[i].Elements.size(); ++j)
			{
				vertexInputAttributeDescription[elementIndex] =
				{
					.location	= elementIndex,
					.binding	= i,
					.format		= VulkanShader::GetShaderDataToVulkanFormat(m_Specification.VertexLayout.GetElementLayouts()[i].Elements[j].Type),
					.offset		= m_Specification.VertexLayout.GetElementLayouts()[i].Elements[j].Offset
				};

				++elementIndex;
			}
		}
		
		/* Contains the information for vertex buffers and vertex formats.
		   This is equivalent to the VAO configuration on opengl.*/
		VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.pNext = VK_NULL_HANDLE,
			.flags = 0,
			.vertexBindingDescriptionCount = static_cast<std::uint32_t>(vertexInputBindingDescriptions.size()),
			.pVertexBindingDescriptions = vertexInputBindingDescriptions.data(),
			.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(vertexInputAttributeDescription.size()),
			.pVertexAttributeDescriptions = vertexInputAttributeDescription.data()
		};

		// For dynamic render 
		SharedPointer<FrameBuffer> frameBuffer;

		// If frame buffer exist we can take it from specification
		if (m_Specification.FrameBuffer)
			frameBuffer = m_Specification.FrameBuffer;
		else
		{
			// Else, we are using swapchain as render target !
			// As far as we know that our swap chain has the same image formats we can take the image format from any frame in flight 
			frameBuffer = Application::GetWindow()->GetSwapChain()->GetFrameBuffers()[0];
		}

		std::vector<VkFormat> colorsFormats;
		VkFormat depthFormat = VK_FORMAT_UNDEFINED;

		for (auto& format : frameBuffer->GetSpecification().Attachments.TextureAttachments)
		{
			if (VKUtils::IsDepthFormat(format.Format) || VKUtils::IsDepthStencilFormat(format.Format))
				depthFormat = VKUtils::ToVulkanImageFormat(format.Format);
			else
				colorsFormats.emplace_back(VKUtils::ToVulkanImageFormat(format.Format));
		}

		m_PipelineRenderingCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.pNext = VK_NULL_HANDLE,
			.viewMask = 0,
			.colorAttachmentCount = static_cast<uint32_t>(colorsFormats.size()),
			.pColorAttachmentFormats = colorsFormats.data(),
			.depthAttachmentFormat = depthFormat,
			.stencilAttachmentFormat = VK_FORMAT_UNDEFINED
		};

		//------------------------------------------------------------------------
	    // Color Blend State
	    //------------------------------------------------------------------------
		std::vector<VkPipelineColorBlendAttachmentState> pipelineColorBlendAttachmentStates(colorsFormats.size());
		
		for (auto& pipelineColorBlendAttachmentState : pipelineColorBlendAttachmentStates)
		{
			pipelineColorBlendAttachmentState =
			{
				.blendEnable = VK_FALSE,
				.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
				.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
				.colorBlendOp = VK_BLEND_OP_ADD,
				.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
				.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
				.alphaBlendOp = VK_BLEND_OP_ADD,
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
			};
		}
		VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.pNext = VK_NULL_HANDLE,
			.flags = 0,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = static_cast<std::uint32_t>(pipelineColorBlendAttachmentStates.size()),
			.pAttachments = pipelineColorBlendAttachmentStates.data(),
			.blendConstants = { 0.f, 0.f, 0.f, 0.f }
		};
		//------------------------------------------------------------------------
		// Pipeline Create Info
		//------------------------------------------------------------------------
		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			// Dynamic rendering 
			.pNext = &m_PipelineRenderingCreateInfo,
			.flags = 0,
			.stageCount = static_cast<std::uint32_t>(shader.GetPipelineShaderStageCreateInfo().size()),
			.pStages = shader.GetPipelineShaderStageCreateInfo().data(),
			.pVertexInputState = &pipelineVertexInputStateCreateInfo,
			.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo,
			.pTessellationState = VK_NULL_HANDLE,
			.pViewportState = &pipelineViewportStateCreateInfo,
			.pRasterizationState = &pipelineRasterizationStateCreateInfo,

			.pMultisampleState = &pipelineMultisampleStateCreateInfo,

			.pDepthStencilState = &pipelineDepthStencilStateCreateInfo,
			.pColorBlendState = &pipelineColorBlendStateCreateInfo,
			.pDynamicState = &pipelineDynamicStateCreateInfo,
			.layout = m_PipelineLayout,
			// Since we are using dynamic render we dont need render pass otherwise get it form frame buffer
			.renderPass = VK_NULL_HANDLE,
			// Ignored 
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex = 0
		};

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, instance.AllocationCallbaks, &m_Pipeline), "Failed to create graphics pipeline!");
		VKUtils::SetDebugObjectName(instance.Instance, m_Specification.Name, device, VK_OBJECT_TYPE_PIPELINE, m_Pipeline);
	}
}

shade::VulkanPipeline::~VulkanPipeline()
{
	auto& device = VulkanContext::GetLogicalDevice()->GetDevice();
	auto& instance = VulkanContext::GetInstance();

	if (m_PipelineLayout != VK_NULL_HANDLE)
		vkDestroyPipelineLayout(device, m_PipelineLayout, instance.AllocationCallbaks);
	if (m_Pipeline != VK_NULL_HANDLE)
		vkDestroyPipeline(device, m_Pipeline, instance.AllocationCallbaks);
}

VkPipeline shade::VulkanPipeline::GetPipeline()
{
	return m_Pipeline;
}

VkPipelineLayout shade::VulkanPipeline::GetPipelineLayout()
{
	return m_PipelineLayout;
}

void shade::VulkanPipeline::Bind(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex)
{
	vkCmdBindPipeline(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
	// Move it some how from there or i dont know 
	//BindDescriptorsSets(commandBuffer, frameIndex);
}

void shade::VulkanPipeline::BindDescriptorsSets(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex)
{
	vkCmdBindDescriptorSets(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex), VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_PipelineLayout, std::uint32_t(Pipeline::Set::Global), 1, &VulkanRenderAPI::GetGlobalDescriptorSet(frameIndex)->GetDescriptorSet(), 0, VK_NULL_HANDLE);

	for (auto& [set, layout] : m_DescriptorsLayouts)
	{
		if (set != Pipeline::Set::Global)
		{
			// TODO: Try not use map, vector insted !
			auto resourcesAtSetIndex = m_Resources.find(set);
			if (resourcesAtSetIndex != m_Resources.end())
			{
				auto& resources = resourcesAtSetIndex->second;
				vkCmdBindDescriptorSets(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex), VK_PIPELINE_BIND_POINT_GRAPHICS,
					m_PipelineLayout, std::uint32_t(set), 1, &VulkanDescriptorsManager::ReciveDescriptor(layout, resources, frameIndex)->GetDescriptorSet(), 0, VK_NULL_HANDLE);
			}
			else
			{
				/*for (auto& binding : layout.GetDescriptorSetLayoutBindings())
				{
					std::string type;

					switch (VulkanDescriptorSet::Type(binding.descriptorType))
					{
					case VulkanDescriptorSet::Type::UniformBuffer: type = "UniformBuffer"; break;
					case VulkanDescriptorSet::Type::StorageBuffer: type = "StorageBuffer"; break;
					case VulkanDescriptorSet::Type::CombinedImageSampler: type = "CombinedImageSampler"; break;
					default:
						break;
					}

					SHADE_CORE_WARNING("Descriptor set layout requires resource type = '{0}', set = '{1}', bindidng = '{2}' but it has not been providet!", type, set, binding.binding);
				}*/
			}
		}
	}
}

void shade::VulkanPipeline::SetResource(SharedPointer<StorageBuffer> storageBuffer, Pipeline::Set set, std::uint32_t frameIndex, std::uint32_t offset)
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
void shade::VulkanPipeline::SetTexture(Asset<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex)
{
	if (texture)
		m_Resources[set].Images[binding][0] = texture->As<VulkanTexture2D>().GetDescriptorImageInfo();
	else
		m_Resources[set].Images[binding][0] = Renderer::GetDefaultDiffuseTexture()->As<VulkanTexture2D>().GetDescriptorImageInfo();
}
void shade::VulkanPipeline::SetTexture(SharedPointer<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex)
{
	if (texture)
		m_Resources[set].Images[binding][0] = texture->As<VulkanTexture2D>().GetDescriptorImageInfo();
	else
		m_Resources[set].Images[binding][0] = Renderer::GetDefaultDiffuseTexture()->As<VulkanTexture2D>().GetDescriptorImageInfo();
}
void shade::VulkanPipeline::SetTexturePerMipLevel(Asset<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t mipLevel)
{
	if (texture)
		m_Resources[set].Images[binding][0] = texture->As<VulkanTexture2D>().GetDescriptorImageInfoMip(mipLevel);
	else
		m_Resources[set].Images[binding][0] = Renderer::GetDefaultDiffuseTexture()->As<VulkanTexture2D>().GetDescriptorImageInfoMip(mipLevel);
}
void shade::VulkanPipeline::SetTexturePerMipLevel(SharedPointer<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t mipLevel)
{
	if (texture)
		m_Resources[set].Images[binding][0] = texture->As<VulkanTexture2D>().GetDescriptorImageInfoMip(mipLevel);
	else
		m_Resources[set].Images[binding][0] = Renderer::GetDefaultDiffuseTexture()->As<VulkanTexture2D>().GetDescriptorImageInfoMip(mipLevel);
}
void shade::VulkanPipeline::SetTexturePerLayer(Asset<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t layer)
{
	if (texture)
		m_Resources[set].Images[binding][0] = texture->As<VulkanTexture2D>().GetDescriptorImageInfoLayer(layer);
	else
		m_Resources[set].Images[binding][0] = Renderer::GetDefaultDiffuseTexture()->As<VulkanTexture2D>().GetDescriptorImageInfoLayer(layer);
}
void shade::VulkanPipeline::SetTexturePerLayer(SharedPointer<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t layer)
{
	if (texture)
		m_Resources[set].Images[binding][0] = texture->As<VulkanTexture2D>().GetDescriptorImageInfoLayer(layer);
	else
		m_Resources[set].Images[binding][0] = Renderer::GetDefaultDiffuseTexture()->As<VulkanTexture2D>().GetDescriptorImageInfoLayer(layer);
}

void shade::VulkanPipeline::UpdateResources(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex)
{
	BindDescriptorsSets(commandBuffer, frameIndex);
}

void shade::VulkanPipeline::SetMaterial(SharedPointer<StorageBuffer> buffer, std::uint32_t bufferoffset, const SharedPointer<Material>& material, std::uint32_t frameIndex)
{
	SetResource(buffer, Pipeline::Set::PerInstance, frameIndex, bufferoffset);
	SetTexture((material) ? material->TextureDiffuse : nullptr, Pipeline::Set::PerInstance, RenderAPI::DIFFUSE_TEXTURE_BINDING, frameIndex);
	SetTexture((material) ? material->TextureSpecular : nullptr, Pipeline::Set::PerInstance, RenderAPI::SPECULAR_TEXTURE_BINDING, frameIndex);
	SetTexture((material) ? material->TextureNormals : nullptr, Pipeline::Set::PerInstance, RenderAPI::NORMAL_TEXTURE_BINDING, frameIndex);
}

void shade::VulkanPipeline::SetMaterial(SharedPointer<StorageBuffer> buffer, std::uint32_t bufferoffset, const Asset<Material>& material, std::uint32_t frameIndex)
{
	SetResource(buffer, Pipeline::Set::PerInstance, frameIndex, bufferoffset);
	SetTexture((material) ? material->TextureDiffuse : nullptr, Pipeline::Set::PerInstance, RenderAPI::DIFFUSE_TEXTURE_BINDING, frameIndex);
	SetTexture((material) ? material->TextureSpecular : nullptr, Pipeline::Set::PerInstance, RenderAPI::SPECULAR_TEXTURE_BINDING, frameIndex);
	SetTexture((material) ? material->TextureNormals : nullptr, Pipeline::Set::PerInstance, RenderAPI::NORMAL_TEXTURE_BINDING, frameIndex);
}

void shade::VulkanPipeline::SetUniform(SharedPointer<RenderCommandBuffer>& commandBuffer, std::size_t size, const void* data, std::uint32_t frameIndex, Shader::TypeFlags shaderStage, std::uint32_t offset)
{
	vkCmdPushConstants(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex),
		GetPipelineLayout(), VulkanShader::FromShaderTypeFlagsToVkShaderTypeFlags(shaderStage), offset, size, data);
}

void shade::VulkanPipeline::SetBarrier(SharedPointer<RenderCommandBuffer>& commandBuffer, Stage srcStage, Stage dstStage, Access srcAccess, Access dstAccces, std::uint32_t frameIndex)
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

void shade::VulkanPipeline::SetBarrier(SharedPointer<RenderCommandBuffer>& commandBuffer, Asset<Texture2D> texture, Stage srcStage, Stage dstStage, Access srcAccess, Access dstAccces, std::uint32_t frameIndex, std::uint32_t mip)
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

void shade::VulkanPipeline::SetBarrier(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<Texture2D> texture, Stage srcStage, Stage dstStage, Access srcAccess, Access dstAccces, std::uint32_t frameIndex, std::uint32_t mip)
{

	bool isIsDepth			= VKUtils::IsDepthFormat(texture->GetImage()->As<VulkanImage2D>().GetSpecification().Format);
	bool issDepthStencil	= VKUtils::IsDepthStencilFormat(texture->GetImage()->As<VulkanImage2D>().GetSpecification().Format);

	VkImageAspectFlags aspect = (isIsDepth) ? VK_IMAGE_ASPECT_STENCIL_BIT  : (issDepthStencil) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_NONE;

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
				//texture->GetImage()->As<VulkanImage2D>().GetAspectMask() | aspect, // aspectMask
			VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT,
				(mip) ? mip : 0, // baseMipLevel
				(mip) ? 1 : texture->GetImage()->GetSpecification().MipLevels, // levelCount
				0, // baseArrayLayer
				texture->GetImage()->GetSpecification().Layers // layerCount
		}
	};
	vkCmdPipelineBarrier(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex),
		static_cast<VkPipelineStageFlags>(srcStage),
		static_cast<VkPipelineStageFlags>(dstStage),
		VK_DEPENDENCY_BY_REGION_BIT,
		0,
		VK_NULL_HANDLE,
		0,
		VK_NULL_HANDLE,
		1,
		&imageMemoryBarier);
}

void shade::VulkanPipeline::Recompile(bool clearCache)
{
	if (auto shader = Shader::Create(m_Specification.Shader->GetSpecification(), clearCache))
	{
		m_Specification.Shader = shader; Invalidate();
	}
}
