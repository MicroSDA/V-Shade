#pragma once
#include <shade/core/render/RenderAPI.h>
#include <shade/platforms/render/vulkan/VulkanShader.h>
#include <shade/platforms/render/vulkan/VulkanSwapChain.h>
#include <shade/platforms/render/vulkan/VulkanPipeline.h>
#include <shade/platforms/render/vulkan/buffers/VulkanCommandBuffer.h>
#include <shade/platforms/render/vulkan/buffers/VulkanVertexBuffer.h>
#include <shade/platforms/render/vulkan/buffers/VulkanIndexBuffer.h>
#include <shade/platforms/render/vulkan/descriptors/VulkanDescriptor.h>
#include <vulkan/vulkan.h>

namespace shade
{
	class VulkanRenderAPI : public RenderAPI
	{
	public:
		virtual UniquePointer<RenderContext> Initialize(const SystemsRequirements& requirements) override;
		virtual void ShutDown() override;
		virtual void BeginFrame(std::uint32_t frameIndex) override;
		virtual void EndFrame(std::uint32_t frameIndex) override;

		virtual void BeginScene(SharedPointer<Camera>& camera, std::uint32_t frameIndex) override;
		virtual void EndScene(std::uint32_t frameIndex) override;

		virtual void BeginRender(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<RenderPipeline>& pipeline, std::uint32_t frameIndex, bool clear, std::uint32_t clearCount) override;
		virtual void BeginRenderWithCustomomViewPort(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<RenderPipeline>& pipeline, std::uint32_t frameIndex, glm::vec2 viewPort, bool isClear) override;

		virtual void EndRender(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex) override;

		virtual void DrawInstanced(
			SharedPointer<RenderCommandBuffer>& commandBuffer, 
			const SharedPointer<VertexBuffer>& vertices, 
			const SharedPointer<IndexBuffer>& indices,
			const SharedPointer<VertexBuffer>& transforms,
			std::uint32_t count,
			std::uint32_t transformOffset) override;

		virtual void DrawInstancedAnimated(
			SharedPointer<RenderCommandBuffer>& commandBuffer,
			const SharedPointer<VertexBuffer>& vertices,
			const SharedPointer<IndexBuffer>& indices,
			const SharedPointer<VertexBuffer>& bones,
			const SharedPointer<VertexBuffer>& transforms,
			std::uint32_t count,
			std::uint32_t transformOffset) override;

		static const std::shared_ptr<shade::VulkanDescriptorSet> GetGlobalDescriptorSet(std::uint32_t frameIndex);

		virtual void BeginTimestamp(SharedPointer<RenderCommandBuffer>& commandBuffer, const std::string& name) override;
		virtual float EndTimestamp(SharedPointer<RenderCommandBuffer>& commandBuffer, const std::string& name) override;
		virtual VramUsage GetVramMemoryUsage() override;


		static void UpdateMaterial(std::uint32_t frameIndex, std::uint32_t offset);
		// Return global descriptor set
		static const UniquePointer<VulkanDescriptorSetLayout>& GetGlobalDescriptorSetLayout();
		static const std::vector<VkPushConstantRange>& GlobalPushConstantRanges();

		std::uint32_t GetMaxImageLayers() const override;
		std::uint32_t GetMaxViewportsCount() const override;
	private:
		struct VulkanGlobalSeceneData
		{
			std::unordered_map<Pipeline::Set, DescriptorBufferBindings> Bindings;
			UniquePointer<VulkanDescriptorSetLayout> DescriptorSetLayout;
			std::vector<VkPushConstantRange> PushConstantRanges;
		};

		static VulkanGlobalSeceneData m_sVulkanGlobaSceneData;

		static VkDevice m_sVkDevice;
		static VulkanContext::VulkanInstance m_sVkInstance;

		static std::unordered_map<std::string, VkQueryPool> m_sQueryPools;
	};

}