#pragma once
#include <shade/core/render/shader/Shader.h>
#include <shade/platforms/render/vulkan/VulkanContext.h>
#include <shade/platforms/render/vulkan/VulkanShader.h>
#include <shade/platforms/render/vulkan/buffers/VulkanFrameBuffer.h>
#include <shade/platforms/render/vulkan/buffers/VulkanCommandBuffer.h>
#include <shade/platforms/render/vulkan/buffers/VulkanStorageBuffer.h>
#include <shade/platforms/render/vulkan/buffers/VulkanUniformBuffer.h>
#include <shade/platforms/render/vulkan/VulkanTexture2D.h>
#include <shade/platforms/render/vulkan/descriptors/VulkanDescriptor.h>

#include <shade/core/render/RenderPipeline.h>

namespace shade
{
	class VulkanPipeline : public RenderPipeline
	{
	public:
		VulkanPipeline(const RenderPipeline::Specification& specification);
		void Invalidate();
		virtual ~VulkanPipeline();

		VkPipeline GetPipeline();
		VkPipelineLayout GetPipelineLayout();

		virtual void Bind(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex) override;
		void BindDescriptorsSets(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex);

		virtual void SetResource(SharedPointer<StorageBuffer> storageBuffer, Pipeline::Set set, std::uint32_t frameIndex, std::uint32_t offset) override;

		virtual void SetTexture(Asset<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex) override;
		virtual void SetTexture(SharedPointer<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex) override;

		virtual void SetTexturePerMipLevel(Asset<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t mipLevel) override;
		virtual void SetTexturePerMipLevel(SharedPointer<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t mipLevel) override;

		virtual void SetTexturePerLayer(Asset<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t layer) override;
		virtual void SetTexturePerLayer(SharedPointer<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t layer) override;

		virtual void UpdateResources(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex) override;
		virtual void SetMaterial(SharedPointer<StorageBuffer> buffer, std::uint32_t bufferoffset, const SharedPointer<Material>& material, std::uint32_t frameIndex) override;
		virtual void SetMaterial(SharedPointer<StorageBuffer> buffer, std::uint32_t bufferoffset, const Asset<Material>& material, std::uint32_t frameIndex) override;
		virtual void SetUniform(SharedPointer<RenderCommandBuffer>& commandBuffer, std::size_t size, const void* data, std::uint32_t frameIndex, std::uint32_t offset) override;

		virtual void SetBarrier(SharedPointer<RenderCommandBuffer>& commandBuffer, Stage srcStage, Stage dstStage, Access srcAccess, Access dstAccces, std::uint32_t frameIndex) override;

		virtual void Recompile() override;
	private:
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		// Dynamic rendering 
		VkPipelineRenderingCreateInfo m_PipelineRenderingCreateInfo;
		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		// Set - >
		std::map<Pipeline::Set, VulkanDescriptorSetLayout> m_DescriptorsLayouts;
		// Set - >
		std::map<Pipeline::Set, DescriptorBufferBindings> m_Resources;
	};
}