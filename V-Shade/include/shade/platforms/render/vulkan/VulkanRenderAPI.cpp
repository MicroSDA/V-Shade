#include "shade_pch.h"
#include "VulkanRenderAPI.h"
#include <shade/core/application/Application.h> // TODO: Temporary
#include <shade/core/event/Input.h>

shade::VulkanRenderAPI::VulkanGlobalSeceneData shade::VulkanRenderAPI::m_sVulkanGlobaSceneData;
VkDevice shade::VulkanRenderAPI::m_sVkDevice = VK_NULL_HANDLE;
shade::VulkanContext::VulkanInstance shade::VulkanRenderAPI::m_sVkInstance;
std::unordered_map<std::string, std::pair<VkQueryPool, float>> shade::VulkanRenderAPI::m_sQueryPools;

shade::UniquePointer<shade::RenderContext> shade::VulkanRenderAPI::Initialize(const SystemsRequirements& requirements)
{
	/* Create render context */
	UniquePointer<RenderContext> renderContext = RenderContext::Create();
	/* Init context */
	renderContext->Initialize(requirements);

	m_sSystemsRequirements = requirements;
	m_sVkDevice = renderContext->As<VulkanContext>().GetLogicalDevice()->GetDevice();
	m_sVkInstance = renderContext->As<VulkanContext>().GetInstance();
	/* Init Descriptor Manager */
	VulkanDescriptorsManager::Initialize(m_sVkDevice, m_sVkInstance, GetFramesCount());

	/* Create global descriptor layout */
	m_sVulkanGlobaSceneData.DescriptorSetLayout = UniquePointer<VulkanDescriptorSetLayout>::Create(
		m_sVkDevice,
		m_sVkInstance,
		std::initializer_list<VkDescriptorSetLayoutBinding>
	{
		// Camera
		{
			.binding = CAMERA_BINDING,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
			.pImmutableSamplers = VK_NULL_HANDLE
		},
			// Scene Data
		{
			.binding = SCENE_RENDER_DATA_BINDING,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
			.pImmutableSamplers = VK_NULL_HANDLE
		},
			// Render settings
		{
			.binding = RENDER_SETTINGS_BINDING,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
			.pImmutableSamplers = VK_NULL_HANDLE
		},
			// Directional lights
		{
			.binding = GLOBAL_LIGHT_BINDING,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
			.pImmutableSamplers = VK_NULL_HANDLE
		},
			// Point lights
		{
			.binding = POINT_LIGHT_BINDING,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
			.pImmutableSamplers = VK_NULL_HANDLE
		},
			// Spot lights
		{
			.binding = SPOT_LIGHT_BINDING,
			.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
			.pImmutableSamplers = VK_NULL_HANDLE
		}
	});

	return renderContext;
}

void shade::VulkanRenderAPI::ShutDown()
{
	for (auto& [name, pool] : m_sQueryPools)
		vkDestroyQueryPool(m_sVkDevice, pool.first, m_sVkInstance.AllocationCallbaks);

	m_sVulkanGlobaSceneData.DescriptorSetLayout.Reset();

	VulkanDescriptorsManager::ShutDown();
}

void shade::VulkanRenderAPI::BeginFrame(std::uint32_t frameIndex)
{
	SetCurrentFrameIndex(frameIndex);
	//VulkanDescriptorsManager::ResetDepricated(frameIndex);
	// For RenderDoc
	VulkanDescriptorsManager::ResetAllDescripotrs(frameIndex);
}

void shade::VulkanRenderAPI::EndFrame(std::uint32_t frameIndex)
{

}

void shade::VulkanRenderAPI::BeginScene(SharedPointer<Camera>& camera, std::uint32_t frameIndex)
{
	// Update global descriptors
	// It's very importatnt to update all descriptrs at the bgining of scene and before any piplines will use it!
	m_sVulkanGlobaSceneData.Bindings[Pipeline::Set::Global].Buffers[CAMERA_BINDING][0] = m_sSubmitedSceneRenderData.CameraBuffer->As<VulkanUniformBuffer>().GetDescriptorBufferInfo(frameIndex);
	m_sVulkanGlobaSceneData.Bindings[Pipeline::Set::Global].Buffers[SCENE_RENDER_DATA_BINDING][0] = m_sSubmitedSceneRenderData.SceneRenderDataBuffer->As<VulkanUniformBuffer>().GetDescriptorBufferInfo(frameIndex);
	m_sVulkanGlobaSceneData.Bindings[Pipeline::Set::Global].Buffers[RENDER_SETTINGS_BINDING][0] = m_sSubmitedSceneRenderData.RenderSettingsDataBuffer->As<VulkanUniformBuffer>().GetDescriptorBufferInfo(frameIndex);
	m_sVulkanGlobaSceneData.Bindings[Pipeline::Set::Global].Buffers[GLOBAL_LIGHT_BINDING][0] = m_sSubmitedSceneRenderData.GlobalLightsBuffer->As<VulkanStorageBuffer>().GetDescriptorBufferInfo(frameIndex);
	m_sVulkanGlobaSceneData.Bindings[Pipeline::Set::Global].Buffers[POINT_LIGHT_BINDING][0] = m_sSubmitedSceneRenderData.PointsLightsBuffer->As<VulkanStorageBuffer>().GetDescriptorBufferInfo(frameIndex);
	m_sVulkanGlobaSceneData.Bindings[Pipeline::Set::Global].Buffers[SPOT_LIGHT_BINDING][0] = m_sSubmitedSceneRenderData.SpotLightsBuffer->As<VulkanStorageBuffer>().GetDescriptorBufferInfo(frameIndex);
	m_sVulkanGlobaSceneData.Bindings[Pipeline::Set::PerInstance].Buffers[MATERIAL_BINDING][0] = m_sSubmitedSceneRenderData.MaterialsBuffer->As<VulkanStorageBuffer>().GetDescriptorBufferInfo(frameIndex);
}

void shade::VulkanRenderAPI::EndScene(std::uint32_t frameIndex)
{

}
// Clear doesnt work right now
void shade::VulkanRenderAPI::BeginRender(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<RenderPipeline>& pipeline, std::uint32_t frameIndex, bool clear, std::uint32_t clearCount)
{
	auto vulkanCommandBuffer = commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex);

	if (pipeline->GetSpecification().FrameBuffer != nullptr)
	{

		auto& frameBuffer = pipeline->GetSpecification().FrameBuffer->As<VulkanFrameBuffer>();
		auto& renderingInfo = frameBuffer.GetRenderingInfo();
		
		std::vector<VkViewport> viewports(renderingInfo.layerCount);
		std::vector<VkRect2D>   scissors(renderingInfo.layerCount);

		for (auto i = 0; i < renderingInfo.layerCount; i++)
		{
			// X and Y position 
			viewports[i] = { 0.f, 0.f, static_cast<float>(frameBuffer.GetSpecification().Width), static_cast<float>(frameBuffer.GetSpecification().Height), // Size
				// Min and Max depth values
				  0.f, 1.f
			};
			scissors[i] = { {0, 0} /*offset*/, { frameBuffer.GetSpecification().Width, frameBuffer.GetSpecification().Height } /*extent*/ };
		}

		vkCmdBeginRendering(vulkanCommandBuffer, &renderingInfo);

		if (clear)
		{
			static VkClearRect clearRect;
			clearRect =
			{
				.rect = renderingInfo.renderArea,
				.baseArrayLayer = 0,
				.layerCount = (clearCount) ? clearCount : renderingInfo.layerCount,
			};

			vkCmdClearAttachments(vulkanCommandBuffer, frameBuffer.GetClearAttachments().size(), frameBuffer.GetClearAttachments().data(), 1, &clearRect);
		}


		//for (auto i = 0; i < renderingInfo.colorAttachmentCount; i++)
		//{
		//	VkImageMemoryBarrier barrier
		//	{
		//		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		//		.pNext = VK_NULL_HANDLE,
		//		.srcAccessMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		//		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		//		.oldLayout = VK_IMAGE_LAYOUT_GENERAL,
		//		.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		//		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		//		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		//		.image = frameBuffer.GetTextureAttachment(i)->GetImage()->As<VulkanImage2D>().GetImage(),
		//		.subresourceRange =
		//		{
		//				frameBuffer.GetTextureAttachment(i)->GetImage()->As<VulkanImage2D>().GetAspectMask(), // aspectMask
		//				0, // baseMipLevel
		//				1, // levelCount
		//				0, // baseArrayLayer
		//				1 // layerCount
		//		}
		//	};

		//	vkCmdPipelineBarrier(vulkanCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		//}

		vkCmdSetViewport(vulkanCommandBuffer, 0, (viewports.size() > GetMaxViewportsCount()) ? GetMaxViewportsCount() : viewports.size(), viewports.data());
		vkCmdSetScissor(vulkanCommandBuffer, 0, (scissors.size() > GetMaxViewportsCount()) ? GetMaxViewportsCount() : scissors.size(), scissors.data());

		
		pipeline->Bind(commandBuffer, frameIndex);

	}
	else // Frosm swapchain
	{
		auto& frameBuffer = Application::GetWindow()->GetSwapChain()->GetFrameBuffers()[m_sCurrentFrameIndex]->As<VulkanFrameBuffer>();
		auto& renderingInfo = frameBuffer.GetRenderingInfo();

		// Normal -Y version
		VkViewport viewport
		{ 0.f,  0.f, // X and Y position
		  static_cast<float>(frameBuffer.GetSpecification().Width), static_cast<float>(frameBuffer.GetSpecification().Height), // Size
		  0.f, 1.f // Min and Max depth values
		};
		VkRect2D scissor{ {0, 0} /*offset*/, { frameBuffer.GetSpecification().Width, frameBuffer.GetSpecification().Height } /*extent*/ };

		vkCmdBeginRendering(vulkanCommandBuffer, &renderingInfo);

		if (clear)
		{
			VkClearRect clearRect
			{
				.rect = renderingInfo.renderArea,
				.baseArrayLayer = 0,
				.layerCount = renderingInfo.layerCount,
			};

			vkCmdClearAttachments(vulkanCommandBuffer, frameBuffer.GetClearAttachments().size(), frameBuffer.GetClearAttachments().data(), 1, &clearRect);
		}

		vkCmdSetViewport(vulkanCommandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(vulkanCommandBuffer, 0, 1, &scissor);
		pipeline->Bind(commandBuffer, m_sCurrentFrameIndex);
	}
}

void shade::VulkanRenderAPI::BeginRenderWithCustomomViewPort(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<RenderPipeline>& pipeline, std::uint32_t frameIndex, glm::vec2 viewPort, bool clear)
{
	auto vulkanCommandBuffer = commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex);
	auto& frameBuffer = pipeline->GetSpecification().FrameBuffer->As<VulkanFrameBuffer>();
	auto& renderingInfo = frameBuffer.GetRenderingInfo();

	VkViewport viewport = { 0.f, 0.f,    viewPort.x, viewPort.y, 0.f,  1.f };
	VkRect2D scissor = { {0.f, 0.f}, {viewPort.x, viewPort.y } };

	vkCmdBeginRendering(vulkanCommandBuffer, &renderingInfo);

	if (clear)
	{
		static VkClearRect clearRect;
		clearRect =
		{
			.rect = renderingInfo.renderArea,
			.baseArrayLayer = 0,
			.layerCount = 1,
		};

		vkCmdClearAttachments(vulkanCommandBuffer, frameBuffer.GetClearAttachments().size(), frameBuffer.GetClearAttachments().data(), 1, &clearRect);
	}

	vkCmdSetViewport(vulkanCommandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(vulkanCommandBuffer, 0, 1, &scissor);

	pipeline->Bind(commandBuffer, frameIndex);
}

void shade::VulkanRenderAPI::EndRender(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex)
{
	vkCmdEndRendering(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(frameIndex));
}

void shade::VulkanRenderAPI::DrawInstanced(SharedPointer<RenderCommandBuffer>& commandBuffer, const SharedPointer<VertexBuffer>& vertices, const SharedPointer<IndexBuffer>& indices, const SharedPointer<VertexBuffer>& transforms, std::uint32_t count, std::uint32_t transformOffset)
{
	static constexpr std::uint32_t VERTEX_BINDING = 0, TRANSFORMS_BINDING = 1;

	if (vertices && indices && transforms)
	{
		vertices->Bind(commandBuffer, m_sCurrentFrameIndex, VERTEX_BINDING);
		indices->Bind(commandBuffer, m_sCurrentFrameIndex);
		transforms->Bind(commandBuffer, m_sCurrentFrameIndex, TRANSFORMS_BINDING, transformOffset);

		vkCmdDrawIndexed(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(m_sCurrentFrameIndex), static_cast<std::uint32_t>(indices->GetCount()), count, 0, 0, 0);
	}
	
}

void shade::VulkanRenderAPI::DrawInstancedAnimated(SharedPointer<RenderCommandBuffer>& commandBuffer, const SharedPointer<VertexBuffer>& vertices, const SharedPointer<IndexBuffer>& indices, const SharedPointer<VertexBuffer>& bones, const SharedPointer<VertexBuffer>& transforms, std::uint32_t count, std::uint32_t transformOffset)
{
	static constexpr std::uint32_t VERTEX_BINDING = 0, BONES_BINDING = 1, TRANSFORMS_BINDING = 2;

	if (vertices && indices && bones && transforms)
	{
		vertices->Bind(commandBuffer, m_sCurrentFrameIndex, VERTEX_BINDING);
		indices->Bind(commandBuffer, m_sCurrentFrameIndex);
		bones->Bind(commandBuffer, m_sCurrentFrameIndex, BONES_BINDING);
		transforms->Bind(commandBuffer, m_sCurrentFrameIndex, TRANSFORMS_BINDING, transformOffset);

		vkCmdDrawIndexed(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(m_sCurrentFrameIndex), static_cast<std::uint32_t>(indices->GetCount()), count, 0, 0, 0);
	}
}

void shade::VulkanRenderAPI::DummyInvocation(SharedPointer<RenderCommandBuffer>& commandBuffer, const SharedPointer<VertexBuffer>& vertices, const SharedPointer<IndexBuffer>& indices, const SharedPointer<VertexBuffer>& bones, const SharedPointer<VertexBuffer>& transforms, std::uint32_t count, std::uint32_t transformOffset)
{
	static constexpr std::uint32_t VERTEX_BINDING = 0, BONES_BINDING = 1, TRANSFORMS_BINDING = 2;

	/*if (vertices && indices && bones && transforms)
	{
		vertices->Bind(commandBuffer, m_sCurrentFrameIndex, VERTEX_BINDING);
		indices->Bind(commandBuffer, m_sCurrentFrameIndex);
		bones->Bind(commandBuffer, m_sCurrentFrameIndex, BONES_BINDING);
		transforms->Bind(commandBuffer, m_sCurrentFrameIndex, TRANSFORMS_BINDING, transformOffset);
	}*/
	
	// It was created for draw bones !!
	vkCmdDraw(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(m_sCurrentFrameIndex), 1, RenderAPI::MAX_BONES_PER_INSTANCE, 0, 0);
}

const std::shared_ptr<shade::VulkanDescriptorSet> shade::VulkanRenderAPI::GetGlobalDescriptorSet(std::uint32_t frameIndex)
{
	return VulkanDescriptorsManager::ReciveDescriptor(*m_sVulkanGlobaSceneData.DescriptorSetLayout, m_sVulkanGlobaSceneData.Bindings[Pipeline::Set::Global], frameIndex);
}

void shade::VulkanRenderAPI::BeginTimestamp(SharedPointer<RenderCommandBuffer>& commandBuffer, const std::string& name)
{
	if (m_sQueryPools[name].first == VK_NULL_HANDLE)
	{
		VkQueryPoolCreateInfo queryPoolCreateInfo
		{
			 .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
			 .pNext = VK_NULL_HANDLE,
			 .flags = 0,
			 .queryType = VK_QUERY_TYPE_TIMESTAMP,
			 .queryCount = 2,
			 .pipelineStatistics = 0
		};
		VK_CHECK_RESULT(vkCreateQueryPool(m_sVkDevice, &queryPoolCreateInfo, nullptr, &m_sQueryPools.at(name).first), "Failed to create m_sQueryPool");
	}

	vkCmdResetQueryPool(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(m_sCurrentFrameIndex), m_sQueryPools.at(name).first, 0, 2);
	vkCmdWriteTimestamp(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(m_sCurrentFrameIndex), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_sQueryPools.at(name).first, 0);
}

float shade::VulkanRenderAPI::EndTimestamp(SharedPointer<RenderCommandBuffer>& commandBuffer, const std::string& name)
{
	assert(m_sQueryPools.at(name).first != VK_NULL_HANDLE && "Trying to end undefined render timestamp !");

	vkCmdWriteTimestamp(commandBuffer->As<VulkanCommandBuffer>().GetCommandBuffer(m_sCurrentFrameIndex), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_sQueryPools.at(name).first, 1);
	// Doenst work at this moment 
	//vkGetQueryPoolResults(m_sVkDevice, m_sQueryPools[name], 0, 2, sizeof(glm::vec<2, std::uint64_t>), glm::value_ptr(time), sizeof(std::uint64_t), VK_QUERY_RESULT_64_BIT);

	//vkGetQueryPoolResults(m_sVkDevice, m_sQueryPools[name], 1, 1, sizeof(uint64_t), &end, 0, VK_QUERY_RESULT_64_BIT);

	return 0.0;
}

void shade::VulkanRenderAPI::QueryResults(std::uint32_t frameIndex)
{
	for (auto& [name, q] : m_sQueryPools)
	{
		uint64_t start = 0, end = 0;
		vkGetQueryPoolResults(m_sVkDevice, q.first, 0, 1, sizeof(uint64_t), &start, 0, VK_QUERY_RESULT_64_BIT);
		vkGetQueryPoolResults(m_sVkDevice, q.first, 1, 1, sizeof(uint64_t), &end, 0, VK_QUERY_RESULT_64_BIT);
		q.second = (end - start) / 1000000.f;
	}
}

float shade::VulkanRenderAPI::GetQueryResult(const std::string& name)
{
	auto q = m_sQueryPools.find(name);

	if (q != m_sQueryPools.end())
	{
		return q->second.second;
	}
	else
	{
		return 0.f;
	}
	
}

shade::RenderAPI::VramUsage shade::VulkanRenderAPI::GetVramMemoryUsage()
{
	VkPhysicalDeviceMemoryProperties2 memoryProperties = {};
	memoryProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;

	VkPhysicalDeviceMemoryBudgetPropertiesEXT memoryBudgetProperties = {};
	memoryBudgetProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT;
	memoryProperties.pNext = &memoryBudgetProperties;

	vkGetPhysicalDeviceMemoryProperties2(VulkanContext::GetPhysicalDevice()->GetDevice(), &memoryProperties);

	VramUsage usage{ std::vector<VramUsage::Heap>(memoryProperties.memoryProperties.memoryHeapCount) };

	for (std::uint32_t i = 0; i < usage.Heaps.size(); ++i)
	{
		usage.Heaps[i].UsedMemoryBytes  = memoryBudgetProperties.heapUsage[i];
		usage.Heaps[i].TotalMemoryBytes = memoryProperties.memoryProperties.memoryHeaps[i].size;
		usage.Heaps[i].AvailableMemory  = memoryBudgetProperties.heapBudget[i];
	}
	return usage;
}

void shade::VulkanRenderAPI::UpdateMaterial(std::uint32_t frameIndex, std::uint32_t offset)
{
	// Update descriptor buffer info with data from MaterialsBuffer and frameIndex
	m_sVulkanGlobaSceneData.Bindings[Pipeline::Set::PerInstance].Buffers[MATERIAL_BINDING][0] = m_sSubmitedSceneRenderData.MaterialsBuffer->As<VulkanStorageBuffer>().GetDescriptorBufferInfo(frameIndex);

	// Add offset to existing buffer offset in Bindings.Buffers[1][0]
	m_sVulkanGlobaSceneData.Bindings[Pipeline::Set::PerInstance].Buffers[MATERIAL_BINDING][0].offset += offset;

	// Subtract offset from existing buffer range in Bindings.Buffers[1][0]
	m_sVulkanGlobaSceneData.Bindings[Pipeline::Set::PerInstance].Buffers[MATERIAL_BINDING][0].range -= offset;
}

const shade::UniquePointer<shade::VulkanDescriptorSetLayout>& shade::VulkanRenderAPI::GetGlobalDescriptorSetLayout()
{
	return m_sVulkanGlobaSceneData.DescriptorSetLayout;
}

const std::vector<VkPushConstantRange>& shade::VulkanRenderAPI::GlobalPushConstantRanges()
{
	return m_sVulkanGlobaSceneData.PushConstantRanges;
}

std::uint32_t shade::VulkanRenderAPI::GetMaxImageLayers() const
{
	return VulkanContext::GetPhysicalDevice()->GetPhysicalDeviceProperties().limits.maxImageArrayLayers;
}

std::uint32_t shade::VulkanRenderAPI::GetMaxViewportsCount() const
{
	//VulkanContext::GetPhysicalDevice()->GetPhysicalDeviceProperties().limits.maxGeometryOutputVertices;
	return VulkanContext::GetPhysicalDevice()->GetPhysicalDeviceProperties().limits.maxViewports;
}
