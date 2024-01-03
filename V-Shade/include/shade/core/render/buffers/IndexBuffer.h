#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/render/buffers/RenderCommandBuffer.h>

namespace shade
{
	class SHADE_API IndexBuffer
	{
		SHADE_CAST_HELPER(FrameBuffer)

	public:
		enum class Usage
		{
			GPU = 0, CPU_GPU = 1
		};
		virtual ~IndexBuffer() = default;

		virtual std::uint32_t GetCount() const = 0;
		virtual void Bind(SharedPointer<RenderCommandBuffer>& commandBuffer, std::uint32_t frameIndex, std::uint32_t offset = 0) const = 0;
		virtual void Resize(std::uint32_t size) = 0;
		virtual void SetData(std::uint32_t size, const void* data, std::uint32_t offset = 0) = 0;

	public:
		static SharedPointer<IndexBuffer> Create(Usage usage, std::uint32_t size, std::uint32_t resizeThreshold = 0, const void* data = nullptr);
	protected:
		Usage m_Usage;
		std::uint32_t m_Size = 0;
		std::uint32_t m_ResizeThreshold = 0;
		bool HasToBeResized(std::uint32_t oldSize, std::uint32_t newSize, std::uint32_t threshold);
	};
}