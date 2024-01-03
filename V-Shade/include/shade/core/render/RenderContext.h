#pragma once
#include <shade/config/ShadeAPI.h>
#include <shade/core/memory/Memory.h>
#include <shade/core/system/SystemsRequirements.h>

namespace shade
{
	class SHADE_API RenderContext
	{

		SHADE_CAST_HELPER(RenderContext)

	public:
		virtual ~RenderContext() = default;
		virtual void Initialize(const SystemsRequirements& requirements) = 0;
		virtual void ShutDown() = 0;
		// Create instance.
		static UniquePointer<RenderContext> Create();
	};
}