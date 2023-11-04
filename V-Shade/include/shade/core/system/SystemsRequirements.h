#pragma once

namespace shade
{
	struct SystemsRequirements
	{
		struct
		{
			bool Discrete = false;
			std::uint32_t MinGpuMemory = 0;
		} GPU;
		std::uint32_t FramesInFlight = 1;
	};
}