#pragma once
#include <shade/core/render/drawable/Drawable.h>

namespace shade
{
	class SHADE_API Cone: public Drawable
	{
	public:
		static SharedPointer<Cone> Create(float radius, float length, std::uint32_t density, float step, const glm::vec3& derection = glm::vec3(0.f, 0.f, 1.f));
		virtual ~Cone() = default;
	private:
		Cone(float radius, float length, std::uint32_t density, float step, const glm::vec3& derection);
		friend class SharedPointer<Cone>;
	};

}