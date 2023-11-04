#pragma once
#include <shade/core/render/RenderPipeline.h>
#include <shade/platforms/render/vulkan/VulkanShader.h>
#include <shade/platforms/render/vulkan/descriptors/VulkanDescriptor.h>
#include <vulkan/vulkan.h>

namespace shade
{
	class VulkanComputePipeline : public ComputePipeline
	{
	public:
		VulkanComputePipeline(const VkDevice& device, const VulkanContext::VulkanInstance& instance, const ComputePipeline::Specification& specification);
		virtual ~VulkanComputePipeline();

		virtual void Begin(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex) override;
		virtual void Dispatch(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t groupCountX, std::uint32_t groupCountY, std::uint32_t groupCountZ, std::uint32_t frameIndex) override;

		virtual void SetTexture(Asset<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex) override;
		virtual void SetTexture(SharedPointer<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex) override;

		virtual void SetTexturePerMipLevel(Asset<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t mipLevel) override;
		virtual void SetTexturePerMipLevel(SharedPointer<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t mipLevel) override;

		virtual void SetTexturePerLayer(Asset<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t layer) override;
		virtual void SetTexturePerLayer(SharedPointer<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t layer) override;

		virtual void SetResource(SharedPointer<StorageBuffer> storageBuffer, Pipeline::Set set, std::uint32_t frameIndex, std::uint32_t offset) override;
		virtual void SetResource(SharedPointer<UniformBuffer> uniformBuffer, Pipeline::Set set, std::uint32_t frameIndex, std::uint32_t offset) override;

		virtual void SetUniform(SharedPointer<RenderCommandBuffer>& commandBuffer, std::size_t size, const void* data, std::uint32_t frameIndex, std::uint32_t offset) override;

		virtual void UpdateResources(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex) override;

		virtual void SetBarrier(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<StorageBuffer> buffer, Stage srcStage, Stage dstStage, Access srcAccess, Access dstAccces, std::uint32_t frameIndex) override;

		virtual void SetBarrier(SharedPointer<RenderCommandBuffer>& commandBuffer, Asset<Texture2D> texture, Stage srcStage, Stage dstStage, Access srcAccess, Access dstAccces, std::uint32_t frameIndex, std::uint32_t mip) override;
		virtual void SetBarrier(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<Texture2D> texture, Stage srcStage, Stage dstStage, Access srcAccess, Access dstAccces, std::uint32_t frameIndex, std::uint32_t mip) override;
		virtual void SetBarrier(SharedPointer<RenderCommandBuffer>& commandBuffer, Stage srcStage, Stage dstStage, Access srcAccess, Access dstAccces, std::uint32_t frameIndex) override;

		virtual void Recompile() override;

		void BindDescriptorsSets(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex);

	private:
		VkDevice						m_VkDevice = VK_NULL_HANDLE;
		VulkanContext::VulkanInstance	m_VkInstance;
		VkPipelineLayout				m_PipelineLayout = VK_NULL_HANDLE;
		VkPipeline						m_Pipeline = VK_NULL_HANDLE;

		// Set - >
		std::map<Pipeline::Set, VulkanDescriptorSetLayout> m_DescriptorsLayouts;
		// Set - >
		std::map<Pipeline::Set, DescriptorBufferBindings> m_Resources;
	private:
		void Invalidate();
	};
}