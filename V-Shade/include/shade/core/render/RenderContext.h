#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/system/SystemsRequirements.h>

namespace shade
{
	class SHADE_API RenderContext
	{
	public:
		virtual ~RenderContext() = default;
		virtual void Initialize(const SystemsRequirements& requirements) = 0;
		virtual void ShutDown() = 0;
		// Create instance.
		static UniquePointer<RenderContext> Create();
		// For internal converting.
		template<typename T>
		T& As();
	};
	template<typename T>
	inline T& RenderContext::As()
	{
		static_assert(std::is_base_of<RenderContext, T>::value, "Is not base of RenderContext");
		return static_cast<T&>(*this);
	}
}