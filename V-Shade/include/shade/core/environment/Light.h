#pragma once
#include <shade/core/memory/Memory.h>
#include <glm/glm/glm.hpp>

namespace shade
{
	class SHADE_API Light
	{
	public:
		struct RenderData
		{
			alignas(16)     float Intensity;
			alignas(16)		glm::vec3 DiffuseColor;
			alignas(16)		glm::vec3 SpecularColor;
		};
	public:
		Light() = default;
		virtual ~Light() = default;
	public:
		glm::vec3		DiffuseColor	= glm::vec3(1.f, 1.f, 1.f);
		glm::vec3		SpecularColor	= glm::vec3(1.f, 1.f, 1.f);
		float			Intensity		= 1.f;
	};
}