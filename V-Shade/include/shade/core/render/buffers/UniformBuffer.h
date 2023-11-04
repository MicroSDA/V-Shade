#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/render/buffers/RenderCommandBuffer.h>

namespace shade
{
	class SHADE_API UniformBuffer
	{
	public:
		enum class Usage
		{
			GPU = 0, CPU_GPU = 1
		};
	public:
		virtual ~UniformBuffer() = default;

		static SharedPointer<UniformBuffer> Create(Usage usage, std::uint32_t binding, std::uint32_t size, std::uint32_t framesCount = 1, std::size_t resizeThreshold = 0);
		virtual void SetData(std::uint32_t size, const void* data, std::uint32_t frameIndex = 1, std::uint32_t offset = 0) = 0;
		virtual void Resize(std::uint32_t size) = 0;
		virtual std::uint32_t GetBinding() = 0;

		template<typename T>
		T& As();
	protected:
		std::uint32_t m_ResizeThreshold = 0;
		Usage m_Usage;
		bool HasToBeResized(std::uint32_t oldSize, std::uint32_t newSize, std::uint32_t threshold);
	};
	template<typename T>
	inline T& UniformBuffer::As()
	{
		static_assert(std::is_base_of<UniformBuffer, T>::value, "Is not base of UniformBuffer");
		return static_cast<T&>(*this);
	}
}