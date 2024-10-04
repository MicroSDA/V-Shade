#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/render/shader/Shader.h>
#include <shade/core/render/buffers/VertexBuffer.h>
#include <shade/core/render/buffers/RenderCommandBuffer.h>
#include <shade/core/render/buffers/StorageBuffer.h>
#include <shade/core/render/RenderData.h>


namespace shade
{
#ifndef BIND_PIPELINE_PROCESS_FUNCTION
#define BIND_PIPELINE_PROCESS_FUNCTION(pipeline, class_name, function_name, instance) \
    pipeline.Process = std::bind(std::mem_fn(&class_name::function_name), instance, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5)
#endif
#ifndef BIND_COMPUTE_PIPELINE_PROCESS_FUNCTION
	#define BIND_COMPUTE_PIPELINE_PROCESS_FUNCTION(pipeline, class_name, function_name, instance) \
    pipeline.Process = std::bind(std::mem_fn(&class_name::function_name), instance, std::placeholders::_1, std::placeholders::_2)
#endif

	class SHADE_API Pipeline
	{
	public:

		SHADE_CAST_HELPER(Pipeline)

		enum class Set : std::uint32_t
		{
			Global = 0,
			PerInstance = 1,
			User = 2
		};
		// See https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineStageFlagBits.html
		enum class Stage : std::uint32_t
		{
			TopPipe = 0x00000001,
			DrawInderect = 0x00000002,
			VertexInput = 0x00000004,
			VertexShader = 0x00000008,
			TessellationControlShader = 0x00000010,
			TessellationEvaluationShader = 0x00000020,
			GeometryShader = 0x00000040,
			FragmentShader = 0x00000080,

			ColorAttachmentOutput = 0x00000400,
			ComputeShader = 0x00000800,
			Transfer = 0x00001000,
			BottomPipe = 0x00002000,
			Host = 0x00004000,

			AllGraphics = 0x00008000,
			AllCommands = 0x00010000,
		};
		// See https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkAccessFlagBits.html
		// https://vulkan.lunarg.com/doc/view/1.3.250.1/windows/1.3-extensions/vkspec.html#synchronization-access-types-supported
		enum class Access : std::uint32_t
		{
			InderectCommandRead = 0x00000001,
			IndexRead = 0x00000002,
			VertexAttributeRead = 0x00000004,
			UniformRead = 0x00000008,
			InputAttachmentRead = 0x00000010,
			ShaderRead = 0x00000020,
			ShaderWrite = 0x00000040,
			ColorAttachmentRead = 0x00000080,
			ColorAttachmentWrite = 0x00000100,
			DepthStencilAttachmentRead = 0x00000200,
			DepthStencilAttachmentWrite = 0x00000400,
			TransferRead = 0x00000800,
			TransferWrite = 0x00001000,
			HostRead = 0x00002000,
			HostWrite = 0x00004000,
			MemoryRead = 0x00008000,
			MemoryWrite = 0x00010000
		};
		enum class PrimitiveTopology : std::uint32_t
		{
			Point = 0,
			Line = 1,
			LineStrip = 2,
			Triangle = 3,
			TriangleStrip = 4,
			TriangleFan = 5,
		};
		enum class PrimitivePolygonMode : std::uint32_t
		{
			Fill = 0,
			Line = 1,
			Point = 2,
		};
		enum class DepthTestFunction : std::uint32_t
		{
			Never = 0,
			Less = 1,
			Equal = 2,
			LessOrEqual = 3,
			Greater = 4,
			NotEqual = 5,
			GreaterOrEqual = 6,
			Always = 7
		};

		struct Specification
		{
			std::string					Name;
			SharedPointer<Shader>		Shader;
			SharedPointer<FrameBuffer>	FrameBuffer;
			VertexBuffer::Layout		VertexLayout;
			PrimitiveTopology			Topology = PrimitiveTopology::Triangle;
			PrimitivePolygonMode		PolygonMode = PrimitivePolygonMode::Fill;
			DepthTestFunction			DepthTest = DepthTestFunction::LessOrEqual;
			bool						BackFalceCull = true;
			bool						DepthClamp = false;
			bool						DepthTestEnabled = true;
			float						DepsBiasConstantFactor = 0.f;
			float						DepthBiasClamp = 0.f;
			float						DepthBiasSlopeFactor = 0.f;
			float						LineWidth = 2.f;
		};

		virtual void Recompile(bool clearCache = false) = 0;

		Specification& GetSpecification();
		const Specification& GetSpecification() const;

		float GetTimeQuery() const;
		void  SetTimeQuery(float time);
		bool  IsActive() const;
		void  SetActive(bool is);
	protected:
		Specification m_Specification;
		float m_TimeQuery = 0.f;
		bool m_IsActive = true;
	};

	class SHADE_API RenderPipeline : public Pipeline
	{
		SHADE_CAST_HELPER(RenderPipeline)
	public:
		static SharedPointer<RenderPipeline> Create(const Pipeline::Specification& specification);
		virtual ~RenderPipeline() = default;

		virtual void Bind(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex) = 0;

		virtual void SetResource(SharedPointer<StorageBuffer> storageBuffer, Pipeline::Set set, std::uint32_t frameIndex, std::uint32_t offset = 0) = 0;

		virtual void SetTexture(Asset<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex) = 0;
		virtual void SetTexture(SharedPointer<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex) = 0;

		virtual void SetTexturePerMipLevel(Asset<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t mipLevel) = 0;
		virtual void SetTexturePerMipLevel(SharedPointer<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t mipLevel) = 0;

		virtual void SetTexturePerLayer(Asset<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t layer) = 0;
		virtual void SetTexturePerLayer(SharedPointer<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t layer) = 0;

		virtual void SetMaterial(SharedPointer<StorageBuffer> buffer, std::uint32_t bufferoffset, const Asset<Material>& material,std::uint32_t frameIndex) = 0;

		virtual void SetMaterial(SharedPointer<StorageBuffer> buffer, std::uint32_t bufferoffset, const SharedPointer<Material>& material, std::uint32_t frameIndex) = 0;
		virtual void SetUniform(SharedPointer<RenderCommandBuffer>& commandBuffer, std::size_t size, const void* data, std::uint32_t frameIndex, Shader::TypeFlags shaderStage, std::uint32_t offset = 0) = 0;

		virtual void UpdateResources(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex) = 0;

		virtual void SetBarrier(SharedPointer<RenderCommandBuffer>& commandBuffer, Stage srcStage, Stage dstStage, Access srcAccess, Access dstAccces, std::uint32_t frameIndex) = 0;
		
		std::function<void(SharedPointer<RenderPipeline>&, const render::SubmitedInstances&, const render::SubmitedSceneRenderData&, std::uint32_t, bool)> Process;
	};
	
	class SHADE_API ComputePipeline : public Pipeline
	{

		SHADE_CAST_HELPER(ComputePipeline)

	public:
		static SharedPointer<ComputePipeline> Create(const Pipeline::Specification& specification);
		virtual ~ComputePipeline() = default;

		virtual void Begin(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex) = 0;
		virtual void Dispatch(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t groupCountX, std::uint32_t groupCountY, std::uint32_t groupCountZ, std::uint32_t frameIndex) = 0;

		virtual void SetTexture(Asset<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex) = 0;
		virtual void SetTexture(SharedPointer<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex) = 0;

		virtual void SetTexturePerMipLevel(Asset<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t mipLevel) = 0;
		virtual void SetTexturePerMipLevel(SharedPointer<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t mipLevel) = 0;

		virtual void SetTexturePerLayer(Asset<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t layer) = 0;
		virtual void SetTexturePerLayer(SharedPointer<Texture2D> texture, Pipeline::Set set, std::uint32_t binding, std::uint32_t frameIndex, std::uint32_t layer) = 0;

		virtual void SetResource(SharedPointer<StorageBuffer> storageBuffer, Pipeline::Set set, std::uint32_t frameIndex, std::uint32_t offset = 0) = 0;
		virtual void SetResource(SharedPointer<UniformBuffer> uniformBuffer, Pipeline::Set set, std::uint32_t frameIndex, std::uint32_t offset = 0) = 0;

		virtual void SetUniform(SharedPointer<RenderCommandBuffer>& commandBuffer, std::size_t size, const void* data, std::uint32_t frameIndex, std::uint32_t offset = 0) = 0;

		virtual void SetBarrier(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<StorageBuffer> buffer, Stage srcStage, Stage dstStage, Access srcAccess, Access dstAccces, std::uint32_t frameIndex) = 0;

		virtual void SetBarrier(SharedPointer<RenderCommandBuffer>& commandBuffer, Asset<Texture2D> texture, Stage srcStage, Stage dstStage, Access srcAccess, Access dstAccces, std::uint32_t frameIndex, std::uint32_t mip = 0) = 0;
		virtual void SetBarrier(SharedPointer<RenderCommandBuffer>& commandBuffer, SharedPointer<Texture2D> texture, Stage srcStage, Stage dstStage, Access srcAccess, Access dstAccces, std::uint32_t frameIndex, std::uint32_t mip = 0) = 0;
		virtual void SetBarrier(SharedPointer<RenderCommandBuffer>& commandBuffer, Stage srcStage, Stage dstStage, Access srcAccess, Access dstAccces, std::uint32_t frameIndex) = 0;
		virtual void UpdateResources(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex) = 0;

		std::function<void(SharedPointer<ComputePipeline>& pipeline, std::uint32_t)> Process;
	};
}