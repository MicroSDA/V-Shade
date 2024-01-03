#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/render/buffers/RenderCommandBuffer.h>

namespace shade
{
	class SHADE_API SwapChain
	{

		SHADE_CAST_HELPER(SwapChain)

	public:
		virtual~SwapChain() = default;
		static UniquePointer<SwapChain> Create();

		virtual void Initialize() = 0;
		virtual void CreateFrame(std::uint32_t* width, std::uint32_t* height, std::uint32_t framesCount, bool vsync) = 0;
		virtual void OnResize(std::uint32_t width, std::uint32_t height) = 0;
		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;
		virtual void Present(std::uint32_t timeout = UINT32_MAX) = 0;
		virtual void Destroy() = 0;
		virtual const std::uint32_t& GetCurrentBufferIndex() const = 0;

		void SetFramesCount(std::uint32_t count);
		std::uint32_t GetFramesCount();

		SharedPointer<RenderCommandBuffer> GetCommandBuffer();
		virtual std::vector<SharedPointer<FrameBuffer>>& GetFrameBuffers() { return m_FrameBuffers; };
		virtual SharedPointer<FrameBuffer>& GetFrameBuffer() = 0;
	protected:
		SharedPointer<RenderCommandBuffer> m_CommandBuffer;
		std::uint32_t m_FramesCount;
		std::vector<SharedPointer<FrameBuffer>> m_FrameBuffers;
	};
}