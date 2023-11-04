#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/render/shader/Shader.h>
#include <shade/core/render/buffers/FrameBuffer.h>

namespace shade
{
	class SHADE_API RenderCommandBuffer
	{
	public:
		enum class Type
		{
			None = 0,
			Secondary,
			Primary
		};
		enum Family
		{
			None = -1,
			Present,
			Graphic,
			Transfer,
			Compute,
			FAMILY_MAX_ENUM
		};
		virtual ~RenderCommandBuffer() = default;
		virtual void Begin(std::uint32_t index = 0) = 0;
		virtual void End(std::uint32_t index = 0) = 0;
		virtual void Submit(std::uint32_t index = 0, std::uint32_t timeout = UINT32_MAX) = 0;

		template<typename T>
		T& As();
	public:
		static SharedPointer<RenderCommandBuffer> Create(const Type& type = Type::Primary, const Family& family = Family::Graphic, const std::uint32_t& count = 1, const std::string& name = "Command buffer.");
		static SharedPointer<RenderCommandBuffer> CreateFromSwapChain();

		bool IsFromSwapChain = false;
	protected:
		Type m_Type;
		Family m_Family;
		std::string m_Name;
	protected:
		//TIP: Or in case that will not work just set this as before only for one frame std::array<std::mutex, Family::FAMILY_MAX_ENUM> without std::map
		static std::array<std::mutex, Family::FAMILY_MAX_ENUM> m_sMutexs;
	};

	template<typename T>
	inline T& RenderCommandBuffer::As()
	{
		static_assert(std::is_base_of<RenderCommandBuffer, T>::value, "Is not base of RenderCommandBuffer");
		return static_cast<T&>(*this);
	}

}